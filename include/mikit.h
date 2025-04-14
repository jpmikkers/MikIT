/*

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

