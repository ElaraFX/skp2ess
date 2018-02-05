#include "SkpMesh.h"
#include "Material.h"
#include "esslib.h"

#include <stdio.h>
#include <string.h>

#include <ei_matrix.h>
#include <ei_vector.h>
#include <ei_vector2.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/mesh_helper.h>
#include <SketchUpAPI/model/scene.h>
#include <SketchUpAPI/model/group.h>
#include <SketchUpAPI/model/camera.h>
#include <SketchUpAPI/model/entity.h>
#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/component_instance.h>
#include <SketchUpAPI/model/attribute_dictionary.h>
#include <SketchUpAPI/model/dynamic_component_info.h>
#include <SketchUpAPI/model/shadow_info.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/typed_value.h>
#include <SketchUpAPI/model/drawing_element.h>

#include <unordered_map>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <limits>

static const std::string DEFAULT_MTL_NAME = "default_mtl";
static const std::string ESS_RENDER_PATH = "highmodel_path";
static const std::string TEXTURE_RENDER_PATH = "texture_path";
static const int default_width = 1280;
static const int default_height = 720;
static const float REMOVE_VERTEX_EPS = 0.00000001;
static const float COMBINE_NORMAL_THRESHOLD = std::cos(radians(70));
static const float INCH2MM = 25.4;

static std::string MAT_PATH;

skp2ess_set g_skp2ess_set;

struct Vertex
{
	std::vector<eiVector> vertices;
	std::vector<eiVector2> uvs;
	std::vector<uint_t> indices;
	std::vector<eiVector> normals;
};

struct TriangleIndex
{
	size_t i, j, k;

	TriangleIndex(size_t i, size_t j, size_t k) :
		i(i),
		j(j),
		k(k)
	{
	}
};

struct UVScale
{
	float u, v;

	UVScale():
	u(1.0),
		v(1.0)
	{

	}
};



struct VertexCacheData
{
	eiVector vs;
	eiVector2 uv;

	bool operator ==(const VertexCacheData &rht) const
	{
		if(std::abs(vs.x - rht.vs.x) < REMOVE_VERTEX_EPS &&
			std::abs(vs.y - rht.vs.y) < REMOVE_VERTEX_EPS &&
			std::abs(vs.z - rht.vs.z) < REMOVE_VERTEX_EPS &&
			std::abs(uv.x - rht.uv.x) < REMOVE_VERTEX_EPS &&
			std::abs(uv.y - rht.uv.y) < REMOVE_VERTEX_EPS)
		{
			return true;
		}

		return false;
	}
};
struct KeyDataHasher
{
	std::size_t operator () (const VertexCacheData &key) const 
	{
		// A commonly used way is to use boost
		std::size_t seed = 0;
		boost::hash_combine(seed, boost::hash_value(key.vs.x));
		boost::hash_combine(seed, boost::hash_value(key.vs.y));
		boost::hash_combine(seed, boost::hash_value(key.vs.z));
		boost::hash_combine(seed, boost::hash_value(key.uv.x));
		boost::hash_combine(seed, boost::hash_value(key.uv.y));
		return seed;
	}
};
typedef boost::unordered_map<int, VertexCacheData> VertexCacheDataMap;

typedef std::unordered_map<std::string, Vertex*> MtlVertexMap;
static MtlVertexMap g_mtl_to_vertex_map;
typedef std::unordered_map<std::string, EH_Material> MtlMap;
static MtlMap g_mtl_map;
typedef boost::unordered_map<VertexCacheData, VertexCacheDataMap, KeyDataHasher> VertexCacheMap;
//VertexMap g_vertex_map;
typedef std::unordered_map<std::string, VertexCacheMap*> MtlVertexCacheMap;
static MtlVertexCacheMap g_mtl_vertex_cache_map;

static std::vector<std::string> mat_list;

typedef std::vector<EH_Light> LightsVector;
static LightsVector light_vector;

static void export_mesh_mtl_from_entities(SUEntitiesRef entities, SUTransformation *transform, SUMaterialRef parent_mat, std::string par_tex_path);
static void convert_mesh_and_mtl(EH_Context *ctx, const std::string &mtl_name, Vertex *vertex, const std::string &poly_name);
static void convert_to_eh_mtl(EH_Material &eh_mtl, SUMaterialRef skp_mtl, UVScale &uv_scale);
static void convert_to_eh_camera(EH_Camera &cam, SUCameraRef su_cam_ref);
static void export_light(EH_Context *ctx);
static void import_mat_list();
static void release_all_res();
static EH_Camera create_camera_from_pos_normal(const eiVector &pos, const eiVector &normal);
static void fix_inf_vertex(eiVector &v);
static void set_day_exposure(EH_Context *ctx);
static void set_night_exposure(EH_Context *ctx);
static void set_outworld_day_exposure(EH_Context *ctx);
static void set_sun(EH_Context *ctx, EH_Vec2 &dir);

#ifdef _MSC_VER
static std::wstring to_utf16(std::string str);
#endif

static void import_mat_list()
{
	std::locale global_loc = std::locale(); 
	std::locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet); 
	boost::filesystem::path::imbue(loc);
	char cur_dir[ EI_MAX_FILE_NAME_LEN ];
	ei_get_current_directory(cur_dir);
	MAT_PATH = cur_dir + std::string("/materials");
	boost::filesystem::path p(MAT_PATH);
	bool is_path_valid = boost::filesystem::exists(p);
	if(is_path_valid)
	{
		for (boost::filesystem::directory_iterator i = boost::filesystem::directory_iterator(p); i != boost::filesystem::directory_iterator(); i++)
		{
			if (!is_directory(i->path())) //we eliminate directories
			{
				const std::string &filename = i->path().filename().string();
				//printf("mat name %s\n", filename.c_str());
				mat_list.push_back(filename.substr(0, filename.find('.')));
			}
			else
				continue;
		}
	}
	else
	{
		printf("Invalid materials path: %s\n", MAT_PATH.c_str());
	}
}

static void MatrixMutiply(SUTransformation &t1, SUTransformation &t2, SUTransformation &tout)
{
	for (int i = 0; i < 4; i++)	
	{
		for (int j = 0; j < 4; j++)
		{
			tout.values[i * 4 + j] = 0;	
			for (int k = 0; k < 4; k++)
				tout.values[i * 4 + j] += t1.values[i * 4 + k] * t2.values[k * 4 + j];
		}
	}
}

bool get_entity_attribute(SUEntityRef entity, const char* dict_name, const char* attr_name, char* value_out)
{
	SUResult result = SU_ERROR_NONE;
	SUTypedValueRef componentAttributeValue = SU_INVALID;
	result = SUTypedValueCreate(&componentAttributeValue);
	SUAttributeDictionaryRef dictionay = SU_INVALID;
	result = SUEntityGetAttributeDictionary(entity, dict_name, &dictionay);
	if (result != SU_ERROR_NONE)
		return false;

	SUStringRef value = SU_INVALID;
	result = SUAttributeDictionaryGetValue(dictionay, attr_name, &componentAttributeValue);
	if (result != SU_ERROR_NONE)
		return false;

	SUTypedValueType type;
	SUTypedValueGetType(componentAttributeValue, &type);
	result = SUStringCreate(&value);
	result = SUTypedValueGetString(componentAttributeValue, &value);
	if (result != SU_ERROR_NONE)
		return false;

	size_t strLen = 0;
	size_t copied = 0;
	result = SUStringGetUTF8Length(value, &strLen);
	if (result != SU_ERROR_NONE)
		return false;

	SUStringGetUTF8(value, strLen, value_out, &copied);
	return true;
}

static void writeEntities(SUEntitiesRef &entities, SUTransformation &t, SUMaterialRef w_par_mat, EH_Context *ctx)
{
	//SUMaterialRef null_mat = SU_INVALID;
	export_mesh_mtl_from_entities(entities, &t, w_par_mat, std::string(""));

	size_t num_instances = 0;
	SU_CALL(SUEntitiesGetNumInstances(entities, &num_instances));
	if (num_instances > 0) 
	{
		std::vector<SUComponentInstanceRef> instances(num_instances);
		SU_CALL(SUEntitiesGetInstances(entities, num_instances,
			&instances[0], &num_instances));
		for (size_t c = 0; c < num_instances; c++) 
		{
			SUComponentInstanceRef instance = instances[c];
			SUComponentDefinitionRef definition = SU_INVALID;
			SU_CALL(SUComponentInstanceGetDefinition(instance, &definition));

			SUTransformation transform, transform_mul;
			SU_CALL(SUComponentInstanceGetTransform(instance, &transform));
			MatrixMutiply(transform, t, transform_mul);

			// get ess_path	
			SUEntityRef en = SUComponentDefinitionToEntity(definition);
			char render_path[MAX_PATH] = "";
			if (get_entity_attribute(en, "info", ESS_RENDER_PATH.c_str(), render_path))
			{
				/**< 加载外部ESS */
				eiMatrix transform_without_t = ei_matrix( /* 引用外部ESS模型的变化矩阵 */
					transform_mul.values[0], transform_mul.values[1], transform_mul.values[2], transform_mul.values[3],
					transform_mul.values[4], transform_mul.values[5], transform_mul.values[6], transform_mul.values[7],
					transform_mul.values[8], transform_mul.values[9], transform_mul.values[10], transform_mul.values[11],
					0, 0, 0, transform_mul.values[15]
				);
				eiMatrix include_ess_mat = ei_matrix(1.0f / INCH2MM, 0, 0, 0, 0, 1.0f / INCH2MM, 0, 0, 0, 0, 1.0f / INCH2MM, 0, 0, 0, 0, 1.0f)
					* transform_without_t;
				
				include_ess_mat.m[3][0] = transform_mul.values[12];
				include_ess_mat.m[3][1] = transform_mul.values[13];
				include_ess_mat.m[3][2] = transform_mul.values[14];					

				EH_AssemblyInstance include_inst;

				include_inst.filename = render_path; /* 需要包含的ESS */
				std::string include_inst_name(render_path);
				char num[20] = "";
				sprintf(num, "_c_%d", c);
				include_inst_name += num;
				memcpy(include_inst.mesh_to_world, (include_ess_mat/* * inch2mm*/).m, sizeof(include_inst.mesh_to_world));
				EH_add_assembly_instance(ctx, include_inst_name.c_str(), &include_inst); /* include_test_ess 是ESS中节点的名字 不能重名 */
				continue;
			}
			
			// get texture_path	
			memset(render_path, 0, MAX_PATH * sizeof(char));
			get_entity_attribute(en, "info", TEXTURE_RENDER_PATH.c_str(), render_path);
			std::string par_tex(render_path);

			SUEntitiesRef c_entities;
			SUComponentDefinitionGetEntities(definition, &c_entities);

			SUMaterialRef material = SU_INVALID;
			SUDrawingElementGetMaterial(SUComponentInstanceToDrawingElement(instance), &material);
			export_mesh_mtl_from_entities(c_entities, &transform_mul, material, par_tex);
			writeEntities(c_entities, transform_mul, material, ctx);
		}
	}

	// Get Groups
	size_t num_groups = 0;
	SUEntitiesGetNumGroups(entities, &num_groups);
	//printf("Groups number: %d.\n", num_groups);
	if (num_groups > 0) 
	{
		std::vector<SUGroupRef> groups(num_groups);
		SU_CALL(SUEntitiesGetGroups(entities, num_groups, &groups[0], &num_groups));
		for (size_t group_i = 0; group_i < num_groups; group_i++) 
		{
			for(MtlVertexCacheMap::iterator iter = g_mtl_vertex_cache_map.begin(); 
				iter != g_mtl_vertex_cache_map.end(); ++iter)
			{
				delete iter->second;
				iter->second = NULL;
			}

			g_mtl_vertex_cache_map.clear();
			SUGroupRef group = groups[group_i];

			SUComponentDefinitionRef group_component = SU_INVALID;
			SUGroupGetDefinition(group, &group_component);
			SUEntitiesRef group_entities = SU_INVALID;
			SU_CALL(SUGroupGetEntities(group, &group_entities));
			
			// Write transformation
			SUTransformation transform, transform_mul;
			SU_CALL(SUGroupGetTransform(group, &transform));
			MatrixMutiply(transform, t, transform_mul);

			// get ess_path	
			SUEntityRef en = SUComponentDefinitionToEntity(group_component);
			char render_path[MAX_PATH] = "";
			if (get_entity_attribute(en, "info", ESS_RENDER_PATH.c_str(), render_path))
			{
				/**< 加载外部ESS */
				eiMatrix transform_without_t = ei_matrix( /* 引用外部ESS模型的变化矩阵 */
					transform_mul.values[0], transform_mul.values[1], transform_mul.values[2], transform_mul.values[3],
					transform_mul.values[4], transform_mul.values[5], transform_mul.values[6], transform_mul.values[7],
					transform_mul.values[8], transform_mul.values[9], transform_mul.values[10], transform_mul.values[11],
					0, 0, 0, transform_mul.values[15]
				);
				eiMatrix include_ess_mat = ei_matrix(1.0f / INCH2MM, 0, 0, 0, 0, 1.0f / INCH2MM, 0, 0, 0, 0, 1.0f / INCH2MM, 0, 0, 0, 0, 1.0f)
					* transform_without_t;
				
				include_ess_mat.m[3][0] = transform_mul.values[12];
				include_ess_mat.m[3][1] = transform_mul.values[13];
				include_ess_mat.m[3][2] = transform_mul.values[14];					

				EH_AssemblyInstance include_inst;

				include_inst.filename = render_path; /* 需要包含的ESS */
				std::string include_inst_name(render_path);
				char num[20] = "";
				sprintf(num, "_g_%d", group_i);
				include_inst_name += num;
				memcpy(include_inst.mesh_to_world, (include_ess_mat/* * inch2mm*/).m, sizeof(include_inst.mesh_to_world));
				EH_add_assembly_instance(ctx, include_inst_name.c_str(), &include_inst); /* include_test_ess 是ESS中节点的名字 不能重名 */
				continue;
			}

			// get texture_path	
			memset(render_path, 0, MAX_PATH * sizeof(char));
			get_entity_attribute(en, "info", TEXTURE_RENDER_PATH.c_str(), render_path);
			std::string par_tex(render_path);

			SUEntitiesRef c_entities;
			SUGroupGetEntities(group, &c_entities);

			SUMaterialRef material = SU_INVALID;
			SUDrawingElementGetMaterial(SUGroupToDrawingElement(group), &material);

			export_mesh_mtl_from_entities(c_entities, &transform_mul, material, par_tex);
			writeEntities(c_entities, transform_mul, material, ctx);
		}
	}
}

bool skp_to_ess(const char *skp_file_name, EH_Context *ctx)
{	
	// Always initialize the API before using it
	SUInitialize();

	// Load the model from a file
	SUModelRef model = SU_INVALID;
	SUResult res = SUModelCreateFromFile(&model, skp_file_name);	

	// It's best to always check the return code from each SU function call.
	// Only showing this check once to keep this example short.
	if (res != SU_ERROR_NONE)
	{
		SUModelRelease(&model);
		SUTerminate();
		printf("Open %s failed! Error code = %d\n", skp_file_name, res);
		return false;
	}
	printf("Read %s finish!\n", skp_file_name);	

	//Add a default material
	EH_Material default_mat;
	default_mat.diffuse_color[0] = 1.0f;
	default_mat.diffuse_color[1] = 1.0f;
	default_mat.diffuse_color[2] = 1.0f;
	default_mat.diffuse_weight = 0.7f;
	g_mtl_map.insert(std::pair<std::string, EH_Material>(DEFAULT_MTL_NAME, default_mat));

	//import material list
	import_mat_list();

	// Get the entity container of the model.
	SUEntitiesRef entities = SU_INVALID;
	SUModelGetEntities(model, &entities);

	//Get shadow info
	SUShadowInfoRef shadow_info;
	SUResult get_sun_dir_ret;
	SUTypedValueRef dir_val;
	dir_val.ptr = NULL;
	SUResult create_val_ret = SUTypedValueCreate(&dir_val);
	static char *shadow_key = "SunDirection";
	if(SUModelGetShadowInfo(model, &shadow_info) == SU_ERROR_NONE)
	{
		size_t shadow_key_count = 0;
		SUShadowInfoGetNumKeys(shadow_info, &shadow_key_count);
		if(shadow_key_count > 0)
		{
			get_sun_dir_ret = SUShadowInfoGetValue(shadow_info, "SunDirection", &dir_val);
		}

	}
	
	// get sun direction
	EH_Vec2 sun_dir;
	if(get_sun_dir_ret == SU_ERROR_NONE)
	{		
		double vector3d_val[3];
		SUTypedValueGetVector3d(dir_val, vector3d_val);
		printf("x = %f, y = %f, z = %f\n", vector3d_val[0], vector3d_val[1], vector3d_val[2]);

		sun_dir[0] = std::acos(vector3d_val[2]);
		float phi = std::atan(vector3d_val[1]/vector3d_val[0]);
		if(vector3d_val[0] > 0.0f)
		{
			phi = phi;
		}
		else
		{
			phi = phi + EI_PI;
		}
		sun_dir[1] = phi;
	}
	else
	{
		printf("Get sun direction error ! error code = %d\n", get_sun_dir_ret);
	}
	SUTypedValueRelease(&dir_val);

	//add exposure	
	//set_outworld_day_exposure(ctx);
	if (g_skp2ess_set.exposure_type == ET_DAY)
	{
		set_day_exposure(ctx);	
		set_sun(ctx, sun_dir);
	}
	else if (g_skp2ess_set.exposure_type == ET_NIGHT)
	{
		set_night_exposure(ctx);
	}
	else
	{
		set_outworld_day_exposure(ctx);	
		set_sun(ctx, sun_dir);
	}
	
	SUCameraRef su_cam_ref = SU_INVALID;
	if (g_skp2ess_set.camera_num <= 0)
	{
		SUModelGetCamera(model, &su_cam_ref);
		if(su_cam_ref.ptr)
		{
			EH_Camera eh_cam;
			convert_to_eh_camera(eh_cam, su_cam_ref);
			if (g_skp2ess_set.camera_type == CT_CUBEMAP)
			{
				eh_cam.cubemap_render = true;
			}
			else if (g_skp2ess_set.camera_type == CT_SPHERICAL)
			{
				eh_cam.spherical_render = true;
			}
			EH_set_camera(ctx, &eh_cam);
		}
		else
		{
			printf("This scene has no active camera!\n");
		}
	}
	else
	{
		// Get Scenes Camera
		size_t num_s = 0, realcount = 0;
		SUModelGetNumScenes(model, &num_s);
		SUSceneRef scenes[MAX_MODEL_SCENES] = SU_INVALID;
		res = SUModelGetScenes(model, num_s, scenes, &realcount);
		for (int index = 0; index < realcount; index++)
		{
			SUSceneGetCamera(scenes[index], &su_cam_ref);
			// --test get scene name
			/*CSUString s_name;
			SUSceneGetName(scenes[index], s_name);
			std::string scene_n = s_name.utf8();*/
			// --end test
			if(su_cam_ref.ptr)
			{
				if (!g_skp2ess_set.cameras_index[index])
				{
					continue;
				}

				EH_Camera eh_cam;
				convert_to_eh_camera(eh_cam, su_cam_ref);
				if (g_skp2ess_set.camera_type == CT_CUBEMAP)
				{
					eh_cam.cubemap_render = true;
				}
				else if (g_skp2ess_set.camera_type == CT_SPHERICAL)
				{
					eh_cam.spherical_render = true;
				}

				char inst_name[128] = "";
				sprintf(inst_name, "SceneCamera_%d", index);
				EH_add_camera(ctx, &eh_cam, inst_name);
			}
		}
	}

	// Get all materials
	GetAllMaterials(model);	

	// Get all the faces from the entities object
	//export_mesh_mtl_from_entities(entities);

	// Get external component entities
	/*size_t component_num;
	SUModelGetNumComponentDefinitions(model, &component_num);
	if(component_num > 0)
	{
		std::vector<SUComponentDefinitionRef> definitions(component_num);
		SUModelGetComponentDefinitions(model, component_num, &definitions[0], &component_num);		

		for (int ci = 0; ci < component_num; ++ci)
		{
			SUComponentDefinitionRef &com = definitions[ci];
			SUPoint3D p;
			SUComponentDefinitionGetInsertPoint(com, &p);

			SUEntitiesRef c_entities;
			SUComponentDefinitionGetEntities(com, &c_entities);

			export_mesh_mtl_from_entities(c_entities);
		}
	}*/

	SUTransformation t;
	memset(t.values, 0, sizeof(double) * 16);
	t.values[0] = t.values[5] = t.values[10] = t.values[15] = 1;

	SUMaterialRef null_mat = SU_INVALID;
	writeEntities(entities, t, null_mat, ctx);

	int poly_index = 0;
	for (MtlVertexMap::iterator iter = g_mtl_to_vertex_map.begin();
		iter != g_mtl_to_vertex_map.end(); ++iter)
	{
		char poly_name[EI_MAX_FILE_NAME_LEN];
		sprintf(poly_name, "ploy_mesh_%d", poly_index);
		convert_mesh_and_mtl(ctx, iter->first, iter->second, poly_name);

		delete iter->second;

		++poly_index;
	}
	g_mtl_to_vertex_map.clear();
	g_mtl_map.clear();

	export_light(ctx);

	// Must release the model or there will be memory leaks
	SUModelRelease(&model);

	// Always terminate the API when done using it
	SUTerminate();

	release_all_res();

	return true;
}

static void convert_mesh_and_mtl(EH_Context *ctx, const std::string &mtl_name, Vertex *vertex, const std::string &poly_name)
{
	//vertex->normals.reserve(vertex->vertices.size());
	//Generate normals	
	/*eiVector zero_val =  ei_vector(0.0f, 0.0f, 0.0f);
	vertex->normals.resize(vertex->vertices.size(), zero_val);
	for(std::vector<uint_t>::iterator it = vertex->indices.begin(); it != vertex->indices.end(); it += 3)
	{
		eiVector v[3] = {vertex->vertices[*it], vertex->vertices[*(it+1)], vertex->vertices[*(it+2)]};
		eiVector normal = cross(v[1] - v[0], v[2] - v[0]);

		for(int j = 0; j < 3; ++j)
		{
			eiVector a = normalize(v[(j+1)%3] - v[j]);
			eiVector b = normalize(v[j] - v[(j+2)%3]);
			float w_dot = clamp<float>(dot(a, b), -1.0, 1.0);
			float weight = std::acos(w_dot);
			vertex->normals[*(it+j)] += weight * normal;
		}
	}
	for(int i = 0; i < vertex->normals.size(); ++i)
	{
		vertex->normals[i] = normalize(vertex->normals[i]);
	}*/

	EH_Material mat = g_mtl_map[mtl_name];

	//Fill with data in EH_Mesh
	EH_Mesh eh_mesh_data;
	eh_mesh_data.verts = (EH_Vec *)&vertex->vertices[0];
	eh_mesh_data.face_indices = (uint_t*)&vertex->indices[0];
	eh_mesh_data.uvs = (EH_Vec2 *)&vertex->uvs[0];
	eh_mesh_data.num_verts = vertex->vertices.size();
	eh_mesh_data.num_faces = vertex->indices.size()/3;
	eh_mesh_data.normals = (EH_Vec *)&vertex->normals[0];

	std::string export_mesh_name = poly_name + "_mesh";
	EH_add_mesh(ctx, export_mesh_name.c_str(), &eh_mesh_data);
		
	bool find_map_material = false;
	std::string import_ess_filename;
	//if (mat.diffuse_tex.filename)	
	//{
		//std::string tex_filename = mat.diffuse_tex.filename;
	for(int i = 0; i < mat_list.size(); ++i)
	{
		if (mtl_name.find(mat_list[i]) != std::string::npos)
		{
			find_map_material = true;
			printf("mtl_name = %s\n", mtl_name.c_str());
			import_ess_filename = MAT_PATH + "/" + mat_list[i] + ".ess";
			break;
		}
	}
	//}

	if (find_map_material)
	{
		reinterpret_cast<EssExporter*>(ctx)->AddMaterialFromEss(mat, mtl_name.c_str(), (char*)to_utf16(import_ess_filename).c_str());
	}
	else
	{
		EH_add_material(ctx, mtl_name.c_str(), &mat);
	}	
	EH_MeshInstance inst;
	inst.mesh_name = export_mesh_name.c_str();
	inst.mtl_names[0] = mtl_name.c_str();
	eiMatrix inst_tran = ei_matrix(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
		);			
	memcpy(inst.mesh_to_world, inst_tran.m, sizeof(inst.mesh_to_world));
	std::string export_inst_name = poly_name + "_inst";
	EH_add_mesh_instance(ctx, export_inst_name.c_str(), &inst);
}

static void convert_to_eh_mtl(EH_Material &eh_mtl, SUMaterialRef skp_mtl, UVScale &uv_scale)
{
	// Get name
	CSUString name;
	SU_CALL(SUMaterialGetNameLegacyBehavior(skp_mtl, name));
	int index = g_material_container.FindIndexWithString(name.utf8());
	MaterialInfo &mtl_info = g_material_container.materialinfos[index];
	eh_mtl.diffuse_tex.filename = mtl_info.texture_path_.c_str();
	if (mtl_info.has_alpha_)
	{
		eh_mtl.transp_weight = 1.0 - mtl_info.alpha_;
	}
	if (mtl_info.has_color_)
	{
		eh_mtl.diffuse_color[0] = float(mtl_info.color_.red) / 255;
		eh_mtl.diffuse_color[1] = float(mtl_info.color_.green) / 255;
		eh_mtl.diffuse_color[2] = float(mtl_info.color_.blue) / 255;
	}
}

static void convert_to_eh_camera(EH_Camera &cam, SUCameraRef su_cam_ref)
{
	SUPoint3D postion, target;
	SUVector3D up;
	SUCameraGetOrientation(su_cam_ref, &postion, &target, &up);

	SUTransformation trans; //column major
	SUCameraGetViewTransformation(su_cam_ref, &trans);

	double w, h;	
	double aspect_ratio;
	SUResult aspect_ratio_ret;
	aspect_ratio_ret = SUCameraGetAspectRatio(su_cam_ref, &aspect_ratio);
	if(aspect_ratio_ret == SU_ERROR_NONE)
	{
		SUCameraGetImageWidth(su_cam_ref, &w);
		h = w / aspect_ratio;
	}
	else
	{		
		//set default width and height
		w = default_width;
		h = default_height;
		aspect_ratio = w / h;
	}

	double f, n;
	SUCameraGetClippingDistances(su_cam_ref, &n, &f);

	double fov;
	SUCameraGetPerspectiveFrustumFOV(su_cam_ref, &fov);

	eiMatrix ei_tran = ei_matrix(
		trans.values[0], trans.values[4], trans.values[8], trans.values[12],
		trans.values[1], trans.values[5], trans.values[9], trans.values[13],
		trans.values[2], trans.values[6], trans.values[10], trans.values[14],
		postion.x, postion.y, postion.z, trans.values[15]
	);

	bool is_height_fov;
	SUCameraGetFOVIsHeight(su_cam_ref, &is_height_fov);
	if(is_height_fov)
	{
		//vertical fov -> horizonal fov
		float w_fov = std::atan((w/2) / (h/2/std::tan(radians(fov/2)))) * 2;
		fov = w_fov;
	}
	else
	{
		fov = radians(fov);
	}

	cam.image_width = w;
	cam.image_height = h;
	cam.near_clip = n / 100; //SketchUp near clip is large
	cam.far_clip = f * 1000; //SketchUp far clip is small
	cam.fov = fov;
	memcpy(cam.view_to_world, &ei_tran.m[0], sizeof(cam.view_to_world));
}

// w_par_mat 为了处理材质的递归，par_tex_path则是专门针对过家家项目指定材质的纹理贴图位置
static void export_mesh_mtl_from_entities(SUEntitiesRef entities, SUTransformation *transform, SUMaterialRef parent_mat, std::string par_tex_path)
{
	size_t faceCount = 0;	
	const size_t MAX_NAME_LENGTH = 128;
	SUEntitiesGetNumFaces(entities, &faceCount);
	if (faceCount > 0) {
		std::vector<SUFaceRef> faces(faceCount);
		SUEntitiesGetFaces(entities, faceCount, &faces[0], &faceCount);
		size_t w = 0, h = 0;
		double s = 1, t = 1;

		// Get all the edges in this face
		for (size_t face_count_i = 0; face_count_i < faceCount; ++face_count_i) {
			SUFaceRef &face_data = faces[face_count_i];

			SUMeshHelperRef mesh_ref = SU_INVALID;
			SUMeshHelperCreate(&mesh_ref, face_data);

			//Get material index
			UVScale uv_scale;
			std::string mat_name = DEFAULT_MTL_NAME;
			SUMaterialRef material = SU_INVALID;
			if (SUFaceGetFrontMaterial(face_data, &material) == SU_ERROR_NONE)
			{
				CSUString name;
				SU_CALL(SUMaterialGetNameLegacyBehavior(material, name));
				std::string material_name = name.utf8();
				if(material_name.find("ehlight") != std::string::npos)
				{
					EH_Light light;
					light.intensity = 146000.0f;
					light.type = EH_LIGHT_IES;
					light.ies_filename = "./19.ies";
					light.light_color[0] = 0.658824f;
					light.light_color[1] = 0.796078f;
					light.light_color[2] = 0.964706f;
					std::vector<SUPoint3D> vertices(1);
					size_t actually_count;
					SUMeshHelperGetVertices(mesh_ref, 1, &vertices[0], &actually_count);
					eiMatrix ei_tran = ei_matrix(
						1.0f, 0, 0, 0,
						0, 1.0f, 0, 0,
						0, 0, 1.0f, 0,
						vertices[0].x, vertices[0].y, vertices[0].z, 1.0f
						);
					light.size[0] = 10000;
					memcpy(light.light_to_world, &ei_tran.m[0], sizeof(light.light_to_world));

					light_vector.push_back(light);

					continue;
				}
				int material_index = g_material_container.FindIndexWithString(material_name);
				if (material_index != -1)
				{
					if (par_tex_path.size() > 0)
					{
						g_material_container.materialinfos[material_index].texture_path_ = par_tex_path;
					}
					MtlMap::iterator it = g_mtl_map.find(material_name);
					if (it == g_mtl_map.end())
					{
						//Add new material
						EH_Material eh_mat;
						eh_mat.diffuse_color[0] = 1.0f;
						eh_mat.diffuse_color[1] = 1.0f;
						eh_mat.diffuse_color[2] = 1.0f;
						eh_mat.diffuse_weight = 0.7f;
						convert_to_eh_mtl(eh_mat, material, uv_scale);
						g_mtl_map.insert(std::pair<std::string, EH_Material>(material_name, eh_mat));
					}

					mat_name = material_name;
				}
			}
			else if(SUIsValid(parent_mat))
			{
				CSUString name;
				SUTextureRef mtex;
				SUMaterialGetTexture(parent_mat, &mtex);
				if (SUIsValid(mtex))
				{
					SUTextureGetDimensions(mtex, &w, &h, &s, &t);
				}
				SU_CALL(SUMaterialGetNameLegacyBehavior(parent_mat, name));
				std::string material_name = name.utf8();
				int material_index = g_material_container.FindIndexWithString(material_name);
				if (material_index != -1)
				{
					if (par_tex_path.size() > 0)
					{
						g_material_container.materialinfos[material_index].texture_path_ = par_tex_path;
					}
					MtlMap::iterator it = g_mtl_map.find(material_name);
					if (it == g_mtl_map.end())
					{
						//Add new material
						EH_Material eh_mat;
						eh_mat.diffuse_color[0] = 1.0f;
						eh_mat.diffuse_color[1] = 1.0f;
						eh_mat.diffuse_color[2] = 1.0f;
						eh_mat.diffuse_weight = 0.7f;
						convert_to_eh_mtl(eh_mat, parent_mat, uv_scale);
						g_mtl_map.insert(std::pair<std::string, EH_Material>(material_name, eh_mat));
					}

					mat_name = material_name;
				}
			}

			//Get vertices
			size_t num_vertices = 0;
			SUMeshHelperGetNumVertices(mesh_ref, &num_vertices);
			if (num_vertices == 0)
			{
				printf("number of vertices is 0!\n");
				continue;
			}
			std::vector<SUPoint3D> vertices(num_vertices);
			SUMeshHelperGetVertices(mesh_ref, num_vertices, &vertices[0], &num_vertices);

			//Get UV
			std::vector<SUPoint3D> front_stq(num_vertices);
			size_t count = 0;
			SUMeshHelperGetFrontSTQCoords(mesh_ref, num_vertices, &front_stq[0], &count);
			for (int i = 0; i < num_vertices; i++)
			{
				front_stq[i].x *= s;
				front_stq[i].y *= t;
			}

			//Get Normals
			std::vector<SUVector3D> su_normals(num_vertices);
			size_t r_normal_count = 0;
			SUMeshHelperGetNormals(mesh_ref, num_vertices, &su_normals[0], &r_normal_count);

			// Transform all vertices
			if (transform != NULL)
			{			
				for (int i = 0; i < num_vertices; i++)
				{
					double pos4[4] = {vertices[i].x, vertices[i].y, vertices[i].z, 1};
					vertices[i].x = transform->values[0] * pos4[0] + transform->values[4] * pos4[1] + transform->values[8] * pos4[2] + transform->values[12] * pos4[3];
					vertices[i].y = transform->values[1] * pos4[0] + transform->values[5] * pos4[1] + transform->values[9] * pos4[2] + transform->values[13] * pos4[3];
					vertices[i].z = transform->values[2] * pos4[0] + transform->values[6] * pos4[1] + transform->values[10] * pos4[2] + transform->values[14] * pos4[3];					

					eiMatrix ei_mat = ei_matrix(
						transform->values[0], transform->values[4], transform->values[8], transform->values[12],
						transform->values[1], transform->values[5], transform->values[9], transform->values[13],
						transform->values[2], transform->values[6], transform->values[10], transform->values[14],
						transform->values[3], transform->values[7], transform->values[11], transform->values[15]
					);
					eiMatrix normal_mat = inverse(transpose(ei_mat));

					double nor4[3] = {su_normals[i].x, su_normals[i].y, su_normals[i].z};
					su_normals[i].x = normal_mat.m[0][0] * nor4[0] + normal_mat.m[0][1] * nor4[1] + normal_mat.m[0][2] * nor4[2];
					su_normals[i].y = normal_mat.m[1][0] * nor4[0] + normal_mat.m[1][1] * nor4[1] + normal_mat.m[1][2] * nor4[2];
					su_normals[i].z = normal_mat.m[2][0] * nor4[0] + normal_mat.m[2][1] * nor4[1] + normal_mat.m[2][2] * nor4[2];

					if(determinant(normal_mat) < 0)
					{
						su_normals[i].x = -su_normals[i].x;
						su_normals[i].y = -su_normals[i].y;
						su_normals[i].z = -su_normals[i].z;
					}
				}
			}

			//std::vector<eiVector> convert_vertices(num_vertices);
			Vertex *pContainVertex = NULL;
			MtlVertexMap::iterator vit = g_mtl_to_vertex_map.find(mat_name);
			if (vit == g_mtl_to_vertex_map.end())
			{
				Vertex *p_new_vertex = new Vertex();
				g_mtl_to_vertex_map.insert(std::pair<std::string, Vertex*>(mat_name, p_new_vertex));
				pContainVertex = p_new_vertex;
			}
			else
			{
				pContainVertex = vit->second;
			}
			VertexCacheMap *p_vertex_cache_map = NULL;
			MtlVertexCacheMap::iterator vcit = g_mtl_vertex_cache_map.find(mat_name);
			if (vcit == g_mtl_vertex_cache_map.end())
			{
				p_vertex_cache_map = new VertexCacheMap();
				g_mtl_vertex_cache_map.insert(std::pair<std::string, VertexCacheMap*>(mat_name, p_vertex_cache_map));
			}
			else
			{
				p_vertex_cache_map = vcit->second;
			}

			//Get triangle indices
			size_t num_triangles = 0;
			SUMeshHelperGetNumTriangles(mesh_ref, &num_triangles);
			const size_t num_indices = 3 * num_triangles;
			size_t num_retrieved = 0;
			std::vector<size_t> local_indices(num_indices);
			SUMeshHelperGetVertexIndices(mesh_ref, num_indices, &local_indices[0], &num_retrieved);

			std::vector<size_t> vert_indices;
			for (int i = 0; i < num_vertices; ++i)
			{
				eiVector curr_vertex = ei_vector(vertices[i].x, vertices[i].y, vertices[i].z);
				//fix_inf_vertex(curr_vertex);
				eiVector2 curr_uv = ei_vector2(front_stq[i].x * uv_scale.u, front_stq[i].y * uv_scale.v);
				eiVector curr_normal = normalize(ei_vector(su_normals[i].x, su_normals[i].y, su_normals[i].z));
				size_t index = pContainVertex->vertices.size();
				/*VertexCacheData v_cache_data;
				v_cache_data.vs = curr_vertex;
				v_cache_data.uv = curr_uv;*/

				//VertexCacheDataMap::value_type item(index, v_data);
				//(*p_vertex_cache_map)[v_data].insert(item);

				vert_indices.push_back(index);
				pContainVertex->vertices.push_back(curr_vertex);
				pContainVertex->uvs.push_back(curr_uv);
				pContainVertex->normals.push_back(curr_normal);
			}

			/*const size_t num_indices = 3 * num_triangles;
			size_t num_retrieved = 0;
			std::vector<size_t> local_indices(num_indices);
			SUMeshHelperGetVertexIndices(mesh_ref, num_indices, &local_indices[0], &num_retrieved);*/
			for (int i = 0; i < num_indices; ++i)
			{
				pContainVertex->indices.push_back(vert_indices[local_indices[i]]);
			}
		}
	}
}

static void export_light(EH_Context *ctx)
{
	char light_inst_name[EI_MAX_FILE_NAME_LEN];
	for(int i = 0; i < light_vector.size(); ++i)
	{
		sprintf(light_inst_name, "IES_light_inst%d", i);
		EH_add_light(ctx, light_inst_name, &light_vector[i]);
	}

	light_vector.clear();
}

#ifdef _MSC_VER
static std::wstring to_utf16(std::string str)
{
	std::wstring ret;
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
	if (len > 0)
	{
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len);
	}
	return ret;
}
#endif

static void release_all_res()
{
	for(MtlVertexMap::iterator iter = g_mtl_to_vertex_map.begin(); 
		iter != g_mtl_to_vertex_map.end(); ++iter)
	{
		delete iter->second;
		iter->second = NULL;
	}
	for(MtlVertexCacheMap::iterator iter = g_mtl_vertex_cache_map.begin(); 
		iter != g_mtl_vertex_cache_map.end(); ++iter)
	{
		delete iter->second;
		iter->second = NULL;
	}

	g_mtl_to_vertex_map.clear();
	g_mtl_map.clear();
	g_mtl_vertex_cache_map.clear();
	mat_list.clear();
	light_vector.clear();
}

static EH_Camera create_camera_from_pos_normal(const eiVector &pos, const eiVector &normal)
{
	EH_Camera cam;
	eiVector zaxis = normal;
	//printf("zaxis = %f, %f, %f\n", zaxis.x, zaxis.y, zaxis.z);
	eiVector xaxis = normalize(cross(ei_vector(0.0f, 0.0f, 1.0f), zaxis));
	if (std::abs(xaxis.x) < EI_SCALAR_EPS && std::abs(xaxis.y) < EI_SCALAR_EPS && std::abs(xaxis.z) < EI_SCALAR_EPS) //z axis parallel to (0, 1, 0)
	{
		xaxis = ei_vector(-1.0f, 0.0f, 0.0f);
	}
	//printf("xaxis = %f, %f, %f\n", xaxis.x, xaxis.y, xaxis.z);
	eiVector yaxis = normalize(cross(zaxis, xaxis));
	//printf("yaxis = %f, %f, %f\n", yaxis.x, yaxis.y, yaxis.z);
	//printf("pos = %f, %f, %f\n", pos.x, pos.y, pos.z);
	eiMatrix ei_tran = ei_matrix(
		xaxis.x, xaxis.y, xaxis.z, 0,
		yaxis.x, yaxis.y, yaxis.z, 0,
		zaxis.x, zaxis.y, zaxis.z, 0,
		pos.x, pos.y, pos.z, 1
		);			

	cam.image_width = default_width;
	cam.image_height = default_height;
	cam.near_clip = 0.0f;
	cam.far_clip = 10000;
	cam.fov = radians(75);
	memcpy(cam.view_to_world, &ei_tran.m[0], sizeof(cam.view_to_world));

	return cam;
}

static void fix_inf_vertex(eiVector &v)
{
	if(boost::math::isinf(v.x))
	{
		v.x = 0;
	}
	if(boost::math::isinf(v.y))
	{
		v.y = 0;
	}
	if(boost::math::isinf(v.z))
	{
		v.z = 0;
	}
}

static void set_day_exposure(EH_Context *ctx)
{
	EH_Exposure day_expo;
	day_expo.exposure_value = 0.5f;
	day_expo.exposure_whitepoint = 6500.0f;
	EH_set_exposure(ctx, &day_expo);

	EH_Sky sky;
	sky.enabled = true;
	sky.hdri_name = "day.hdr";
	sky.hdri_rotation = radians(0);
	sky.intensity = 100.0f;
	sky.enable_emit_GI = true;
	EH_set_sky(ctx, &sky);
}

static void set_night_exposure(EH_Context *ctx)
{
	EH_Exposure night_expo;
	night_expo.exposure_value = -0.5f;
	night_expo.exposure_highlight = 0.1f;
	night_expo.exposure_shadow = 0.4f;
	night_expo.exposure_saturation = 1.3f;
	night_expo.exposure_whitepoint = 6000.0f;
	//night_expo.display_gamma = 2.596f;
	EH_set_exposure(ctx, &night_expo);

	EH_Gamma gamma;
	gamma.display_gamma = 2.596f;
	EH_set_gamma(ctx, &gamma);

	EH_Sky sky;
	sky.enabled = true;
	sky.hdri_name = "night.hdr";
	sky.hdri_rotation = radians(0);
	sky.intensity = 1.0f;
	sky.enable_emit_GI = true;
	EH_set_sky(ctx, &sky);
}

static void set_outworld_day_exposure(EH_Context *ctx)
{
	EH_Exposure day_expo;
	day_expo.exposure_value = 3.0f;
	day_expo.exposure_whitepoint = 6500.0f;
	day_expo.exposure_shadow = 0.4f;
	EH_set_exposure(ctx, &day_expo);

	EH_Gamma gamma;
	gamma.display_gamma = 1.8f;
	EH_set_gamma(ctx, &gamma);

	EH_Sky sky;
	sky.enabled = true;
	sky.hdri_name = "day.hdr";
	sky.hdri_rotation = radians(0);
	sky.intensity = 40.0f;
	sky.enable_emit_GI = true;
	EH_set_sky(ctx, &sky);
}

static void set_sun(EH_Context *ctx, EH_Vec2 &dir)
{
	EH_Sun sun;
	sun.dir[0] = dir[0];
	sun.dir[1] = dir[1];
	float color[3] = {0.94902, 0.776471, 0.619608};
	memcpy(sun.color, color, sizeof(color));
	sun.intensity = 94.24778;
	sun.soft_shadow = 1.0f;
	EH_set_sun(ctx, &sun);
}