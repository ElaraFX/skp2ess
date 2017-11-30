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
Ͳ�� ehlight_td
��� ehlight_sd
����� ehlight_fgd
̨�� ehlight_tid
��ص� ehlight_ldd
�ڵ� ehlight_bd
�ƴ� ehlight_dd
*/


bool import_mesh_from_skp(const char *file_name, const char *output_filename);