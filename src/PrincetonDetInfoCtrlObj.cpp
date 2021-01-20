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

#include "PrincetonDetInfoCtrlObj.h"
#include "PrincetonException.h"

using namespace lima;
using namespace lima::Princeton;

DetInfoCtrlObj::DetInfoCtrlObj(PicamHandle cam) : m_cam(cam)
{
  DEB_CONSTRUCTOR();

  // Detector width
  piint width;
  CHECK_PICAM(Picam_GetParameterIntegerValue(m_cam,
					     PicamParameter_ActiveWidth,
					     &width));
  m_max_columns = width;
  // Detector height
  piint height;
  CHECK_PICAM(Picam_GetParameterIntegerValue(m_cam,
					     PicamParameter_ActiveHeight,
					     &height));
  m_max_rows = height;
}

DetInfoCtrlObj::~DetInfoCtrlObj()
{
}

void DetInfoCtrlObj::getMaxImageSize(Size& max_image_size)
{
  max_image_size = Size(m_max_columns,m_max_rows);
}

void DetInfoCtrlObj::getDetectorImageSize(Size& det_image_size)
{
  getMaxImageSize(det_image_size);
}

void DetInfoCtrlObj::getDefImageType(ImageType& det_image_type)
{
  /* Most Princeton camera are 16bits Mono
     but some can handle other format... Not managed.
  */
  det_image_type = Bpp16;
}

void DetInfoCtrlObj::getCurrImageType(ImageType& curr_image_type)
{
  curr_image_type = Bpp16;
}

void DetInfoCtrlObj::setCurrImageType(ImageType curr_image_type)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(curr_image_type);

  if(curr_image_type != Bpp16)
    THROW_HW_ERROR(Error) << "Only support 16bits image";
}

void DetInfoCtrlObj::getPixelSize(double& x_size,double &y_size)
{
  DEB_MEMBER_FUNCT();
  
  piflt width;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_PixelWidth,
						   &width));
  piflt height;
  CHECK_PICAM(Picam_GetParameterFloatingPointValue(m_cam,
						   PicamParameter_PixelHeight,
						   &height));
  
  x_size = width * 1e-6;
  y_size = height * 1e-6;
}

void DetInfoCtrlObj::getDetectorType(std::string& det_type)
{
  DEB_MEMBER_FUNCT();
  det_type = "Princeton";
}

void DetInfoCtrlObj::getDetectorModel(std::string& det_model)
{
  DEB_MEMBER_FUNCT();
  PicamCameraID cam_id;
  CHECK_PICAM(Picam_GetCameraID(m_cam,&cam_id));
  std::string model = get_human_cam_model(cam_id.model);
  std::string computer_interface = get_human_computer_interface(cam_id.computer_interface);
  const char* sensor_name = cam_id.sensor_name;
  const char* serial_number = cam_id.serial_number;
  det_model = model + " " + computer_interface + " <" + sensor_name +">" +" (" + serial_number + ")";
}

void DetInfoCtrlObj::registerMaxImageSizeCallback(HwMaxImageSizeCallback& cb)
{
  m_mis_cb_gen.registerMaxImageSizeCallback(cb);
}

void DetInfoCtrlObj::unregisterMaxImageSizeCallback(HwMaxImageSizeCallback& cb)
{
  m_mis_cb_gen.unregisterMaxImageSizeCallback(cb);
}
