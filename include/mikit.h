/*

(c) Copyright 1996, 1997 Jean-Paul Mikkers 

This file is part of MikIT.

MikIT is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 of 
the License, or (at your option) any later version.

MikIT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public 
License along with MikIT. If not, see <http://www.gnu.org/licenses/>.

File:           MIKIT.H
Description:    -
Version:        1.00 - original

*/
#ifndef MIKIT_H
#define MIKIT_H

#include <stdio.h>
#include <stdlib.h>

#include "mtypes.h"
#include "minput.h"
#include "mdriver.h"

#ifdef MIKIT_WINDOWS
#include "mdrv_w95.h"
#endif

#ifdef MIKIT_LINUX
#include "mdrv_uss.h"
#endif

#ifdef MIKIT_FREEBSD
#include "mdrv_tim.h"
#include "mdrv_uss.h"
#endif

#include "mmodule.h"
#include "mmod_it.h"
#include "mmod_xm.h"
#include "mmod_mod.h"
#include "mmod_s3m.h"
#include "mmod_dcm.h"

#endif

