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

struct skp2ess_set
{
	EXP_TYPE exposure_type;
	EH_RenderQuality opt_quality;
	skp2ess_set()
	{
		exposure_type = ET_DAY;
		opt_quality = EH_DEFAULT;
	}
};

extern skp2ess_set g_skp2ess_set;