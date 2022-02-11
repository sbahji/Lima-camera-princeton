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
#ifndef PRINCETONSHUTTER_H
#define PRINCETONSHUTTER_H

#include "lima/HwShutterCtrlObj.h"

#include "PrincetonInterface.h"

namespace lima
{
  namespace Princeton
  {
    class PRINCETON_EXPORT ShutterCtrlObj : public HwShutterCtrlObj
    {
      DEB_CLASS_NAMESPC(DebModCamera, "PrincetonShutter", "Princeton");

    public:
      ShutterCtrlObj(PicamHandle cam);
      virtual ~ShutterCtrlObj();
      
      virtual bool checkMode(ShutterMode shut_mode) const;
      virtual void getModeList(ShutterModeList&  mode_list) const;
      virtual void setMode(ShutterMode  shut_mode);
      virtual void getMode(ShutterMode& shut_mode) const;

      virtual void setState(bool  shut_open);
      virtual void getState(bool& shut_open) const;

      virtual void setOpenTime (double  shut_open_time);
      virtual void getOpenTime (double& shut_open_time) const;
      virtual void setCloseTime(double  shut_close_time);
      virtual void getCloseTime(double& shut_close_time) const;

    private:
      PicamHandle m_cam;
    };
  }
}
#endif
