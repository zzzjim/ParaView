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
// .NAME vtkCompositeDataSet - abstact superclass for composite (multi-block or AMR) datasets
// .SECTION Description
// vtkCompositeDataSet is an abstract class that represents a collection
// of datasets (including other composite datasets). This superclass
// does not implement an actual method for storing datasets. It
// only provides an interface to access the datasets through iterators.

// .SECTION See Also
// vtkCompositeDataIterator 

#ifndef __vtkCompositeDataSet_h
#define __vtkCompositeDataSet_h

#include "vtkDataObject.h"

class vtkCompositeDataIterator;
class vtkInformation;

class VTK_FILTERING_EXPORT vtkCompositeDataSet : public vtkDataObject
{
public:
  vtkTypeRevisionMacro(vtkCompositeDataSet,vtkDataObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return a new (forward) iterator 
  // (the iterator has to be deleted by user)
  virtual vtkCompositeDataIterator* NewIterator() = 0;

  // Description:
  // Return class name of data type (see vtkSystemIncludes.h for
  // definitions).
  virtual int GetDataObjectType() {return VTK_COMPOSITE_DATA_SET;}

  // Description:
  // Restore data object to initial state,
  virtual void Initialize();

  // Description:
  // For streaming.  User/next filter specifies which piece the want updated.
  // The source of this data has to return exactly this piece.
  void SetUpdateExtent(int piece, int numPieces, int ghostLevel);
  void SetUpdateExtent(int piece, int numPieces)
    {this->SetUpdateExtent(piece, numPieces, 0);}
  void GetUpdateExtent(int &piece, int &numPieces, int &ghostLevel);

  // Description:
  // We need this here to avoid hiding superclass method
  virtual int* GetUpdateExtent();
  virtual void GetUpdateExtent(int& x0, int& x1, int& y0, int& y1,
                               int& z0, int& z1);
  virtual void GetUpdateExtent(int extent[6]);

  // Description:
  // Call superclass method to avoid hiding
  // Since this data type does not use 3D extents, this set method
  // is useless but necessary since vtkDataSetToDataSetFilter does not
  // know what type of data it is working on.
  void SetUpdateExtent( int x1, int x2, int y1, int y2, int z1, int z2 )
    { this->Superclass::SetUpdateExtent( x1, x2, y1, y2, z1, z2 ); };
  void SetUpdateExtent( int ext[6] )
    { this->Superclass::SetUpdateExtent( ext ); };

  // Description:
  // Adds dobj to the composite dataset. Where the dataset goes is determined
  // by appropriate keys in the index information object. Which keys are used
  // depends on the actual subclass.
  virtual void AddDataSet(vtkInformation* index, vtkDataObject* dobj) = 0;

  // Description: 
  // Returns a dataset pointed by appropriate keys in the index information
  // object.  Which keys are used depends on the actual subclass.
  virtual vtkDataObject* GetDataSet(vtkInformation* index) = 0;

  static vtkInformationIntegerKey* INDEX();

protected:
  vtkCompositeDataSet();
  ~vtkCompositeDataSet();

private:
  vtkCompositeDataSet(const vtkCompositeDataSet&);  // Not implemented.
  void operator=(const vtkCompositeDataSet&);  // Not implemented.
};

#endif

