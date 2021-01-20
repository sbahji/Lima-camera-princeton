###########################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2020
# European Synchrotron Radiation Facility
# CS40220 38043 Grenoble Cedex 9
# FRANCE
#
# Contact: lima@esrf.fr
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
###########################################################################

set(PICAM_INCLUDE_DIRS)
set(PICAM_LIBRARIES)
set(PICAM_DEFINITIONS)

find_path(PICAM_INCLUDE_DIRS
  NAMES picam.h
  HINTS "/opt/PrincetonInstruments/picam/includes"
  HINTS "/Program Files/Princeton Instruments/PICam/Includes"
  )
find_library(PICAM_LIBRARIES
  NAMES picam
  HINTS /opt/pleora/ebus_sdk/x86_64/lib
  HINTS "/Program Files/Princeton Instruments/PICam/Libraries"
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Picam DEFAULT_MSG
  PICAM_LIBRARIES
  PICAM_INCLUDE_DIRS
)
