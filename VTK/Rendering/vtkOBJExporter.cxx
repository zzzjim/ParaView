/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOBJExporter.h"

#include "vtkActorCollection.h"
#include "vtkAssemblyNode.h"
#include "vtkFloatArray.h"
#include "vtkGeometryFilter.h"
#include "vtkMapper.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRendererCollection.h"
#include "vtkTransform.h"

vtkCxxRevisionMacro(vtkOBJExporter, "$Revision$");
vtkStandardNewMacro(vtkOBJExporter);

vtkOBJExporter::vtkOBJExporter()
{
  this->FilePrefix = NULL;
}

vtkOBJExporter::~vtkOBJExporter()
{
  if ( this->FilePrefix )
    {
    delete [] this->FilePrefix;
    }
}

void vtkOBJExporter::WriteData()
{
  vtkRenderer *ren;
  FILE *fpObj, *fpMtl;
  vtkActorCollection *ac;
  vtkActor *anActor, *aPart;
  char nameObj[80];
  char nameMtl[80];
  int idStart = 1;
  
  // make sure the user specified a filename
  if ( this->FilePrefix == NULL)
    {
    vtkErrorMacro(<< "Please specify file prefix to use");
    return;
    }

  // first make sure there is only one renderer in this rendering window
  if (this->RenderWindow->GetRenderers()->GetNumberOfItems() > 1)
    {
    vtkErrorMacro(<< "obj files only support on renderer per window.");
    return;
    }

  // get the renderer
  this->RenderWindow->GetRenderers()->InitTraversal();
  ren = this->RenderWindow->GetRenderers()->GetNextItem();
  
  // make sure it has at least one actor
  if (ren->GetActors()->GetNumberOfItems() < 1)
    {
    vtkErrorMacro(<< "no actors found for writing .obj file.");
    return;
    }
    
  // try opening the files
  sprintf(nameObj,"%s.obj",this->FilePrefix);
  sprintf(nameMtl,"%s.mtl",this->FilePrefix);
  fpObj = fopen(nameObj,"w");
  fpMtl = fopen(nameMtl,"w");
  if (!fpObj || !fpMtl)
    {
    vtkErrorMacro(<< "unable to open .obj and .mtl files ");
    return;
    }
  
  //
  //  Write header
  //
  vtkDebugMacro("Writing wavefront files");
  fprintf(fpObj, 
          "# wavefront obj file written by the visualization toolkit\n\n");
  fprintf(fpObj, "mtllib %s\n\n", nameMtl);
  fprintf(fpMtl, 
          "# wavefront mtl file written by the visualization toolkit\n\n");
  
  ac = ren->GetActors();
  vtkAssemblyPath *apath;
  for (ac->InitTraversal(); (anActor = ac->GetNextActor()); )
    {
    for (anActor->InitPathTraversal(); (apath=anActor->GetNextPath()); )
      {
      aPart=(vtkActor *)apath->GetLastNode()->GetProp();
      this->WriteAnActor(aPart, fpObj, fpMtl, idStart);
      }
    }
  
  fclose(fpObj);
  fclose(fpMtl);
}

void vtkOBJExporter::WriteAnActor(vtkActor *anActor, FILE *fpObj, FILE *fpMtl,
                                  int &idStart)
{
  vtkDataSet *ds;
  vtkPolyData *pd;
  vtkGeometryFilter *gf = NULL;
  vtkPointData *pntData;
  vtkPoints *points = NULL;
  vtkDataArray *normals = NULL;
  vtkDataArray *tcoords = NULL;
  int i, i1, i2, idNext;
  vtkProperty *prop;
  float *tempf, *p;
  vtkCellArray *cells;
  vtkTransform *trans = vtkTransform::New();
  vtkIdType npts = 0;
  vtkIdType *indx = 0;
  
  // see if the actor has a mapper. it could be an assembly
  if (anActor->GetMapper() == NULL)
    {
    return;
    }

  // write out the material properties to the mat file
  prop = anActor->GetProperty();
  fprintf(fpMtl,"newmtl mtl%i\n",idStart);
  tempf = prop->GetAmbientColor();
  fprintf(fpMtl,"Ka %g %g %g\n",tempf[0], tempf[1], tempf[2]);
  tempf = prop->GetDiffuseColor();
  fprintf(fpMtl,"Kd %g %g %g\n",tempf[0], tempf[1], tempf[2]);
  tempf = prop->GetSpecularColor();
  fprintf(fpMtl,"Ks %g %g %g\n",tempf[0], tempf[1], tempf[2]);
  fprintf(fpMtl,"Ns %g\n",prop->GetSpecularPower());
  fprintf(fpMtl,"Tf %g %g %g\n",1.0 - prop->GetOpacity(),
          1.0 - prop->GetOpacity(),1.0 - prop->GetOpacity());
  fprintf(fpMtl,"illum 3\n\n");

  // get the mappers input and matrix
  ds = anActor->GetMapper()->GetInput();
  ds->Update();
  trans->SetMatrix(anActor->vtkProp3D::GetMatrix());
    
  // we really want polydata
  if ( ds->GetDataObjectType() != VTK_POLY_DATA )
    {
    gf = vtkGeometryFilter::New();
    gf->SetInput(ds);
    gf->Update();
    pd = gf->GetOutput();
    }
  else
    {
    pd = (vtkPolyData *)ds;
    }

  // write out the points
  points = vtkPoints::New();
  trans->TransformPoints(pd->GetPoints(),points);
  for (i = 0; i < points->GetNumberOfPoints(); i++)
    {
    p = points->GetPoint(i);
    fprintf (fpObj, "v %g %g %g\n", p[0], p[1], p[2]);
    }
  idNext = idStart + (int)(points->GetNumberOfPoints());
  points->Delete();
  
  // write out the point data
  pntData = pd->GetPointData();
  if (pntData->GetNormals())
    {
    normals = vtkFloatArray::New();
    normals->SetNumberOfComponents(3);
    trans->TransformNormals(pntData->GetNormals(),normals);
    for (i = 0; i < normals->GetNumberOfTuples(); i++)
      {
      p = normals->GetTuple(i);
      fprintf (fpObj, "vn %g %g %g\n", p[0], p[1], p[2]);
      }
    }
  
  tcoords = pntData->GetTCoords();
  if (tcoords)
    {
    for (i = 0; i < tcoords->GetNumberOfTuples(); i++)
      {
      p = tcoords->GetTuple(i);
      fprintf (fpObj, "vt %g %g\n", p[0], p[1]);
      }
    }
  
  // write out a group name and material
  fprintf (fpObj, "\ng grp%i\n", idStart);
  fprintf (fpObj, "usemtl mtl%i\n", idStart);
  
  // write out verts if any
  if (pd->GetNumberOfVerts() > 0)
    {
    cells = pd->GetVerts();
    for (cells->InitTraversal(); cells->GetNextCell(npts,indx); )
      {
      fprintf(fpObj,"p ");
      for (i = 0; i < npts; i++)
        {
        // treating vtkIdType as int
        fprintf(fpObj,"%i ", ((int)indx[i])+idStart);
        }
      fprintf(fpObj,"\n");
      }
    }

  // write out lines if any
  if (pd->GetNumberOfLines() > 0)
    {
    cells = pd->GetLines();
    for (cells->InitTraversal(); cells->GetNextCell(npts,indx); )
      {
      fprintf(fpObj,"l ");
      if (tcoords)
        {
        for (i = 0; i < npts; i++)
          {
          // treating vtkIdType as int
          fprintf(fpObj,"%i/%i ",((int)indx[i])+idStart,
                  ((int)indx[i]) + idStart);
          }
        }
      else
        {
        for (i = 0; i < npts; i++)
          {
          // treating vtkIdType as int
          fprintf(fpObj,"%i ", ((int)indx[i])+idStart);
          }
        }
      fprintf(fpObj,"\n");
      }
    }

  // write out polys if any
  if (pd->GetNumberOfPolys() > 0)
    {
    cells = pd->GetPolys();
    for (cells->InitTraversal(); cells->GetNextCell(npts,indx); )
      {
      fprintf(fpObj,"f ");
      for (i = 0; i < npts; i++)
        {
        if (normals)
          {
          if (tcoords)
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i/%i/%i ", ((int)indx[i])+idStart, 
                    ((int)indx[i]) + idStart, ((int)indx[i]) + idStart);
            }
          else
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i//%i ",((int)indx[i])+idStart,
                    ((int)indx[i]) + idStart);
            }
          }
        else
          {
          if (tcoords)
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i/%i ", ((int)indx[i])+idStart, 
                    ((int)indx[i]) + idStart);
            }
          else
            {
            // treating vtkIdType as int
            fprintf(fpObj,"%i ", ((int)indx[i])+idStart);
            }
          }
        }
      fprintf(fpObj,"\n");
      }
    }

  // write out tstrips if any
  if (pd->GetNumberOfStrips() > 0)
    {
    cells = pd->GetStrips();
    for (cells->InitTraversal(); cells->GetNextCell(npts,indx); )
      {
      for (i = 2; i < npts; i++)
        {
        if (i%2)
          {
          i1 = i - 1;
          i2 = i - 2;
          }
        else
          {
          i1 = i - 1;
          i2 = i - 2;
          }
        if (normals)
          {
          if (tcoords)
            {
            // treating vtkIdType as int
            fprintf(fpObj,"f %i/%i/%i ", ((int)indx[i1]) + idStart, 
                    ((int)indx[i1]) + idStart, ((int)indx[i1]) + idStart);
            fprintf(fpObj,"%i/%i/%i ", ((int)indx[i2])+ idStart, 
                    ((int)indx[i2]) + idStart, ((int)indx[i2]) + idStart);
            fprintf(fpObj,"%i/%i/%i\n", ((int)indx[i]) + idStart, 
                    ((int)indx[i]) + idStart, ((int)indx[i]) + idStart);
            }
          else
            {
            // treating vtkIdType as int
            fprintf(fpObj,"f %i//%i ", ((int)indx[i1]) + idStart,
                    ((int)indx[i1]) + idStart);
            fprintf(fpObj,"%i//%i ", ((int)indx[i2]) + idStart,
                    ((int)indx[i2]) + idStart);
            fprintf(fpObj,"%i//%i\n",((int)indx[i]) + idStart,
                    ((int)indx[i]) + idStart);
            }
          }
        else
          {
          if (tcoords)
            {
            // treating vtkIdType as int
            fprintf(fpObj,"f %i/%i ", ((int)indx[i1]) + idStart,
                    ((int)indx[i1]) + idStart);
            fprintf(fpObj,"%i/%i ", ((int)indx[i2]) + idStart,
                    ((int)indx[i2]) + idStart);
            fprintf(fpObj,"%i/%i\n", ((int)indx[i]) + idStart,
                    ((int)indx[i]) + idStart);
            }
          else
            {
            // treating vtkIdType as int
            fprintf(fpObj,"f %i %i %i\n", ((int)indx[i1]) + idStart, 
                    ((int)indx[i2]) + idStart, ((int)indx[i]) + idStart);
            }
          }
        }
      }
    }
  
  idStart = idNext;
  trans->Delete();
  if (normals)
    {
    normals->Delete();
    }
  if (gf)
    {
    gf->Delete();
    }
}



void vtkOBJExporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  if (this->FilePrefix)
    {
    os << indent << "FilePrefix: " << this->FilePrefix << "\n";
    }
  else
    {
    os << indent << "FilePrefix: (null)\n";      
    }
}

