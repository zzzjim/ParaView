/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.1. 

   See License_v1.1.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#ifndef _pqElementInspectorWidget_h
#define _pqElementInspectorWidget_h

#include "pqComponentsExport.h"
#include <QWidget>

class pqPipelineSource;
class pqSelectionManager;
class pqServerManagerModelItem;
class vtkUnstructuredGrid;

/// Displays a collection of data set elements in a spreadsheet form.
/// pqElementInspectorWidget has dual role:
/// \li showing the data from the active selection
/// \li showing the data from a "ExtractCellSelection" 
//      or "ExtractPointSelection" filter.
class PQCOMPONENTS_EXPORT pqElementInspectorWidget :
  public QWidget
{
  Q_OBJECT
  
public:
  pqElementInspectorWidget(QWidget* parent);
  ~pqElementInspectorWidget();

  /// Set the selection manager. The selection manager is used to
  /// obtain the current user defined cell selection.
  void setSelectionManager(pqSelectionManager*);

signals:
  /// Signal emitted whenever the collection of elements changes
  void elementsChanged(vtkUnstructuredGrid*);

public slots:
  /// Called when user creates a new surface selection (or old 
  /// surface selection is cleared).
  void onSelectionChanged();

  /// Called to inspect the data generated by any source.
  /// Currently, we only show data from "ExtractPointSelection" or
  /// "ExtractCellSelection" filters.
  void inspect(pqPipelineSource* source);

  /// Refreshes the view.
  void refresh();

protected:
  /// Call this to set the collection of elements
  void setElements(vtkUnstructuredGrid* ug);

  void onElementsChanged();

  /// Updates the data for the CurrentSource.
  void onSourceChanged();

  /// Call this to clear the collection of elements
  void clear();

protected slots:
  void onCurrentTypeTextChanged(const QString& text);

  /// Called when the application selection changes.
  void onCurrentChanged(pqServerManagerModelItem*);

private:
  struct pqImplementation;
  pqImplementation* const Implementation;
};

#endif
