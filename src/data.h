#pragma once
#include "ElaraHomeAPI.h"

#define MAX_MODEL_SCENES 64

enum EXP_TYPE{
	ET_DAY = 0,
	ET_NIGHT,
	ET_DAY_OUTWORLD,

	ET_MAX
};

enum CAMERA_TYPE{
	CT_NORMAL = 0,
	CT_CUBEMAP,
	CT_SPHERICAL,

	CT_MAX
};

struct skp2ess_set
{
	int camera_num;
	float exp_val;
	float enviroment_hdr_multipler;
	bool exp_val_on;
	bool cameras_index[MAX_MODEL_SCENES];
	EXP_TYPE exposure_type;
	EH_RenderQuality opt_quality;
	CAMERA_TYPE camera_type;
	std::string hdr_name;
	skp2ess_set()
	{
		camera_type = CT_NORMAL;
		exposure_type = ET_DAY;
		opt_quality = EH_DEFAULT;
		camera_num = 0;
		memset(cameras_index, 0, sizeof(int) * MAX_MODEL_SCENES);
		cameras_index[0] = true;
		exp_val = 0;
		exp_val_on = false;
		enviroment_hdr_multipler = 1.0f;
		hdr_name = "";
	}
};

extern skp2ess_set g_skp2ess_set;