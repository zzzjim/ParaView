/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkClientServerMoveData.h"

#include "vtkCharArray.h"
#include "vtkClientConnection.h"
#include "vtkDataObject.h"
#include "vtkDataObjectTypes.h"
#include "vtkGenericDataObjectReader.h"
#include "vtkGenericDataObjectWriter.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkServerConnection.h"
#include "vtkSocketController.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMultiBlockDataSet.h"

vtkStandardNewMacro(vtkClientServerMoveData);
vtkCxxRevisionMacro(vtkClientServerMoveData, "$Revision$");
vtkCxxSetObjectMacro(vtkClientServerMoveData, ProcessModuleConnection, 
  vtkProcessModuleConnection);

//-----------------------------------------------------------------------------
vtkClientServerMoveData::vtkClientServerMoveData()
{
  this->ProcessModuleConnection = 0;
  this->OutputDataType = VTK_POLY_DATA;
  this->WholeExtent[0] =  0;
  this->WholeExtent[1] = -1;
  this->WholeExtent[2] =  0;
  this->WholeExtent[3] = -1;
  this->WholeExtent[4] =  0;
  this->WholeExtent[5] = -1;
}

//-----------------------------------------------------------------------------
vtkClientServerMoveData::~vtkClientServerMoveData()
{
  this->SetProcessModuleConnection(0);
}

//-----------------------------------------------------------------------------
int vtkClientServerMoveData::FillInputPortInformation(int idx, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return this->Superclass::FillInputPortInformation(idx, info);
}

//----------------------------------------------------------------------------
int vtkClientServerMoveData::RequestDataObject(
  vtkInformation* vtkNotUsed(reqInfo), 
  vtkInformationVector**, 
  vtkInformationVector* outputVector)
{
  const char *outTypeStr = 
    vtkDataObjectTypes::GetClassNameFromTypeId(this->OutputDataType);

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkDataObject *output = info->Get(vtkDataObject::DATA_OBJECT());
  if (!output || !output->IsA(outTypeStr)) 
    {
    vtkDataObject* newOutput = 
      vtkDataObjectTypes::NewDataObject(this->OutputDataType);
    if (!newOutput)
      {
      vtkErrorMacro("Could not create chosen output data type: "
                    << outTypeStr);
      return 0;
      }
    newOutput->SetPipelineInformation(info);
    this->GetOutputPortInformation(0)->Set(
      vtkDataObject::DATA_EXTENT_TYPE(), newOutput->GetExtentType());
    newOutput->Delete();
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkClientServerMoveData::RequestInformation(
  vtkInformation* request, 
  vtkInformationVector** inputVector, 
  vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (inputVector[0]->GetNumberOfInformationObjects() > 0)
    {
    return 
      this->Superclass::RequestInformation(request, inputVector, outputVector);
    }
  else
    {
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
                 this->WholeExtent,
                 6);
    }
  return 1;
}

//-----------------------------------------------------------------------------
int vtkClientServerMoveData::RequestData(vtkInformation*,
                                vtkInformationVector** inputVector,
                                vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkDataObject* input = 0;
  vtkDataObject* output = outInfo->Get(vtkDataObject::DATA_OBJECT());

  if (inputVector[0]->GetNumberOfInformationObjects() > 0)
    {
    input = inputVector[0]->GetInformationObject(0)->Get(
        vtkDataObject::DATA_OBJECT());
    }

  vtkRemoteConnection* rc = vtkRemoteConnection::SafeDownCast(
    this->ProcessModuleConnection);
  if (rc)
    {
    vtkSocketController* controller = rc->GetSocketController();
    if (this->ProcessModuleConnection->IsA("vtkClientConnection"))
      {
      vtkDebugMacro("Server Root: Send input data to client.");
      // This is a server root node.
      return controller->Send(input, 1,
                              vtkClientServerMoveData::TRANSMIT_DATA_OBJECT);
      }
    else if (this->ProcessModuleConnection->IsA("vtkServerConnection"))
      {
      vtkDebugMacro("Client: Get data from server and put it on the output.");
      // This is a client node.
      vtkDataObject* data = 
        controller->ReceiveDataObject(
          1, vtkClientServerMoveData::TRANSMIT_DATA_OBJECT); 
      if (data)
        {
        if (output->IsA(data->GetClassName()))
          {
          output->ShallowCopy(data);
          }
        else
          {
          data->SetPipelineInformation(outputVector->GetInformationObject(0));
          }
        data->Delete();
        return 1;
        }
      }
    }

  vtkDebugMacro("Shallow copying input to output.");
  // If not a remote connection, nothing more to do than
  // act as a pass through filter.
  output->ShallowCopy(input);
  return 1;
}

//-----------------------------------------------------------------------------
void vtkClientServerMoveData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ProcessModuleConnection: " << this->ProcessModuleConnection
    << endl;
  os << indent << "WholeExtent: " 
    << this->WholeExtent[0] << ", "
    << this->WholeExtent[1] << ", "
    << this->WholeExtent[2] << ", "
    << this->WholeExtent[3] << ", "
    << this->WholeExtent[4] << ", "
    << this->WholeExtent[5] << endl;
  os << indent << "OutputDataType: " << this->OutputDataType << endl;
}
