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
// .NAME vtkPCAAnalysisFilter - Performs principal component analysis of a set of aligned pointsets
// .SECTION Description
//
// vtkPCAAnalysisFilter is a filter that takes as input a set of aligned
// pointsets (any object derived from vtkPointSet) and performs
// a principal component analysis of the coordinates. 
// This can be used to visualise the major or minor modes of variation
// seen in a set of similar biological objects with corresponding
// landmarks.
// vtkPCAAnalysisFilter is designed to work with the output from
// the vtkProcrustesAnalysisFilter
// 
// Call SetNumberOfInputs(n) before calling SetInput(0) ... SetInput(n-1).
// Retrieve the outputs using GetOutput(0) ... GetOutput(n-1).
//
// vtkPCAAnalysisFilter is an implementation of (for example):
//
// T. Cootes et al. : Active Shape Models - their training and application.
// Computer Vision and Image Understanding, 61(1):38-59, 1995.
//
// The material can also be found in Tim Cootes' ever-changing online report
// published at his website:
// http://www.isbe.man.ac.uk/~bim/
//
// .SECTION Caveats
// All of the input pointsets must have the same number of points.
//
// .SECTION Thanks
// Rasmus Paulsen and Tim Hutton who developed and contributed this class
//
// .SECTION See Also
// vtkProcrustesAlignmentFilter

#ifndef __vtkPCAAnalysisFilter_h
#define __vtkPCAAnalysisFilter_h

#include "vtkSource.h"

class vtkFloatArray;
class vtkPointSet;

class VTK_HYBRID_EXPORT vtkPCAAnalysisFilter : public vtkSource
{
 public:
  vtkTypeRevisionMacro(vtkPCAAnalysisFilter,vtkSource);
  
  // Description:
  // Prints information about the state of the filter.
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Creates with similarity transform.
  static vtkPCAAnalysisFilter *New();

  // Description:
  // Get the vector of eigenvalues sorted in descending order
  vtkGetObjectMacro(Evals, vtkFloatArray);
  
  // Description:
  // Specify how many pointsets are going to be given as input.
  void SetNumberOfInputs(int n);
  
  // Description:
  // Specify the input pointset with index idx.
  // Call SetNumberOfInputs before calling this function.
  void SetInput(int idx,vtkPointSet* p);
  
  // Description:
  // Retrieve the output point set with index idx.
  // The output is the eigenvector sorted according to their
  // eigenvalues (descending order)
  vtkPointSet* GetOutput(int idx);

  // Description:
  // Fills the shape with:
  //
  // mean + b[0] * sqrt(eigenvalue[0]) * eigenvector[0]
  //      + b[1] * sqrt(eigenvalue[1]) * eigenvector[1]
  // ...
  //      + b[sizeb-1] * sqrt(eigenvalue[bsize-1]) * eigenvector[bsize-1]
  //
  // here b are the parameters expressed in standard deviations
  // bsize is the number of parameters in the b vector
  // This function assumes that shape is allready allocated
  // with the right size, it just moves the points.
  void GetParameterisedShape(vtkFloatArray *b, vtkPointSet* shape);

  // Description:
  // Return the bsize parameters b that best model the given shape
  // (in standard deviations). 
  // That is that the given shape will be approximated by:
  //
  // shape ~ mean + b[0] * sqrt(eigenvalue[0]) * eigenvector[0]
  //              + b[1] * sqrt(eigenvalue[1]) * eigenvector[1]
  //         ...
  //              + b[bsize-1] * sqrt(eigenvalue[bsize-1]) * eigenvector[bsize-1]
  void GetShapeParameters(vtkPointSet *shape, vtkFloatArray *b, int bsize);

 protected:
  vtkPCAAnalysisFilter();
  ~vtkPCAAnalysisFilter();

  // Description:
  // Usual data generation method.
  void Execute();

  // Description:
  // A wrapper function for accessing the inputs within the filter and subclasses.
  vtkPointSet* GetInput(int idx);

 private:
  vtkPCAAnalysisFilter(const vtkPCAAnalysisFilter&);  // Not implemented.
  void operator=(const vtkPCAAnalysisFilter&);  // Not implemented.

  // Eigenvalues
  vtkFloatArray *Evals;

  // Matrix where each column is an eigenvector
  double **evecMat2;

  // The mean shape in a vector
  double *meanshape;
};

#endif


