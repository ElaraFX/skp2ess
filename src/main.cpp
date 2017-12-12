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


void skpCloudRender(skp2ess_config &sg)
{
	std::string ess_filename(sg.skp_projectname);
	ess_filename += ".ess";
	g_material_container.SetProjectName(sg.skp_projectname.c_str());
	import_mesh_from_skp(sg.skp_filename.c_str(), ess_filename.c_str());
	CloudRender(sg.exepath.c_str(), ess_filename.c_str(), sg.outputfileprefix.c_str(), sg.outputfiletype.c_str(), sg.outputpath.c_str());
}

//int main(int argc, char* argv[]) {
//	skp2ess_config sg;
//	sg.skp_filename = "gjj1.skp";
//	sg.skp_projectname = "gjj1";
//	sg.outputpath = "D:/";
//	sg.outputfileprefix = "result";
//	sg.outputfiletype = "png";
//	sg.exepath = argv[0];
//
//	skpCloudRender(sg);
//	return 0;
//}

