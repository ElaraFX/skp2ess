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

#define SKP_FILENAME "test.skp"


int main() {
	import_mesh_from_skp(SKP_FILENAME);
	return 0;
}