/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Description:
---------------------------------------------------------------------------
This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "UGrid.hh"
#include "Point.hh"
#include "PolyPts.hh"
#include "Line.hh"
#include "PolyLine.hh"
#include "Triangle.hh"
#include "TriStrip.hh"
#include "Quad.hh"
#include "Rect.hh"
#include "Polygon.hh"
#include "Tetra.hh"
#include "Hexa.hh"
#include "Brick.hh"
#include "UGridMap.hh"

vlUnstructuredGrid::vlUnstructuredGrid ()
{
  this->Connectivity = NULL;
  this->Cells = NULL;
  this->Links = NULL;
}

void vlUnstructuredGrid::Allocate (int numCells=1000, int extSize=1000)
{
  if ( numCells < 1 ) numCells = 1000;
  if ( extSize < 1 ) extSize = 1000;

  this->Connectivity = new vlCellArray(numCells,4*extSize);
  this->Connectivity->Register(this);

  this->Cells = new vlCellList(numCells,extSize);
  this->Cells->Register(this);
  
  this->Links = new vlLinkList(numCells,extSize);
  this->Links->Register(this);
}

vlUnstructuredGrid::vlUnstructuredGrid(const vlUnstructuredGrid& pd)
{
  this->Connectivity = pd.Connectivity;
  if (this->Connectivity) this->Connectivity->Register(this);

  this->Cells = pd.Cells;
  if (this->Cells) this->Cells->Register(this);

  this->Links = pd.Links;
  if (this->Links) this->Links->Register(this);
}

vlUnstructuredGrid::~vlUnstructuredGrid()
{
  vlUnstructuredGrid::Initialize();
}

vlMapper *vlUnstructuredGrid::MakeMapper()
{
  vlUnstructuredGridMapper *mapper;

  if ( this->Mapper == NULL )
    {
    this->Mapper = mapper = new vlUnstructuredGridMapper;
    this->Mapper->Register(this);
    mapper->SetInput(this);
    }
  return this->Mapper;
}

void vlUnstructuredGrid::Initialize()
{
  vlPointSet::Initialize();

  if ( this->Connectivity )
  {
    this->Connectivity->UnRegister(this);
    this->Connectivity = NULL;
  }

  if ( this->Cells )
  {
    this->Cells->UnRegister(this);
    this->Cells = NULL;
  }

  if ( this->Links )
  {
    this->Links->UnRegister(this);
    this->Links = NULL;
  }
};

int vlUnstructuredGrid::GetCellType(int cellId)
{
  return this->Cells->GetCellType(cellId);
}

vlCell *vlUnstructuredGrid::GetCell(int cellId)
{
  static vlPoint point;
  static vlPolyPoints ppoints;
  static vlLine line;
  static vlPolyLine pline;
  static vlTriangle triangle;
  static vlTriangleStrip strip;
  static vlPolygon poly;
  static vlRectangle rect;
  static vlQuad quad;
  static vlTetra tetra;
  static vlBrick brick;
  static vlHexahedron hexa;
  int i, loc, numPts, *pts;
  vlCell *cell;

  switch (this->Cells->GetCellType(cellId))
    {
    case vlPOINT:
     cell = &point;
     break;

    case vlPOLY_POINTS:
     cell = &ppoints;
     break;

    case vlLINE: 
      cell = &line;
      break;

    case vlPOLY_LINE:
      cell = &pline;
      break;

    case vlTRIANGLE:
      cell = &triangle;
      break;

    case vlTRIANGLE_STRIP:
      cell = &strip;
      break;

    case vlRECTANGLE:
      cell = &rect;
      break;

    case vlQUAD:
      cell = &quad;
      break;

    case vlPOLYGON:
      cell = &poly;
      break;

    case vlTETRA:
      cell = &tetra;
      break;

    case vlBRICK:
      cell = &brick;
      break;

    case vlHEXAHEDRON:
      cell = &hexa;
      break;
    }

  loc = this->Cells->GetCellLocation(cellId);
  this->Connectivity->GetCell(loc,numPts,pts); 

  for (i=0; i<numPts; i++)
    {
    cell->PointIds.SetId(i,pts[i]);
    cell->Points.SetPoint(i,this->Points->GetPoint(pts[i]));
    }

  return cell;
}

int vlUnstructuredGrid::GetNumberOfCells() 
{
  return (this->Connectivity ? this->Connectivity->GetNumberOfCells() : 0);
}

void vlUnstructuredGrid::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlUnstructuredGrid::GetClassName()))
    {
    vlDataSet::PrintSelf(os,indent);
    }
}

int vlUnstructuredGrid::InsertNextCell(int type, vlIdList& ptIds)
{
  int i;
  int npts=ptIds.GetNumberOfIds();

  // insert connectivity
  this->Connectivity->InsertNextCell(npts);
  for (i=0; i < npts; i++) this->Connectivity->InsertCellPoint(ptIds.GetId(i));

  // insert type and storage information   
  this->Cells->InsertNextCell(type,this->Connectivity->GetLocation(npts));
}

int vlUnstructuredGrid::InsertNextCell(int type, int npts, int pts[MAX_CELL_SIZE])
{
  this->Connectivity->InsertNextCell(npts,pts);
  this->Cells->InsertNextCell(type,this->Connectivity->GetLocation(npts));
}

void vlUnstructuredGrid::InsertCells(int numCells, int width, int *data)
{

}

void vlUnstructuredGrid::InsertCells(int numCells, int *data)
{

}

void vlUnstructuredGrid::BuildLinks()
{
  this->Links = new vlLinkList(this->GetNumberOfPoints());
  this->Links->BuildLinks(this);
}

void vlUnstructuredGrid::GetPointCells(int ptId, vlIdList *cellIds)
{
  int *cells;
  int numCells;
  int i;

  if ( ! this->Links ) this->BuildLinks();
  cellIds->Reset();

  numCells = this->Links->GetNcells(ptId);
  cells = this->Links->GetCells(ptId);

  for (i=0; i < numCells; i++)
    {
    cellIds->InsertId(i,cells[i]);
    }
}

void vlUnstructuredGrid::Squeeze()
{
  if ( this->Connectivity ) this->Connectivity->Squeeze();
  if ( this->Cells ) this->Cells->Squeeze();
  if ( this->Links ) this->Links->Squeeze();
}

