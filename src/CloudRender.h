#pragma once
#include "CurlHttp.h"
#include "lhdt_sdk.h"
#include <iostream>

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
	CLOUD_STATE_LOGIN_FAILED
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
	std::string username;
	std::string password;
	cloud_render_info()
	{
		res_x = 1024;
		res_y = 768;
		c_state = CLOUD_STATE_INITIAL;
		ch.init();
		paramTransfer = 0;
		transferMaxSpeed = -1;
		username = "30466622";
		password = "a123456";
	}
};

extern cloud_render_info g_cri;
