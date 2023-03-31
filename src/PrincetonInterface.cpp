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

#include <cmath>

#include "PrincetonInterface.h"
#include "PrincetonDetInfoCtrlObj.h"
#include "PrincetonSyncCtrlObj.h"
#include "PrincetonBinCtrlObj.h"
#include "PrincetonRoiCtrlObj.h"
#include "PrincetonShutterCtrlObj.h"
#include "PrincetonException.h"

using namespace lima;
using namespace lima::Princeton;

#include "processlib/SinkTask.h"
#include "processlib/TaskMgr.h"
 
// _StopAcq
class _StopAcq : public SinkTaskBase
{
public:
  _StopAcq(Interface &anInterface) : m_interface(anInterface) {}
  virtual ~_StopAcq() {}
  virtual void process(Data&)
  {
    m_interface.stopAcq();
  }
private:
  Interface& m_interface;
};
//Callback
PicamError Princeton::AcquisitionUpdatedCallback(PicamHandle cam,
						 const PicamAvailableData* available,
						 const PicamAcquisitionStatus* status)
{
  Interface *interface = (Interface*)get_user_pointer(cam);
  if(!interface)
    {
      std::cerr << "Something weird happen ;)" << std::endl;
      return PicamError_UnexpectedError;
    }

  interface->newFrameReady(available,status);

  return PicamError_None;
}

Interface::Interface(const std::string& camera_serial) :
  m_sdk_initialized(false),
  m_available_camera(NULL),
  m_available_camera_number(0),
  m_cam(NULL),
  m_status(Ready),
  m_det_info(NULL),
  m_sync(NULL), 
  m_bin(NULL),
  m_roi(NULL),
  m_shutter(NULL)
{
  DEB_CONSTRUCTOR();
  m_pixel_stream = {NULL,0};
  piint major, minor, distribution, released;
  CHECK_PICAM(Picam_GetVersion(&major, &minor, &distribution, &released));

  DEB_ALWAYS() << "PICam version " << major << "." << minor << "." << distribution
	       << " (" << released << ")";
  pibln inited;
  CHECK_PICAM(Picam_IsLibraryInitialized(&inited));

  if(inited)
    THROW_HW_ERROR(Error) << "Something weird happen, the library is already initialized";

  DEB_ALWAYS() << "Initialize PICam library";
  CHECK_PICAM(Picam_InitializeLibrary());
  m_sdk_initialized = true;
  
  CHECK_PICAM(Picam_GetAvailableCameraIDs(&m_available_camera,
					  &m_available_camera_number));
  if (m_available_camera_number == 0)
    THROW_HW_ERROR(Error) << "No cameras found in the system\n";

  // Try to find the camera.
  // if camera_serial is empty, open the first found.
  bool found = false;
  for(int i = 0;i < m_available_camera_number;++i)
    {
      const PicamCameraID* cam_id = &m_available_camera[i];
      
      if(camera_serial.empty() || camera_serial == cam_id->serial_number)
	{
	  CHECK_PICAM(PicamAdvanced_OpenCameraDevice(cam_id, &m_cam));
	  std::string model = get_human_cam_model(cam_id->model);
	  std::string computer_interface = get_human_computer_interface(cam_id->computer_interface);
	  const char* sensor_name = cam_id->sensor_name;
	  const char* serial_number = cam_id->serial_number;

	  DEB_ALWAYS() << "Connected to camera "
		       << DEB_VAR4(model,computer_interface,sensor_name,serial_number);
	  break;
	}
    }

  if(!m_cam)
    {
      DEB_ALWAYS() << "Cameras found:";
      for(int i = 0;i < m_available_camera_number;++i)
	{
	  const PicamCameraID* cam_id = &m_available_camera[i];
	  std::string model = get_human_cam_model(cam_id->model);
	  DEB_ALWAYS() << DEB_VAR1(model);

	  std::string computer_interface = get_human_computer_interface(cam_id->computer_interface);
	  DEB_ALWAYS() << DEB_VAR1(computer_interface);

	  const char* sensor_name = cam_id->sensor_name;
	  const char* serial_number = cam_id->serial_number;
	  DEB_ALWAYS() << DEB_VAR2(sensor_name,serial_number);
	  DEB_ALWAYS() << "##############################";
	}
      THROW_HW_ERROR(Error) << "Camera with "
			    << DEB_VAR1(camera_serial) << " is not found!";
    }

  //enable metadata
  piint ts_mask = PicamTimeStampsMask_ExposureStarted | PicamTimeStampsMask_ExposureEnded;
  CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,PicamParameter_TimeStamps,ts_mask));
  
  register_user_pointer(m_cam,this);
  CHECK_PICAM(PicamAdvanced_RegisterForAcquisitionUpdated(m_cam,Princeton::AcquisitionUpdatedCallback));
  
  // HW Caps
  m_det_info = new DetInfoCtrlObj(m_cam);
  m_sync = new SyncCtrlObj(m_cam);
  m_bin = new BinCtrlObj(m_cam);
  m_roi = new RoiCtrlObj(m_cam,*m_bin);
  m_shutter = new ShutterCtrlObj(m_cam);
  
  // Cap list
  m_cap_list.push_back(HwCap(m_det_info));
  m_cap_list.push_back(HwCap(m_sync));
  //m_cap_list.push_back(HwCap(m_bin));
  //m_cap_list.push_back(HwCap(m_roi));
  m_cap_list.push_back(HwCap(m_shutter));
  m_cap_list.push_back(HwCap(&m_buffer_ctrl_obj));
}

Interface::~Interface()
{
  DEB_DESTRUCTOR();

  delete m_det_info;
  delete m_sync;
  delete m_bin;
  delete m_roi;
  delete m_shutter;
  
  unregister_user_pointer(m_cam);
  
  if(m_cam)
    PicamAdvanced_CloseCameraDevice(m_cam);

  if(m_available_camera)
    Picam_DestroyCameraIDs(m_available_camera);
  
  if(m_sdk_initialized)
    Picam_UninitializeLibrary();

  _freePixelBuffer();

}


void Interface::getCapList(CapList &cap_list) const
{
  cap_list = m_cap_list;
}

void Interface::reset(ResetLevel reset_level)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(reset_level);
}

void Interface::prepareAcq()
{
  DEB_MEMBER_FUNCT();
  m_acq_frames = -1;
  // - get the current readout rate
  // - note this accounts for rate increases in online scenarios
  piflt onlineReadoutRate;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_OnlineReadoutRateCalculation,
						   &onlineReadoutRate));

  // - get the current readout stride
  piint readoutStride;
  CHECK_PICAM(Picam_GetParameterIntegerValue(m_cam,
					     PicamParameter_ReadoutStride,
					     &readoutStride));
  // - calculate the buffer size
  pi64s readouts = std::ceil((std::max)(3.*onlineReadoutRate,2.));
  long exp_bytes = readoutStride * readouts;
  if(exp_bytes != m_pixel_stream.memory_size)
    {
      _freePixelBuffer();
#ifdef __unix
      if(posix_memalign(&m_pixel_stream.memory,16,exp_bytes))
#else  /* window */
	m_pixel_stream.memory = _aligned_malloc(exp_bytes,16);
      if(!m_pixel_stream.memory)
#endif
	THROW_HW_ERROR(Error) << "Can't allocate double buffer";
  
      m_pixel_stream.memory_size=exp_bytes;
      CHECK_PICAM(PicamAdvanced_SetAcquisitionBuffer(m_cam,&m_pixel_stream));
    }

  pibln committed;
  CHECK_PICAM(Picam_AreParametersCommitted(m_cam,&committed));
  if(!committed)
    {
      PicamHandle model;
      CHECK_PICAM(PicamAdvanced_GetCameraModel(m_cam,&model));

      CHECK_PICAM(PicamAdvanced_CommitParametersToCameraDevice(model));
    }

  // Cache values for data reading
  CHECK_PICAM(Picam_GetParameterIntegerValue(m_cam,PicamParameter_ReadoutStride,
					     &m_readout_stride));
  CHECK_PICAM(Picam_GetParameterIntegerValue(m_cam,
					     PicamParameter_FrameStride,
					     &m_frame_stride));
  CHECK_PICAM(Picam_GetParameterIntegerValue(m_cam,
					     PicamParameter_FramesPerReadout,
					     &m_frames_per_readout));
  CHECK_PICAM(Picam_GetParameterIntegerValue(m_cam,
					     PicamParameter_FrameSize,
					     &m_frame_size));
  m_status = Ready;
}


void Interface::startAcq()
{
  DEB_MEMBER_FUNCT();
  m_buffer_ctrl_obj.getBuffer().setStartTimestamp(Timestamp::now());
  AutoMutex lock(m_cond.mutex());
  CHECK_PICAM(Picam_StartAcquisition(m_cam));
  // Change acquisition status.
  m_status = Running;
}

 
void Interface::stopAcq()
{
  DEB_MEMBER_FUNCT();
  CHECK_PICAM(Picam_StopAcquisition(m_cam));
  // Wait acquisition stop
  AutoMutex lock(m_cond.mutex());
  while(m_status != Ready &&
	m_status != Fault)
    m_cond.wait();
}

void Interface::getStatus(StatusType& status)
{
  DEB_MEMBER_FUNCT();
  switch(m_status)
    {
    case Ready:
      status.set(HwInterface::StatusType::Ready);
      break;
    case Running:
      status.set(HwInterface::StatusType::Exposure);
      break;
    default:
      status.set(HwInterface::StatusType::Fault);
      break;
    }
  DEB_RETURN() << DEB_VAR1(status);
}

int Interface::getNbHwAcquiredFrames()
{
  DEB_MEMBER_FUNCT();
  return m_acq_frames;
}

void Interface::newFrameReady(const PicamAvailableData* available,
			      const PicamAcquisitionStatus* status)
{
  DEB_MEMBER_FUNCT();
  // Read data if any
  if(available && available->readout_count)
    {
      StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_obj.getBuffer();
      for(int i = 0;i < available->readout_count;++i)
	{
	  pibyte* first_framePt = (pibyte*)available->initial_readout;
	  first_framePt += m_readout_stride * i;
	  for(int fid = 0;fid < m_frames_per_readout;++fid)
	    {
	      pibyte *src_framePt = first_framePt + m_frame_stride * fid;
	      void* framePt = buffer_mgr.getFrameBufferPtr(++m_acq_frames);
	      memcpy(framePt,src_framePt,m_frame_size);
	      HwFrameInfoType frame_info;
	      frame_info.acq_frame_nb = m_acq_frames;
	      bool continueAcq = buffer_mgr.newFrameReady(frame_info);
	      if(!continueAcq)
		{
		  _StopAcq *aStopAcqPt = new _StopAcq(*this);
		  TaskMgr *mgr = new TaskMgr();
		  mgr->addSinkTask(0,aStopAcqPt);
		  aStopAcqPt->unref();

		  PoolThreadMgr::get().addProcess(mgr);
		}
	    }
	}
    }
  // Acquisition status
  bool running = status->running;
  bool errors = bool(status->errors);
  AutoMutex lock(m_cond.mutex());
  if(m_status != Fault)
    {
      if(errors)
	m_status = Fault;
      else
	m_status = running ? Running : Ready;
    }
  m_cond.broadcast();
}

float Interface::getSensorTemperature() const
{
  DEB_MEMBER_FUNCT();
  piflt sensor_temp;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_SensorTemperatureReading,
						   &sensor_temp));
  return sensor_temp;
}

Interface::TemperatureStatus Interface::getSensorTemperatureStatus() const
{
  DEB_MEMBER_FUNCT();
  piint status;
  CHECK_PICAM(Picam_GetParameterIntegerValue(m_cam,
					     PicamParameter_SensorTemperatureStatus,
					     &status));
  Interface::TemperatureStatus temp_status;
  switch(status)
    {
    case PicamSensorTemperatureStatus_Locked:
      temp_status = Interface::TemperatureStatus::Temp_Locked;break;
    case PicamSensorTemperatureStatus_Unlocked:
      temp_status = Interface::TemperatureStatus::Temp_Unlocked;break;
    case PicamSensorTemperatureStatus_Faulted:
    default:
      temp_status = Interface::TemperatureStatus::Temp_Fault;break;
    }
  return temp_status;
}

float Interface::getSensorTemperatureSetpoint() const
{
  DEB_MEMBER_FUNCT();
  piflt setpoint_temp;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_SensorTemperatureSetPoint,
						   &setpoint_temp));
  return setpoint_temp;
}

void Interface::setSensorTemperatureSetpoint(float setpoint)
{
  DEB_MEMBER_FUNCT();
  piflt setpoint_temp = setpoint;
  CHECK_PICAM(Picam_SetParameterFloatingPointValue(m_cam,
						   PicamParameter_SensorTemperatureSetPoint,
						   setpoint_temp));
}

Interface::GainType Interface::getAdcAnalogGain() const
{
  DEB_MEMBER_FUNCT();
  piint gain_type_temp;
  CHECK_PICAM(Picam_GetParameterIntegerValue(m_cam,
						   PicamParameter_AdcAnalogGain,
						   &gain_type_temp));

  Interface::GainType gain_type;

	switch(gain_type_temp)
	{
		case PicamAdcAnalogGain_Low://1
		  gain_type = Interface::GainType::Gain_Low;
		  break;
		case PicamAdcAnalogGain_Medium://2
		  gain_type = Interface::GainType::Gain_Medium;
		  break;
		case PicamAdcAnalogGain_High://3
		  gain_type = Interface::GainType::Gain_High;
		  break;
		default:
		  gain_type = Interface::GainType::Gain_Fault;
		  break;
	}
  return gain_type;
}

void Interface::setAdcAnalogGain(Interface::GainType gain)
{
  DEB_MEMBER_FUNCT();
  piint gain_type_temp;

	switch(gain)
	{
		case Gain_Low://1
		  gain_type_temp = Interface::GainType::Gain_Low;
		  break;
		case Gain_Medium://2
		  gain_type_temp = Interface::GainType::Gain_Medium;
		  break;
		case Gain_High://3
		  gain_type_temp = Interface::GainType::Gain_High;
		  break;
	}

  CHECK_PICAM(Picam_SetParameterIntegerValue(m_cam,
						   PicamParameter_AdcAnalogGain,
						   gain_type_temp));
}

//adcRate
float Interface::getAdcSpeed()
{
  DEB_MEMBER_FUNCT();
  piflt adcSpeed_temp;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_AdcSpeed,
						   &adcSpeed_temp));
  return adcSpeed_temp;
}

void Interface::setAdcSpeed(float adcSpeed)
{
  DEB_MEMBER_FUNCT();
  piflt adcSpeed_temp = adcSpeed;
  CHECK_PICAM(Picam_SetParameterFloatingPointValue(m_cam,
						   PicamParameter_AdcSpeed,
						   adcSpeed_temp));
}

void Interface::_freePixelBuffer()
{
  if(m_pixel_stream.memory)
#ifdef __unix
    free(m_pixel_stream.memory);
#else
    _aligned_free(m_pixel_stream.memory);
#endif
  m_pixel_stream = {NULL,0};
}
