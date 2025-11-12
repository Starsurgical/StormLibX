
#ifndef __STORMLIBX_STORM_H__
#define __STORMLIBX_STORM_H__

#include "StormTypes.h"

#include <Storm/Big.hpp>
#include <Storm/Core.hpp>
#include <Storm/Error.hpp>
#include <Storm/Event.hpp>
#include <Storm/Memory.hpp>
#include <Storm/Region.hpp>
#include <Storm/String.hpp>

#include "SBlt.h"
#include "SBmp.h"
#include "SCmd.h"
#include "SCode.h"
#include "SComp.h"
#include "SDlg.h"
#include "SDraw.h"
#include "SFile.h"
#include "SGdi.h"
#include "SLog.h"
#include "SMsg.h"
#include "SNet.h"
#include "SReg.h"
#include "STrans.h"
#include "SUni.h"
#include "SVid.h"

extern "C" {
  void StormInitialize();

  // @301
  void StormDestroyWrapped();

  // @302
  HINSTANCE StormGetInstance();
}

#endif
