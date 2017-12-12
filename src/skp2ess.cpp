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
#include "Material.h"
#include "ImportSketchup.h"
#include "UploadCloud.h"
#include "CloudRender.h"
#include "skp2ess.h"


int skpCloudRender(const char* exePath, const char* filename, const char* projectname, const char* outputprefix, const char* outputtype, const char* outputpath)
{
	std::string ess_filename(projectname);
	ess_filename += ".ess";
	g_material_container.SetProjectName(projectname);
	if (!import_mesh_from_skp(filename, ess_filename.c_str()))
	{
		return 1;
	}

	int ret = CloudRender(exePath, ess_filename.c_str(), outputprefix, outputtype, outputpath);
	if (ret > 0)
	{
		ret += 1;
	}

	return ret;
}

void setResolution(int x, int y)
{
	g_res_x = x;
	g_res_y = y;
}

//int main(int argc, char* argv[])
//{
//	skpCloudRender(argv[0], "gjj1.skp", "gjj1", "result", "png", "D:/");
//	return 0;
//}
