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

#include "PrincetonSyncCtrlObj.h"
#include "PrincetonException.h"

using namespace lima;
using namespace lima::Princeton;


SyncCtrlObj::SyncCtrlObj(PicamHandle cam) :
  m_cam(cam),m_trig_mode(IntTrig)
{
  DEB_CONSTRUCTOR();
  //Get trigger source capability
  const PicamCollectionConstraint* trigger_capability;
  CHECK_PICAM(Picam_GetParameterCollectionConstraint(m_cam,
						     PicamParameter_TriggerResponse,
						     PicamConstraintCategory_Capable,
						     &trigger_capability));
  for(int i = 0;i < trigger_capability->values_count;++i)
    {
      piint value = piint(trigger_capability->values_array[i]);
      switch(value)
	{
	case PicamTriggerResponse_ExposeDuringTriggerPulse:
	  m_trigger_capability.push_back(ExtGate);
	  m_trigger_capability.push_back(ExtStartStop);
	  break;
	case PicamTriggerResponse_GatePerTrigger:
	  m_trigger_capability.push_back(ExtTrigMult);
	  break;
	case PicamTriggerResponse_ReadoutPerTrigger:
	  m_trigger_capability.push_back(ExtTrigReadout);
	  break;
	case PicamTriggerResponse_StartOnSingleTrigger:
	  m_trigger_capability.push_back(ExtTrigSingle);
	  break;
	default:
	  break;
	}
    }
  CHECK_PICAM(Picam_DestroyCollectionConstraints(trigger_capability));
  // Initialization.
  setTrigMode(IntTrig);		
  setNbHwFrames(1);
  setExpTime(1);
}

SyncCtrlObj::~SyncCtrlObj()
{
}

bool SyncCtrlObj::checkTrigMode(TrigMode mode)
{
  DEB_MEMBER_FUNCT();
  bool valid_mode;
  switch(mode)
    {
    case IntTrig:
    case IntTrigMult:
      valid_mode = true;
      break;
    default:
      valid_mode = false;
      for(auto trigger = m_trigger_capability.begin();
	  !valid_mode && trigger != m_trigger_capability.end();++trigger)
	valid_mode = *trigger == mode;
      break;
    }
  return valid_mode;
}

  
void SyncCtrlObj::setTrigMode(TrigMode trig_mode)
{
  DEB_MEMBER_FUNCT();
  
  switch(trig_mode)
    {
    case IntTrig:
    case IntTrigMult:
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerResponse,
						 PicamTriggerResponse_NoResponse));
      break;
    case ExtGate:
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerResponse,
						 PicamTriggerResponse_ExposeDuringTriggerPulse));
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerDetermination,
						 PicamTriggerDetermination_PositivePolarity));
      break;
    case ExtStartStop:
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerResponse,
						 PicamTriggerResponse_ExposeDuringTriggerPulse));
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerDetermination,
						 PicamTriggerDetermination_RisingEdge));
      break;
    case ExtTrigReadout:
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerResponse,
						 PicamTriggerResponse_ReadoutPerTrigger));
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerDetermination,
						 PicamTriggerDetermination_RisingEdge));
      break;
    case ExtTrigSingle:
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerResponse,
						 PicamTriggerResponse_StartOnSingleTrigger));
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerDetermination,
						 PicamTriggerDetermination_RisingEdge));
      break;
    case ExtTrigMult:
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerResponse,
						 PicamTriggerResponse_GatePerTrigger));
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerDetermination,
						 PicamTriggerDetermination_RisingEdge));
      break;
    default:
      THROW_HW_ERROR(Error) << "Trigger: " << trig_mode << " not managed!";
      break;
    }
  
  pibln exists;
  CHECK_PICAM(Picam_DoesParameterExist(m_cam,
				       PicamParameter_TriggerSource,
				       &exists));
  if(exists)
    {
      PicamTriggerSource source =  (trig_mode == IntTrig || trig_mode == IntTrigMult) ?
	PicamTriggerSource_Internal : PicamTriggerSource_External;
      CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TriggerSource,source));
    }
  m_trig_mode = trig_mode;
}

void SyncCtrlObj::getTrigMode(TrigMode& trig_mode)
{
  trig_mode = m_trig_mode;
}

void SyncCtrlObj::setExpTime(double exp_time)
{
  DEB_MEMBER_FUNCT();
  exp_time *= 1e3;		// ms
  CHECK_PICAM(Picam_SetParameterFloatingPointValue(m_cam,
						   PicamParameter_ExposureTime,
						   exp_time));
}

void SyncCtrlObj::getExpTime(double &exp_time)
{
  DEB_MEMBER_FUNCT();
  piflt float_exp_time;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_ExposureTime,
						   &float_exp_time));
  exp_time = float_exp_time / 1e3;
}

void SyncCtrlObj::setLatTime(double lat_time)
{
  //Not managed
}

void SyncCtrlObj::getLatTime(double& lat_time)
{
  lat_time = 0;
}

void SyncCtrlObj::setNbHwFrames(int nb_frames)
{
  DEB_MEMBER_FUNCT();
  CHECK_PICAM(Picam_SetParameterLargeIntegerValue(m_cam,PicamParameter_ReadoutCount,
						  nb_frames));
  m_acq_nb_frames = nb_frames;
}

void SyncCtrlObj::getNbHwFrames(int& nb_frames)
{
  nb_frames = m_acq_nb_frames;
}
void SyncCtrlObj::getValidRanges(ValidRangesType& valid_ranges)
{
  DEB_MEMBER_FUNCT();
  const PicamRangeConstraint* constraint;
  CHECK_PICAM(Picam_GetParameterRangeConstraint(m_cam,
						PicamParameter_ExposureTime,
						PicamConstraintCategory_Capable,
						&constraint));
  double min_expo = constraint->minimum;
  double max_expo = constraint->maximum;
  CHECK_PICAM(Picam_DestroyRangeConstraints(constraint));
  
  valid_ranges.min_exp_time = min_expo;
  valid_ranges.max_exp_time = max_expo;
  valid_ranges.min_lat_time = 0.;
  valid_ranges.max_lat_time = 0.;
}

