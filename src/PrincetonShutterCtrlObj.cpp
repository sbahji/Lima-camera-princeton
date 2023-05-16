//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2022
// European Synchrotron Radiation Facility
// CS40220 38043 Grenoble Cedex 9 
// FRANCE
//
// Contact: lima@esrf.fr
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
#include "PrincetonShutterCtrlObj.h"
#include "PrincetonException.h"

using namespace lima;
using namespace lima::Princeton;

ShutterCtrlObj::ShutterCtrlObj(PicamHandle cam):
  m_cam(cam)
{
}

ShutterCtrlObj::~ShutterCtrlObj()
{
}

bool ShutterCtrlObj::checkMode(ShutterMode shut_mode) const
{
  return shut_mode == ShutterAutoSequence ? false : true;
}

void ShutterCtrlObj::getModeList(ShutterModeList&  mode_list) const
{
  mode_list.push_back(ShutterManual);
  mode_list.push_back(ShutterAutoFrame);
}

void ShutterCtrlObj::setMode(ShutterMode  shut_mode)
{
  DEB_MEMBER_FUNCT();
  
  switch(shut_mode)
    {
    case ShutterManual:
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_ShutterTimingMode,
						 PicamShutterTimingMode_AlwaysClosed));
      break;
      
    default:
    case ShutterAutoFrame:
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_ShutterTimingMode,
						 PicamShutterTimingMode_Normal));
      break;
    }
}

void ShutterCtrlObj::getMode(ShutterMode& shut_mode) const {}

void ShutterCtrlObj::setState(bool  shut_open)
{
  DEB_MEMBER_FUNCT();
  
  if(shut_open)
    {
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_ShutterTimingMode,
						 PicamShutterTimingMode_AlwaysOpen));
    }
  else
    {
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_ShutterTimingMode,
						 PicamShutterTimingMode_AlwaysClosed));
    }
}

void ShutterCtrlObj::getState(bool& shut_open) const
{
  DEB_MEMBER_FUNCT();
  
  piint shut_timing_mode;
  CHECK_PICAM(Picam_GetParameterIntegerValue(m_cam,
					     PicamParameter_ShutterTimingMode,
					     &shut_timing_mode));
  shut_open = shut_timing_mode == PicamShutterTimingMode_AlwaysOpen ? true : false;
}

void ShutterCtrlObj::setOpenTime(double shut_open_time)
{
  DEB_MEMBER_FUNCT();
  
  piflt shutter_delay_resolution;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_ShutterDelayResolution,
						   &shutter_delay_resolution));
  shut_open_time *= 1e6 / shutter_delay_resolution;
  CHECK_PICAM(Picam_SetParameterFloatingPointValue(m_cam,PicamParameter_ShutterOpeningDelay,
						   shut_open_time));
}

void ShutterCtrlObj::getOpenTime(double& shut_open_time) const
{
  DEB_MEMBER_FUNCT();
  
  piflt shutter_delay_resolution;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_ShutterDelayResolution,
						   &shutter_delay_resolution));
  piflt raw_shutter_open_time;
  try
    {
      CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						       PicamParameter_ShutterOpeningDelay,
						       &raw_shutter_open_time));
    }
  catch(Exception)
    {
      raw_shutter_open_time=0;
    }
  shut_open_time = raw_shutter_open_time / 1e6 * shutter_delay_resolution;
}

void ShutterCtrlObj::setCloseTime(double shut_close_time)
{
  DEB_MEMBER_FUNCT();
  
  piflt shutter_delay_resolution;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_ShutterDelayResolution,
						   &shutter_delay_resolution));
  shut_close_time *= 1e6 / shutter_delay_resolution;
  CHECK_PICAM(Picam_SetParameterFloatingPointValue(m_cam,PicamParameter_ShutterClosingDelay,
						   shut_close_time));
}

void ShutterCtrlObj::getCloseTime(double& shut_close_time) const
{
  DEB_MEMBER_FUNCT();
  
  piflt shutter_delay_resolution;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_ShutterDelayResolution,
						   &shutter_delay_resolution));
  piflt raw_shutter_close_time;
  try
    {
      CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						       PicamParameter_ShutterOpeningDelay,
						       &raw_shutter_close_time));
    }
  catch(Exception)
    {
      raw_shutter_close_time = 0;
    }
  shut_close_time = raw_shutter_close_time / 1e6 * shutter_delay_resolution;
}
