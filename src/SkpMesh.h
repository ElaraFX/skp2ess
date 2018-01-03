#pragma once

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/vertex.h>

#include "ElaraHomeAPI.h"

bool skp_to_ess(const char *skp_file_name, EH_Context *ctx);

enum EXP_TYPE{
	ET_DAY = 0,
	ET_NIGHT,
	ET_DAY_OUTWORLD,

	ET_MAX
};

struct envi_set
{
	EXP_TYPE exposure_type;
	envi_set()
	{
		exposure_type = ET_DAY;
	}
};

extern envi_set g_envi_set;