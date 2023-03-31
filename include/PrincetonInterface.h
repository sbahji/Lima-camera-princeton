//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2020
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
#ifndef PRINCETONINTERFACE_H
#define PRINCETONINTERFACE_H
#include <string>
#include <list>

#include "PrincetonCompatibility.h"
#include <picam.h>
#include <picam_advanced.h>

#include "lima/HwInterface.h"

namespace lima
{
  namespace Princeton
  {
    class DetInfoCtrlObj;
    class SyncCtrlObj;
    class BinCtrlObj;
    class RoiCtrlObj;
    class ShutterCtrlObj;
    
    struct Process
    {
      unsigned int	id;
      std::string	name;
    };

    PicamError AcquisitionUpdatedCallback(PicamHandle device,
					  const PicamAvailableData* available,
					  const PicamAcquisitionStatus* status);

    class PRINCETON_EXPORT Interface : public HwInterface
    {
      DEB_CLASS_NAMESPC(DebModCamera, "PrincetonInterface", "Princeton");
    
    public:
      enum Status {Ready, Running, Fault};
      enum TemperatureStatus {Temp_Fault,Temp_Locked,Temp_Unlocked};
      enum GainType {Gain_Fault, Gain_Low, Gain_Medium, Gain_High};

      Interface(const std::string& camera_serial = "");
      virtual ~Interface();
      //- From HwInterface
      virtual void	getCapList(CapList&) const;
      virtual void	reset(ResetLevel reset_level);
      virtual void	prepareAcq();
      virtual void	startAcq();
      virtual void	stopAcq();
      virtual void	getStatus(StatusType& status);
      virtual int	getNbHwAcquiredFrames();


      void newFrameReady(const PicamAvailableData* available, const PicamAcquisitionStatus* status);

      //Temperature management - temperature attribute
      float getSensorTemperature() const; ///< current sensor temperature
      Interface::TemperatureStatus getSensorTemperatureStatus() const; ///< get regulation status

      // Get/Set temperature setpoint regulation - temperatureTarget attribute
      float getSensorTemperatureSetpoint() const;
      void setSensorTemperatureSetpoint(float temp);

      // gain attribute
      Interface::GainType Interface::getAdcAnalogGain() const;
      //void setAdcAnalogGain(int gain);
      void setAdcAnalogGain(Interface::GainType gain);

      //adcRate attribute
      float getAdcSpeed();
      void setAdcSpeed(float adcSpeed);

    private:
      void _freePixelBuffer();

      bool			m_sdk_initialized;
      const PicamCameraID*	m_available_camera;
      piint			m_available_camera_number;
      PicamHandle		m_cam;
      DetInfoCtrlObj*		m_det_info;
      SyncCtrlObj*		m_sync;
      BinCtrlObj*		m_bin;
      RoiCtrlObj*		m_roi;
      ShutterCtrlObj*           m_shutter;
      
      SoftBufferCtrlObj		m_buffer_ctrl_obj;
      CapList			m_cap_list;

      std::string		m_cam_name;
      int			m_acq_frames;
      Status			m_status;
      PicamAcquisitionBuffer	m_pixel_stream;	// double buffer
      piint			m_readout_stride;
      piint 			m_frames_per_readout;
      piint			m_frame_stride;
      piint 			m_frame_size;
      Cond			m_cond;
    };
  
} // namespace Princeton
} // namespace lima

#endif // PRINCETONINTERFACE_H
