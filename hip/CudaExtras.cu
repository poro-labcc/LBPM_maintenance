/*
  Copyright 2013--2018 James E. McClure, Virginia Polytechnic & State University
  Copyright Equnior ASA

  This file is part of the Open Porous Media project (OPM).
  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

// Basic hip functions callable from C/C++ code
#include "hip/hip_runtime.h"

extern "C" void dvc_AllocateDeviceMemory(void** address, size_t size){
	hipMalloc(address,size);
    hipMemset(*address,0,size);
}

extern "C" void dvc_CopyToDevice(void* dest, void* source, size_t size){
	hipMemcpy(dest,source,size,hipMemcpyHostToDevice);
}


extern "C" void dvc_CopyToHost(void* dest, void* source, size_t size){
	hipMemcpy(dest,source,size,hipMemcpyDeviceToHost);
}

extern "C" void dvc_Barrier(){
	hipDeviceSynchronize();
}

