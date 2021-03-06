#include "Material.h"
#include <ei_platform.h>

MaterialContainer g_material_container;

void GetMaterialInfo(SUMaterialRef material)
{
	if (SUIsInvalid(material))
		return;

	MaterialInfo info;

	// Name
	CSUString name;
	SU_CALL(SUMaterialGetNameLegacyBehavior(material, name));
	info.name_ = name.utf8();

	// Color
	info.has_color_ = false;
	info.has_alpha_ = false;
	SUMaterialType type;
	SU_CALL(SUMaterialGetType(material, &type));
	// Color
	if ((type == SUMaterialType_Colored) ||
		(type == SUMaterialType_ColorizedTexture)) {
		SUColor color;
		if (SUMaterialGetColor(material, &color) == SU_ERROR_NONE) {
			info.has_color_ = true;
			info.color_ = color;
		}
	}

	// Alpha
	bool has_alpha = false;
	SU_CALL(SUMaterialGetUseOpacity(material, &has_alpha));
	if (has_alpha) {
		double alpha = 0;
		SU_CALL(SUMaterialGetOpacity(material, &alpha));
		info.has_alpha_ = true;
		info.alpha_ = alpha;
	}

	// Texture
	info.has_texture_ = false;
	if ((type == SUMaterialType_Textured) ||
		(type == SUMaterialType_ColorizedTexture)) {
		SUTextureRef texture = SU_INVALID;
		if (SUMaterialGetTexture(material, &texture) == SU_ERROR_NONE) {
			info.has_texture_ = true;
			// Texture path
			CSUString texture_name;
			SU_CALL(SUTextureGetFileName(texture, texture_name));
			info.texture_name_ = texture_name.utf8();
			char texPath[EI_MAX_FILE_NAME_LEN];
			const char *projectDir = g_material_container.GetProjectName();
			if (ACCESS(projectDir, 0))
			{
				if (MKDIR(projectDir))
				{
					printf("Create directory: \'%s\' failed!\n", projectDir);
				}
			}
			sprintf(texPath, "%s\\%s", projectDir, info.texture_name_.c_str());
			SUResult ret = SUTextureWriteToFile(texture, texPath);
			if (ret != SU_ERROR_NONE)
			{
				printf("Create texture file: \'%s\' failed!\n", texPath);
			}

			// Set texture full path
			info.texture_path_ = std::string(texPath);

			// Texture scale
			size_t width = 0;
			size_t height = 0;
			double s_scale = 0.0;
			double t_scale = 0.0;
			SU_CALL(SUTextureGetDimensions(texture, &width, &height,
				&s_scale, &t_scale));
			info.texture_sscale_ = s_scale;
			info.texture_tscale_ = t_scale;
		}
	}

	g_material_container.AddMaterial(info);
}

void GetAllMaterials(SUModelRef model)
{
	size_t count = 0;
	SU_CALL(SUModelGetNumMaterials(model, &count));
	if (count > 0) {
		std::vector<SUMaterialRef> materials(count);
		SU_CALL(SUModelGetMaterials(model, count, &materials[0], &count));
		for (size_t i = 0; i<count; i++) {
			// add index into each material name to avoid duplication
			CSUString name;
			char mat_name[EI_MAX_FILE_NAME_LEN];
			SU_CALL(SUMaterialGetNameLegacyBehavior(materials[i], name));
			sprintf(mat_name, "%s_%d", name.utf8().c_str(), i);
			SUMaterialSetName(materials[i], mat_name);
			GetMaterialInfo(materials[i]);
		}
	}

	printf("Get all materials from skp data, number:%d\n", count);
}
