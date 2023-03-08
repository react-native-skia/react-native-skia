/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#define __RNS_PLUGIN_H_INSIDE__

#include <iostream>
#include <string>
#include <vector>

#include "Export.h"
#include "Version.h"

#include "ReactSkia/utils/RnsLog.h"

#ifndef ENABLE
#define ENABLE(RNS_FEATURE) (defined ENABLE_##RNS_FEATURE && ENABLE_##RNS_FEATURE)
#endif

#if ENABLE(RNSP_APP_MANAGER)
#include "ReactSkia/pluginfactory/interfaces/RNSApplicationManagerInterface.h"
#endif

#undef __RNS_PLUGIN_H_INSIDE__

