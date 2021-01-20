//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################

#include "PrincetonRoiCtrlObj.h"
#include "PrincetonException.h"

using namespace lima;
using namespace lima::Princeton;

RoiCtrlObj::RoiCtrlObj(PicamHandle cam, BinCtrlObj& bin) :
  m_cam(cam),
  m_bin(bin)
{
  DEB_CONSTRUCTOR();
  const PicamRoisConstraint  *constraint;
  CHECK_PICAM(Picam_GetParameterRoisConstraint(m_cam, 
					       PicamParameter_Rois, 
					       PicamConstraintCategory_Required, 
					       &constraint));
  /* Get width and height from constraints */
  m_rows  = (piint)constraint->width_constraint.maximum;
  m_columns = (piint)constraint->height_constraint.maximum;
  m_rules = constraint->rules;
  Picam_DestroyRoisConstraints(constraint);

  //Init roi to full frame
  setRoi({0,0,0,0});
}

RoiCtrlObj::~RoiCtrlObj()
{
}

void RoiCtrlObj::setRoi(const Roi& set_roi)
{
  DEB_MEMBER_FUNCT();
  if(set_roi.isActive())
    {
      Bin bin;
      m_bin.getBin(bin);
      Point top_left = set_roi.getTopLeft();
      Size size = set_roi.getSize();
      top_left *= bin;
      size *= bin;

      //Set Bin and roi
      PicamRois rois;
      PicamRoi roi{top_left.x,size.getWidth(),bin.getX(),
		   top_left.y,size.getHeight(),bin.getY()};
      rois.roi_count = 1;
      rois.roi_array = &roi;
      CHECK_PICAM(Picam_SetParameterRoisValue(m_cam, PicamParameter_Rois, &rois));
      m_roi = set_roi;
    }
  else				// full frame
    m_roi = Roi(0,0,m_rows,m_columns);
}

void RoiCtrlObj::getRoi(Roi &hw_roi)
{
  hw_roi = m_roi;
}

void RoiCtrlObj::checkRoi(const Roi& set_roi, Roi& hw_roi)
{
  if((m_rules &  PicamRoisConstraintRulesMask_HorizontalSymmetry) ||
     (m_rules & PicamRoisConstraintRulesMask_VerticalSymmetry))
    {
      // Not managed
      hw_roi = Roi(0,0,m_rows,m_columns);
    }
  else
    {
      // not limit ;)
      hw_roi = set_roi;
    }
}
