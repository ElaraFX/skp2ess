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
#include "Material.h"
#include "ImportSketchup.h"
#include "UploadCloud.h"
#include "CloudRender.h"
#include "skp2ess.h"

int skpCloudRender(const char* exePath, const char* filename, const char* projectname, const char* outputprefix, const char* outputtype, const char* outputpath)
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
	ret = CloudRender(exePath, ess_filename.c_str(), outputprefix, outputtype, outputpath);
	if (ret > 0)
	{
		ret += 1;
	}

	return ret;
}

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

void getState(unsigned int *state, float *param)
{
	*state = g_cri.c_state;
	*param = 0;
	if (g_cri.c_state == CLOUD_STATE_TRANSFERRING)
	{
		*param = g_cri.paramTransfer;
	}
	else if (g_cri.c_state == CLOUD_STATE_DOWNLOADING)
	{
		*param = g_cri.paramTransfer;
	}
	else if (g_cri.c_state == CLOUD_STATE_RENDERING)
	{

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

void setRenderQuality(unsigned int q)
{
	if (q <= EH_DEFAULT)
	{
		g_skp2ess_set.opt_quality = EH_RenderQuality(q);
	}
}

void setScenes(int *scene_indices, int num)
{

}

int main(int argc, char* argv[])
{
	setEnviroment(1);
	setRenderQuality(0);
	skpCloudRender(argv[0], "D:\\workspace\\skp2ess\\skp2ess\\wall_test1.skp", "wall_test1", "result", "png", "D:/");
	system("pause");
	return 0;
}
