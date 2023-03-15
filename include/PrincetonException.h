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
#ifndef PRINCETONEXCEPTION_H
#define PRINCETONEXCEPTION_H

#include <picam.h>
#include <picam_advanced.h>
#include <string>
#include "lima/Exceptions.h"

struct Error2Msg
{
  PicamError	error;
  const char*	msg;
};

std::string get_error_message(PicamError error);
std::string get_human_cam_model(PicamModel);
std::string get_human_computer_interface(PicamComputerInterface);

void register_user_pointer(PicamHandle,void*);
void unregister_user_pointer(PicamHandle);
void* get_user_pointer(PicamHandle);

//TODO SBA: remove comments
#define CHECK_PICAM(status)				\
if(status != PicamError_None) {					\
  std::cout << "SBA - status = " << status << std::endl;
  std::cout << "SBA - get_error_message(status) = " << get_error_message(status) << std::endl;
  THROW_HW_ERROR(Error) << get_error_message(status);		\
 }

#endif // PRINCETONEXCEPTION_H

