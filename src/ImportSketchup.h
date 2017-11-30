#pragma once

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/vertex.h>
#include <vector>

#include "SkpMesh.h"
#include <ei.h>

/*
Í²µÆ ehlight_td
ÉäµÆ ehlight_sd
·º¹âµÆ ehlight_fgd
Ì¨µÆ ehlight_tid
ÂäµØµÆ ehlight_ldd
±ÚµÆ ehlight_bd
µÆ´ø ehlight_dd
*/


bool import_mesh_from_skp(const char *file_name, const char *output_filename);