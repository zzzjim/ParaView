/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  
-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkDenseArray.h"
#include "vtkDiagonalMatrixSource.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSparseArray.h"

// ----------------------------------------------------------------------

vtkCxxRevisionMacro(vtkDiagonalMatrixSource, "$Revision$");
vtkStandardNewMacro(vtkDiagonalMatrixSource);

// ----------------------------------------------------------------------

vtkDiagonalMatrixSource::vtkDiagonalMatrixSource() :
  ArrayType(DENSE),
  Extents(3),
  Diagonal(1.0),
  SuperDiagonal(0.0),
  SubDiagonal(0.0)
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

// ----------------------------------------------------------------------

vtkDiagonalMatrixSource::~vtkDiagonalMatrixSource()
{
}

// ----------------------------------------------------------------------

void vtkDiagonalMatrixSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ArrayType: " << this->ArrayType << endl;
  os << indent << "Extents: " << this->Extents << endl;
  os << indent << "Diagonal: " << this->Diagonal << endl;
  os << indent << "SuperDiagonal: " << this->SuperDiagonal << endl;
  os << indent << "SubDiagonal: " << this->SubDiagonal << endl;
}

// ----------------------------------------------------------------------

int vtkDiagonalMatrixSource::RequestData(
  vtkInformation*, 
  vtkInformationVector**, 
  vtkInformationVector* outputVector)
{
  if(this->Extents < 1)
    {
    vtkErrorMacro(<< "Invalid matrix extents: " << this->Extents << "x" << this->Extents << " array is not supported.");
    return 0;
    }
 
  vtkArray* array = 0;
  switch(this->ArrayType)
    {
    case DENSE:
      array = this->GenerateDenseArray();
      break;
    case SPARSE:
      array = this->GenerateSparseArray();
      break;
    default:
      vtkErrorMacro(<< "Invalid array type: " << this->ArrayType << ".");
      return 0;
    }
    
  vtkArrayData* const output = vtkArrayData::GetData(outputVector);
  output->SetArray(array);
  array->Delete();

  return 1;
}

vtkArray* vtkDiagonalMatrixSource::GenerateDenseArray()
{
  vtkDenseArray<double>* const array = vtkDenseArray<double>::New();
  array->Resize(vtkArrayExtents::Uniform(2, this->Extents));

  array->Fill(0.0);

  if(this->Diagonal != 0.0)
    {
    for(vtkIdType i = 0; i != this->Extents; ++i)
      array->SetValue(vtkArrayCoordinates(i, i), this->Diagonal);
    }

  if(this->SuperDiagonal != 0.0)
    {
    for(vtkIdType i = 0; i + 1 != this->Extents; ++i)
      array->SetValue(vtkArrayCoordinates(i, i+1), this->SuperDiagonal);
    }

  if(this->SubDiagonal != 0.0)
    {
    for(vtkIdType i = 0; i + 1 != this->Extents; ++i)
      array->SetValue(vtkArrayCoordinates(i+1, i), this->SubDiagonal);
    }

  return array;
}

vtkArray* vtkDiagonalMatrixSource::GenerateSparseArray()
{
  vtkSparseArray<double>* const array = vtkSparseArray<double>::New();
  array->Resize(vtkArrayExtents::Uniform(2, this->Extents));

  if(this->Diagonal != 0.0)
    {
    for(vtkIdType i = 0; i != this->Extents; ++i)
      array->AddValue(vtkArrayCoordinates(i, i), this->Diagonal);
    }

  if(this->SuperDiagonal != 0.0)
    {
    for(vtkIdType i = 0; i + 1 != this->Extents; ++i)
      array->AddValue(vtkArrayCoordinates(i, i+1), this->SuperDiagonal);
    }

  if(this->SubDiagonal != 0.0)
    {
    for(vtkIdType i = 0; i + 1 != this->Extents; ++i)
      array->AddValue(vtkArrayCoordinates(i+1, i), this->SubDiagonal);
    }

  return array;
}
