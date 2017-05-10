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
#include "ei_matrix.h"

bool import_mesh_from_skp(const char *file_name)
{
	EH_Context *pContext = EH_create();
	EH_ExportOptions option;
	option.base85_encoding = false;
	option.left_handed = false;
	EH_begin_export(pContext, "skp.ess", &option);

	EH_RenderOptions render_op;
	render_op.quality = EH_MEDIUM;
	EH_set_render_options(pContext, &render_op);

	EH_Sky sky;
	sky.enabled = true;
	sky.hdri_name = "013.hdr";
	sky.hdri_rotation = false;
	sky.intensity = 2.0f;
	EH_set_sky(pContext, &sky);

	EH_Camera cam;
	cam.fov = radians(45);
	cam.near_clip = 0.01f;
	cam.far_clip = 1000.0f;
	cam.image_width = 640;
	cam.image_height = 480;
	eiMatrix cam_tran = ei_matrix(0.731353, -0.681999, -0.0, 0.0, 0.255481, 0.27397, 0.927184, 0.0, -0.632338, -0.678099, 0.374607, 0.0, -38.681263, -49.142731, 21.895681, 1.0);
	memcpy(cam.view_to_world, &cam_tran.m[0][0], sizeof(EH_Mat));
	EH_set_camera(pContext, &cam);

	EH_Sun sun;
	sun.dir[0] = radians(75); 
	sun.dir[1] = radians(46); 
	float color[3] = {1, 1, 1};
	memcpy(sun.color, color, sizeof(color));
	sun.intensity = 3.14;
	sun.soft_shadow = 50.0f;
	EH_set_sun(pContext, &sun);

	skp_to_ess(file_name, pContext);

	EH_end_export(pContext);
	EH_delete(pContext);

	return true;
}