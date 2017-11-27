#include "ImportSketchup.h"
#include <ei_timer.h>

bool import_mesh_from_skp(const char *file_name, const char *output_filename, ParseSkpType type)
{
	printf("parse %s \n", file_name);
	EH_Context *pContext = EH_create();
	EH_ExportOptions option;
	option.base85_encoding = true;
	option.left_handed = false;
	printf("genreate file %s...\n", output_filename);
	EH_begin_export(pContext, output_filename, &option);

	EH_RenderOptions render_op;
	render_op.quality = EH_MEDIUM;
	EH_set_render_options(pContext, &render_op);

	EH_Sky sky;
	sky.enabled = true;
	sky.hdri_name = "004.hdr";
	sky.hdri_rotation = radians(-36);
	sky.intensity = 50.0f;
	EH_set_sky(pContext, &sky);

	/*EH_Camera cam;
	cam.fov = radians(45);
	cam.near_clip = 0.01f;
	cam.far_clip = 1000.0f;
	cam.image_width = 640;
	cam.image_height = 480;
	eiMatrix cam_tran = ei_matrix(1.0, 0.0, 0.0, 0.0, 0.0, 0.642788, 0.766044, 0.0, 0.0, -0.766044, 0.642788, 0.0, 0.0, -116.666321, 122.078224, 1.0);
	memcpy(cam.view_to_world, &cam_tran.m[0][0], sizeof(EH_Mat));
	EH_set_camera(pContext, &cam);*/

	/*EH_Sun sun;
	sun.dir[0] = radians(45); 
	sun.dir[1] = radians(225); 
	float color[3] = {0.94902, 0.776471, 0.619608};
	memcpy(sun.color, color, sizeof(color));
	sun.intensity = 31.4;
	sun.soft_shadow = 1.0;
	EH_set_sun(pContext, &sun);*/

	eiTimer export_ess_timer;
	ei_timer_start(&export_ess_timer);

	switch(type)
	{
	case Envisioneer:
		skp_to_ess(file_name, pContext);
		break;
	case GuoJiaJia:
		gjj_skp_to_ess(file_name, pContext);
		break;
	default:
		gjj_skp_to_ess(file_name, pContext);
		break;
	}

	//skp_to_ess(file_name, pContext);
	//gjj_skp_to_ess(file_name, pContext);
	ei_timer_stop(&export_ess_timer);
	printf("export ess time: %f sec.\n", export_ess_timer.duration/1000.0f);

	EH_end_export(pContext);
	EH_delete(pContext);

	return true;
}