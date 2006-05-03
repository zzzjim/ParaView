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
#include "vtkGenericCell.h"

static const unsigned int depth = 5;
static unsigned char HigherOrderCell[][depth] = {
    { VTK_LINE, VTK_QUADRATIC_EDGE, VTK_NUMBER_OF_CELL_TYPES,
      VTK_NUMBER_OF_CELL_TYPES, VTK_NUMBER_OF_CELL_TYPES },
    { VTK_TRIANGLE, VTK_QUADRATIC_TRIANGLE, VTK_NUMBER_OF_CELL_TYPES,
      VTK_NUMBER_OF_CELL_TYPES, VTK_NUMBER_OF_CELL_TYPES },
    { VTK_QUAD, VTK_QUADRATIC_QUAD, VTK_QUADRATIC_LINEAR_QUAD,
      VTK_BIQUADRATIC_QUAD, VTK_NUMBER_OF_CELL_TYPES},
    { VTK_TETRA, VTK_QUADRATIC_TETRA, VTK_NUMBER_OF_CELL_TYPES,
      VTK_NUMBER_OF_CELL_TYPES, VTK_NUMBER_OF_CELL_TYPES },
    { VTK_HEXAHEDRON, VTK_QUADRATIC_HEXAHEDRON,
      VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON, VTK_TRIQUADRATIC_HEXAHEDRON, VTK_NUMBER_OF_CELL_TYPES },
    { VTK_WEDGE, VTK_QUADRATIC_WEDGE, VTK_QUADRATIC_LINEAR_WEDGE,
      VTK_BIQUADRATIC_QUADRATIC_WEDGE, VTK_NUMBER_OF_CELL_TYPES },
    { VTK_PYRAMID, VTK_QUADRATIC_PYRAMID, VTK_NUMBER_OF_CELL_TYPES,
      VTK_NUMBER_OF_CELL_TYPES, VTK_NUMBER_OF_CELL_TYPES }
    };

//----------------------------------------------------------------------------
// Simply set the points to the pcoords coordinate
// and the point id to the natural order
void InitializeACell(vtkCell *cell)
{
  if( cell )
    {
    int numPts = cell->GetNumberOfPoints();
    for(int i = 0; i < numPts; ++i)
      {
      cell->GetPointIds()->SetId(i,i);
      }
    }
}

//----------------------------------------------------------------------------
// c1 is the reference cell. In the test this is the linear cell
// and thus c2 is the higher order one. We need to check that result on c1
// are consistant with result on c2 (but we cannot say anything after that)
int CompareHigherOrderCell(vtkCell *c1, vtkCell *c2)
{
  int rval = 0;
  //c1->Print( cout );
  //c2->Print( cout );
  int c1numPts = c1->GetNumberOfPoints();
  for( int p = 0; p < c1numPts; ++p)
    {
    vtkIdType pid1 = c1->GetPointId(p);
    vtkIdType pid2 = c2->GetPointId(p);
    if( pid1 != pid2 )
      {
      cerr << "Problem with pid:" << pid1 << " != " << pid2 << " in cell #" <<
        c1->GetCellType() << " and #" << c2->GetCellType() << endl;
      ++rval;
      }
    }
  return rval;
}

//----------------------------------------------------------------------------
int TestHigherOrderCell(int , char *[])
{
  int rval = 0;
  if( sizeof(HigherOrderCell[0]) != depth )
    {
    cerr << sizeof(HigherOrderCell[0]) << endl;
    cerr << "Problem in the test" << endl;
    return 1;
    }

  const unsigned char *orderCell;
  const unsigned int nCells = sizeof(HigherOrderCell)/depth;
  vtkCell* cellArray[depth];
  for( unsigned int i = 0; i < nCells; ++i)
    {
    orderCell = HigherOrderCell[i];
    //cerr << "Higher : " << (int)orderCell[0] << "," << (int)orderCell[1] << ","
    //  << (int)orderCell[2] << "," << (int)orderCell[3] << "," << (int)orderCell[4] << endl;
    for( unsigned int c = 0; c < depth; ++c)
      {
      const int cellType = orderCell[c];
      cellArray[c] = vtkGenericCell::InstantiateCell(cellType);
      InitializeACell( cellArray[c] );
      }
    vtkCell *linCell = cellArray[0]; // this is the reference linear cell
    //const int numPts   = linCell->GetNumberOfPoints();
    const int numEdges = linCell->GetNumberOfEdges();
    const int numFaces = linCell->GetNumberOfFaces();
    const int dim      = linCell->GetCellDimension();
    // First check consistancy across cell of higher dimension:
    for( unsigned int c = 0; c < depth; ++c)
      {
      vtkCell *cell = cellArray[c];
      if( cell )
        {
        if( cell->GetCellType() != (int)orderCell[c] )
          {
          cerr << "Problem in the test" << endl;
          ++rval;
          }
        if( cell->GetCellDimension() != dim )
          {
          cerr << "Wrong dim for cellId #" << cell->GetCellType() << endl;
          ++rval;
          }
        if( cell->GetNumberOfEdges() != numEdges)
          {
          cerr << "Wrong numEdges for cellId #" << cell->GetCellType() << endl;
          ++rval;
          }
        if( cell->GetNumberOfFaces() != numFaces )
          {
          cerr << "Wrong numFace for cellId #" << cell->GetCellType() << endl;
          ++rval;
          }
        // Make sure that edge across all different cell are identical
        for(int e=0; e<numEdges; ++e)
          {
          vtkCell *c1 = linCell->GetEdge(e);
          vtkCell *c2 = cell->GetEdge(e);
          cerr << "Doing: #" << linCell->GetCellType() << " vs "
            << cell->GetCellType() << endl;
          rval += CompareHigherOrderCell(c1, c2);
          }
        // Make sure that face across all different cell are identical
        for(int f=0; f<numFaces; ++f)
          {
          vtkCell *f1 = linCell->GetFace(f);
          vtkCell *f2 = cell->GetFace(f);
          rval += CompareHigherOrderCell(f1, f2);
          }
        }
      }
    // Cleanup
    for( unsigned int c = 0; c < depth; ++c)
      {
      vtkCell *cell = cellArray[c];
      if( cell )
        {
        cell->Delete();
        }
      }
    }

  return rval;
}

