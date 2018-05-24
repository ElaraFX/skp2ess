#pragma once
#include "CurlHttp.h"
#include "lhdt_sdk.h"
#include "data.h"
#include <iostream>
#include "util.h"

extern int CloudRender(const char* exePath, const char* filename, const char* outputprefix, const char* outputtype, const char* outputpath, const char* projectfolder);

extern void stopRenderJobBySceneIndex(int scene_index);
extern void abandonRenderJobBySceneIndex(int scene_index);
extern void resumeRenderJobBySceneIndex(int scene_index);
extern void restartRenderJobBySceneIndex(int scene_index);

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
	CLOUD_STATE_LOGIN_FAILED,
	CLOUD_STATE_STOP,
	CLOUD_STATE_RENDER_FAILED,
	CLOUD_STATE_UNFIND
};

struct cloud_render_info
{
	LHDTSDK::LHDTInterface api;
	CurlHttp ch;
	int res_x;
	int res_y;
	CLOUD_STATE c_state[MAX_MODEL_SCENES];
	float paramTransfer;
	int cur_download_scene;
	int transferMaxSpeed;
	int finished_tasks;
	std::string clientid;
	std::string username;
	std::string password;
	std::string token;
	std::string job_ids[MAX_MODEL_SCENES];
	std::string jobwork_ids[MAX_MODEL_SCENES];
	cloud_render_info()
	{
		reset();
		ch.init();
		res_x = 1024;
		res_y = 768;
	}
	void reset()
	{
		for (int i = 0; i < MAX_MODEL_SCENES; i++)
		{
			c_state[i] = CLOUD_STATE_INITIAL;
			job_ids[i] = "";
			jobwork_ids[i] = "";
		}
		
		paramTransfer = 0;
		transferMaxSpeed = -1;
		cur_download_scene = 0;
		clientid = "gjj";
		username = "30466622";
		password = "a123456";
		finished_tasks = 0;
	}
	void initial(char *exePath)
	{
		api.Initial(exePath);
	}
};

extern cloud_render_info g_cri;
