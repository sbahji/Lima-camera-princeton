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

#include <map>
#include "PrincetonException.h"

std::string _GetEnumString(PicamEnumeratedType type, piint value )
{
  const pichar* string;
  if( Picam_GetEnumerationString( type, value, &string ) == PicamError_None )
    {
      std::string s( string );
      Picam_DestroyString( string );
      return s;
    }
  return std::string();
}

std::string get_error_message(PicamError error)
{
  std::string msg = _GetEnumString(PicamEnumeratedType_Error,error);
  return msg.empty() ? std::string("Unkown Error") : msg;
}

std::string get_human_cam_model(PicamModel model)
{
  std::string msg = _GetEnumString(PicamEnumeratedType_Model,model);
  return msg.empty() ? std::string("Unkown Model") : msg;
}

std::string get_human_computer_interface(PicamComputerInterface interface)
{
  std::string msg = _GetEnumString(PicamEnumeratedType_ComputerInterface,interface);
  return msg.empty() ? std::string("Unkown interface") : msg;
}

std::map<PicamHandle,void*> handle2_user_data;
void register_user_pointer(PicamHandle handle,void* user_data)
{
  handle2_user_data.insert({handle,user_data});
}

void unregister_user_pointer(PicamHandle handle)
{
  std::map<PicamHandle,void*>::iterator search_iterator = handle2_user_data.find(handle);
  if(search_iterator != handle2_user_data.end())
  {
    handle2_user_data.erase(search_iterator);
  }
}

void* get_user_pointer(PicamHandle handle)
{
  std::map<PicamHandle,void*>::iterator search_iterator = handle2_user_data.find(handle);
  return search_iterator != handle2_user_data.end() ? search_iterator->second : NULL;
}
