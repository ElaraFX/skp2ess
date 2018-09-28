#include <SketchUpAPI/color.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/slapi.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/texture.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/vertex.h>
#include <SketchUpAPI/unicodestring.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <process.h>
#include "Material.h"
#include "ImportSketchup.h"
#include "UploadCloud.h"
#include "CloudRender.h"
#include "skp2ess.h"

void setResolution(int x, int y)
{
	g_cri.res_x = x;
	g_cri.res_y = y;
}

void setUsernamePassword(const char *username, const char *password)
{
	g_cri.username = username;
	g_cri.password = password;
}

void getState(int scene_index, unsigned int *state, float *param)
{
	*param = 0;
	if (g_skp2ess_set.cameras_index[scene_index])
	{
		*state = g_cri.c_state[scene_index];
		if (g_cri.c_state[scene_index] == CLOUD_STATE_TRANSFERRING)
		{
			*param = g_cri.paramTransfer;
		}
		else if (g_cri.c_state[scene_index] == CLOUD_STATE_DOWNLOADING)
		{
			*param = g_cri.paramTransfer;
		}
		else if (g_cri.c_state[scene_index] == CLOUD_STATE_RENDERING)
		{

		}
	}
	else
	{
		*state = CLOUD_STATE_UNFIND;
	}
}

void setTranferMaxSpeed(int s)
{
	g_cri.transferMaxSpeed = s;
}

void setEnviroment(unsigned int t)
{
	if (t < ET_MAX)
	{
		g_skp2ess_set.exposure_type = EXP_TYPE(t);
	}
}

void setCameraType(unsigned int t)
{
	if (t < CT_MAX)
	{
		g_skp2ess_set.camera_type = CAMERA_TYPE(t);
	}
}

void setExposureValue(float e)
{
	g_skp2ess_set.exp_val = e;
	g_skp2ess_set.exp_val_on = true;
}

void disableExposureValue()
{
	g_skp2ess_set.exp_val_on = false;
}

void setRenderQuality(unsigned int q)
{
	if (q <= EH_DEFAULT)
	{
		g_skp2ess_set.opt_quality = EH_RenderQuality(q);
	}
}

void setScenes(int *scene_indices, int num)
{
	g_skp2ess_set.camera_num = num;
	memset(&g_skp2ess_set.cameras_index, false, num * sizeof(bool));
	for (int k = 0; k < g_skp2ess_set.camera_num; k++)
	{
		g_skp2ess_set.cameras_index[scene_indices[k]] = true;
	}
}

void stopRenderJob(int scene_index)
{
	stopRenderJobBySceneIndex(scene_index);
}

void deleteRenderJob(int scene_index)
{
	abandonRenderJobBySceneIndex(scene_index);
}

void restartRenderJob(int scene_index)
{
	restartRenderJobBySceneIndex(scene_index);
}

void resumeRenderJob(int scene_index)
{
	resumeRenderJobBySceneIndex(scene_index);
}

bool getJobID(int scene_index, char *job_id)
{
	if (g_skp2ess_set.cameras_index[scene_index])
	{
		int size = g_cri.job_ids[scene_index].size();
		memcpy(job_id, g_cri.job_ids[scene_index].c_str(), sizeof(char) * size);
		job_id[size] = '\0';
		return true;
	}

	return false;
}

void setClientID(char *client_id)
{
	g_cri.clientid = client_id;
}

bool getJobWorkID(int scene_index, char *jobwork_id)
{
	if (g_skp2ess_set.cameras_index[scene_index])
	{
		int size = g_cri.jobwork_ids[scene_index].size();
		memcpy(jobwork_id, g_cri.jobwork_ids[scene_index].c_str(), sizeof(char) * size);
		jobwork_id[size] = '\0';
		return true;
	}

	return false;
}

int skp2ess(const char* filename, const char* projectname, const char* outputpath)
{
	int ret = 0;
	std::string ess_filepath(outputpath);
	if (ess_filepath[ess_filepath.size() - 1] != '/' && ess_filepath[ess_filepath.size() - 1] != '\\')
	{
		ess_filepath += '/';
	}
	ess_filepath += projectname;
	ess_filepath += ".ess";
	g_material_container.SetProjectName(projectname);
	if (!import_mesh_from_skp(filename, ess_filepath.c_str()))
	{
		return 1;
	}

	return ret;
}

int skpCloudRender(const char* exePath, const char* filename, const char* projectname, const char* outputprefix, const char* outputtype, const char* outputpath, const char* projectfolder)
{
	int ret = 0;
	std::string ess_filepath(outputpath);
	if (ess_filepath[ess_filepath.size() - 1] != '/' && ess_filepath[ess_filepath.size() - 1] != '\\')
	{
		ess_filepath += '/';
	}
	ess_filepath += projectname;
	ess_filepath += ".ess";
	g_material_container.SetProjectName(projectname);
	if (!import_mesh_from_skp(filename, ess_filepath.c_str()))
	{
		return 1;
	}
	
	std::string ess_filename(projectname);
	ess_filename += ".ess";
	ret = CloudRender(exePath, ess_filename.c_str(), outputprefix, outputtype, outputpath, projectfolder);
	if (ret > 0)
	{
		ret += 1;
	}

	return ret;
}

void setHDRname(char *hdr_name)
{
	g_skp2ess_set.hdr_name = "HDR/";
	g_skp2ess_set.hdr_name += hdr_name;
}

void setServerOutputDir(char *dir)
{
	g_cri.server_output_dir = dir;
}

void apiUninitial()
{
	g_cri.api.UnInitial();
}

void apiInitial(char *exePath)
{
	g_cri.api.Initial(exePath);
}

void setHDRmultipler(float multipler)
{
	g_skp2ess_set.enviroment_hdr_multipler = multipler;
}

void new_thread(void *param)
{
	setEnviroment(0);
	//setExposureValue(3);
	setRenderQuality(2);
	setResolution(800, 600);
	int scenes[4] = {0,1,2,3};
	//char aa[5] = "xxds";
	//setClientID(aa);
	//setHDRname("白天3.hdr");
	//setScenes(scenes, 2);
	//setCameraType(1);
	skpCloudRender("D:\workspace\skp2ess\skp2ess\x64\Release\skp2ess.exe", "D:/workspace/skp2ess/skp2ess/3401.skp", "bug", "bug", "png", "D:/", "/"); 
}

int main(int argc, char* argv[])
{
	apiInitial(argv[0]);
	//skpCloudRender(argv[0], "D:/284.skp", "chugui4", "chugui4", "png", "D:/", "/"); 
	//apiUninitial();
	new_thread(NULL);
	//_beginthread(new_thread, 0, NULL);
	system("pause");
	return 0;
}
