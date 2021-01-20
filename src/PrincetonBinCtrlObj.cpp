//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2020
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

#include "PrincetonBinCtrlObj.h"
#include "PrincetonException.h"

using namespace lima;
using namespace lima::Princeton;

BinCtrlObj::BinCtrlObj(PicamHandle cam):
  m_cam(cam)
{
  DEB_CONSTRUCTOR();
  const PicamRoisConstraint  *constraint;
  CHECK_PICAM(Picam_GetParameterRoisConstraint(m_cam, 
					       PicamParameter_Rois, 
					       PicamConstraintCategory_Required, 
					       &constraint));
  for( piint i = 0; i < constraint->x_binning_limits_count; ++i )
    m_possible_xbin.push_back(constraint->x_binning_limits_array[i]);
  for( piint i = 0; i < constraint->y_binning_limits_count; ++i )
    m_possible_ybin.push_back(constraint->y_binning_limits_array[i]);
  Picam_DestroyRoisConstraints(constraint);
}

BinCtrlObj::~BinCtrlObj()
{
}

void BinCtrlObj::setBin(const Bin& bin)
{
  m_bin = bin;
}
void BinCtrlObj::getBin(Bin& bin)
{
  bin = m_bin;
}
void BinCtrlObj::checkBin(Bin& bin)
{
  int x_bin = 1;
  int y_bin = 1;
  int request_xbin = bin.getX();
  int request_ybin = bin.getY();

  if(!m_possible_xbin.empty())
    {
      for(auto poss_xbin = m_possible_xbin.begin();
	  poss_xbin != m_possible_xbin.end() && request_ybin >= *poss_xbin;
	  ++poss_xbin)
	x_bin = *poss_xbin;
    }

    if(!m_possible_ybin.empty())
    {
      for(auto poss_ybin = m_possible_ybin.begin();
	  poss_ybin != m_possible_ybin.end() && request_ybin >= *poss_ybin;
	  ++poss_ybin)
	y_bin = *poss_ybin;
    }
    bin = Bin(x_bin,y_bin);
}
