/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
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

========================================================================*/
#include "pqOrbitCreatorDialog.h"
#include "ui_pqOrbitCreatorDialog.h"

#include "pqApplicationCore.h"
#include "pqServerManagerSelectionModel.h"
#include "vtkBoundingBox.h"
#include "vtkPoints.h"
#include "vtkSMUtilities.h"

#include <QDoubleValidator>


class pqOrbitCreatorDialog::pqInternals : public Ui::OrbitCreatorDialog
{
};

//-----------------------------------------------------------------------------
pqOrbitCreatorDialog::pqOrbitCreatorDialog(QWidget* parentObject)
  : Superclass(parentObject)
{
  this->Internals = new pqInternals();
  this->Internals->setupUi(this);

  this->Internals->center0->setValidator(new QDoubleValidator(this));
  this->Internals->center1->setValidator(new QDoubleValidator(this));
  this->Internals->center2->setValidator(new QDoubleValidator(this));

  this->Internals->normal0->setValidator(new QDoubleValidator(this));
  this->Internals->normal1->setValidator(new QDoubleValidator(this));
  this->Internals->normal2->setValidator(new QDoubleValidator(this));
  this->Internals->radius->setValidator(new QDoubleValidator(this));

  QObject::connect(this->Internals->resetBounds, SIGNAL(clicked()),
    this, SLOT(resetBounds()));

  this->resetBounds();
}

//-----------------------------------------------------------------------------
pqOrbitCreatorDialog::~pqOrbitCreatorDialog()
{
  delete this->Internals;
}

//-----------------------------------------------------------------------------
void pqOrbitCreatorDialog::setNormal(double xyz[3])
{
  this->Internals->normal0->setText(QString::number(xyz[0]));
  this->Internals->normal1->setText(QString::number(xyz[1]));
  this->Internals->normal2->setText(QString::number(xyz[2]));
}

//-----------------------------------------------------------------------------
void pqOrbitCreatorDialog::resetBounds()
{
  double input_bounds[6];
  if (pqApplicationCore::instance()->getSelectionModel()->
    getSelectionDataBounds(input_bounds))
    {
    vtkBoundingBox box;
    box.SetBounds(input_bounds);
    double box_center[3];
    box.GetCenter(box_center);
    box.Scale(5, 5, 5);
    this->Internals->center0->setText(QString::number(box_center[0]));
    this->Internals->center1->setText(QString::number(box_center[1]));
    this->Internals->center2->setText(QString::number(box_center[2]));
    this->Internals->normal0->setText("0");
    this->Internals->normal1->setText("1");
    this->Internals->normal2->setText("0");
    this->Internals->radius->setText(QString::number(box.GetMaxLength()/2.0));
    }
}

//-----------------------------------------------------------------------------
QList<QVariant> pqOrbitCreatorDialog::center() const
{
  QList<QVariant> value;
  value << this->Internals->center0->text().toDouble();
  value << this->Internals->center1->text().toDouble();
  value << this->Internals->center2->text().toDouble();
  return value;
}

//-----------------------------------------------------------------------------
QList<QVariant> pqOrbitCreatorDialog::orbitPoints(int resolution) const
{
  double box_center[3];
  double normal[3];
  double radius;

  box_center[0] = this->Internals->center0->text().toDouble();
  box_center[1] = this->Internals->center1->text().toDouble();
  box_center[2] = this->Internals->center2->text().toDouble();

  normal[0] = this->Internals->normal0->text().toDouble();
  normal[1] = this->Internals->normal1->text().toDouble();
  normal[2] = this->Internals->normal2->text().toDouble();

  radius = this->Internals->radius->text().toDouble();

  QList<QVariant> points;
  vtkPoints* pts = vtkSMUtilities::CreateOrbit(
    box_center, normal, radius, resolution);
  for (vtkIdType cc=0; cc < pts->GetNumberOfPoints(); cc++)
    {
    double coords[3];
    pts->GetPoint(cc, coords);
    points << coords[0] << coords[1] << coords[2];
    }
  pts->Delete();
  return points;
}


