#pragma once
#include "CurlHttp.h"
#include "lhdt_sdk.h"

extern int CloudRender(const char* exePath, const char* filename, const char* outputprefix, const char* outputtype, const char* outputpath);

enum CLOUD_STATE
{
	CLOUD_STATE_INITIAL = 0,
	CLOUD_STATE_TRANSFERRING,
	CLOUD_STATE_WAIT_RENDER,
	CLOUD_STATE_RENDERING,
	CLOUD_STATE_WAITING_OUTPUT,
	CLOUD_STATE_DOWNLOADING,
	CLOUD_STATE_RETURN,
	CLOUD_STATE_TRANSFER_FAILED,
};

struct cloud_render_info
{
	LHDTSDK::LHDTInterface api;
	CurlHttp ch;
	int res_x;
	int res_y;
	CLOUD_STATE c_state;
	float paramTransfer;
	int transferMaxSpeed;
	cloud_render_info()
	{
		res_x = 1024;
		res_y = 768;
		c_state = CLOUD_STATE_INITIAL;
		ch.init();
		paramTransfer = 0;
		transferMaxSpeed = -1;
	}
};

extern cloud_render_info g_cri;
