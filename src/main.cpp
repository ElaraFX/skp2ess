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
#include "Material.h"
#include "ImportSketchup.h"
#include "UploadCloud.h"

#define SKP_FILENAME "gjj1.skp"
#define SKP_PROJECTNAME "gjj1"



int main(int argc, char* argv[]) {
	uploadCloud(argv[0]);
	//g_material_container.SetProjectName(SKP_PROJECTNAME);
	//import_mesh_from_skp(SKP_FILENAME, "test.ess");
	return 0;
}

