#include "ImportSketchup.h"
#include <ei_timer.h>

bool import_mesh_from_skp(const char *file_name, const char *output_filename)
{
	printf("parse %s \n", file_name);
	EH_Context *pContext = EH_create();
	EH_ExportOptions option;
	option.base85_encoding = true;
	option.left_handed = false;
	printf("genreate file %s...\n", output_filename);
	EH_begin_export(pContext, output_filename, &option);

	EH_RenderOptions render_op;
	render_op.quality = g_skp2ess_set.opt_quality;
	EH_set_options_name(pContext, "GlobalOptionsName");
	EH_set_render_options(pContext, &render_op);

	eiTimer export_ess_timer;
	ei_timer_start(&export_ess_timer);

	if (!skp_to_ess(file_name, pContext))
		return false;

	//skp_to_ess(file_name, pContext);
	//gjj_skp_to_ess(file_name, pContext);
	ei_timer_stop(&export_ess_timer);
	printf("export ess time: %f sec.\n", export_ess_timer.duration/1000.0f);

	EH_end_export(pContext);
	EH_delete(pContext);

	return true;
}