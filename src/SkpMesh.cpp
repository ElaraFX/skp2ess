#include "SkpMesh.h"
#include "Material.h"
#include "esslib.h"
#include <ei_matrix.h>
#include <ei_vector.h>
#include <ei_vector2.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/mesh_helper.h>
#include <SketchUpAPI/model/scene.h>
#include <SketchUpAPI/model/camera.h>
#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/shadow_info.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/typed_value.h>

#include <unordered_map>

const std::string DEFAULT_MTL_NAME = "default_mtl";
const int default_width = 1280;
const int default_height = 720;

struct Vertex
{
	std::vector<eiVector> vertices;
	std::vector<eiVector2> uvs;
	std::vector<uint_t> indices;
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

typedef std::unordered_map<std::string, Vertex*> MtlVertexMap;
MtlVertexMap g_mtl_to_vertex_map;
typedef std::unordered_map<std::string, EH_Material> MtlMap;
MtlMap g_mtl_map;

typedef std::vector<EH_Light> LightsVector;
LightsVector light_vector;

void export_mesh_mtl_from_entities(SUEntitiesRef entities);
void convert_mesh_and_mtl(EH_Context *ctx, const std::string &mtl_name, Vertex *vertex, const std::string &poly_name);
void convert_to_eh_mtl(EH_Material &eh_mtl, SUMaterialRef skp_mtl, UVScale &uv_scale);
void convert_to_eh_camera(EH_Camera &cam, SUCameraRef su_cam_ref);
void export_light(EH_Context *ctx);

bool skp_to_ess(const char *skp_file_name, EH_Context *ctx)
{	
	// Always initialize the API before using it
	SUInitialize();

	// Load the model from a file
	SUModelRef model = SU_INVALID;
	SUResult res = SUModelCreateFromFile(&model, skp_file_name);
	printf("Read %s finish!\n", skp_file_name);

	//Add a default material
	EH_Material default_mat;
	default_mat.diffuse_color[0] = 1.0f;
	default_mat.diffuse_color[1] = 1.0f;
	default_mat.diffuse_color[2] = 1.0f;
	default_mat.diffuse_weight = 0.7f;
	g_mtl_map.insert(std::pair<std::string, EH_Material>(DEFAULT_MTL_NAME, default_mat));

	// It's best to always check the return code from each SU function call.
	// Only showing this check once to keep this example short.
	if (res != SU_ERROR_NONE)
		return false;

	// Get the entity container of the model.
	SUEntitiesRef entities = SU_INVALID;
	SUModelGetEntities(model, &entities);

	// Get Camera	
	SUCameraRef su_cam_ref = SU_INVALID;
	SUModelGetCamera(model, &su_cam_ref);
	if(su_cam_ref.ptr)
	{
		EH_Camera eh_cam;
		convert_to_eh_camera(eh_cam, su_cam_ref);

		EH_set_camera(ctx, &eh_cam);
	}
	else
	{
		printf("This scene has no active camera!\n");
	}

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
	
	if(get_sun_dir_ret == SU_ERROR_NONE)
	{		
		double vector3d_val[3];
		SUTypedValueGetVector3d(dir_val, vector3d_val);
		printf("x = %f, y = %f, z = %f\n", vector3d_val[0], vector3d_val[1], vector3d_val[2]);
		vector3d_val[0] = -vector3d_val[0];
		vector3d_val[1] = -vector3d_val[1];

		EH_Sun sun;
		sun.dir[0] = std::acos(vector3d_val[2]);
		float phi = std::atan(vector3d_val[1]/vector3d_val[0]);
		if(vector3d_val[0] > 0.0f)
		{
			phi = phi;
		}
		else
		{
			phi = phi + EI_PI;
		}

		sun.dir[1] = phi; 
		//printf("theta = %f, phi = %f\n", sun.dir[0] * (180.0/EI_PI), sun.dir[1] * (180.0/EI_PI));
		float color[3] = {0.94902, 0.776471, 0.619608};
		memcpy(sun.color, color, sizeof(color));
		sun.intensity = 30.4;
		sun.soft_shadow = 1.0f;
		EH_set_sun(ctx, &sun);
	}
	else
	{
		printf("Get sun direction error ! error code = %d\n", get_sun_dir_ret);
	}
	SUTypedValueRelease(&dir_val);

	// Get all materials
	GetAllMaterials(model);

	// Get all the faces from the entities object
	export_mesh_mtl_from_entities(entities);

	// Get external component entities
	size_t component_num;
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
	}

	int poly_index = 0;
	for (MtlVertexMap::iterator iter = g_mtl_to_vertex_map.begin();
		iter != g_mtl_to_vertex_map.end(); ++iter)
	{
		char poly_name[128];
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

	return true;
}

void convert_mesh_and_mtl(EH_Context *ctx, const std::string &mtl_name, Vertex *vertex, const std::string &poly_name)
{
	//Fill with data in EH_Mesh
	EH_Mesh eh_mesh_data;
	eh_mesh_data.verts = (EH_Vec *)&vertex->vertices[0];
	eh_mesh_data.face_indices = (uint_t*)&vertex->indices[0];
	eh_mesh_data.uvs = (EH_Vec2 *)&vertex->uvs[0];
	eh_mesh_data.num_verts = vertex->vertices.size();
	eh_mesh_data.num_faces = vertex->indices.size()/3;

	std::string export_mesh_name = poly_name + "_mesh";
	EH_add_mesh(ctx, export_mesh_name.c_str(), &eh_mesh_data);

	EH_Material mat = g_mtl_map[mtl_name];
	EH_add_material(ctx, mtl_name.c_str(), &mat);
	//reinterpret_cast<EssExporter*>(ctx)->AddMaterialFromEss(mat, mtl_name.c_str(), "test_mtl.ess");
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

void convert_to_eh_mtl(EH_Material &eh_mtl, SUMaterialRef skp_mtl, UVScale &uv_scale)
{
	// Get name
	CSUString name;
	SU_CALL(SUMaterialGetNameLegacyBehavior(skp_mtl, name));
	int index = g_material_container.FindIndexWithString(name.utf8());
	MaterialInfo &mtl_info = g_material_container.materialinfos[index];

	eh_mtl.diffuse_tex.filename = mtl_info.texture_path_.c_str();
	if (mtl_info.has_alpha_)
	{
		eh_mtl.transp_weight = mtl_info.alpha_;
	}

	std::string tex_file_name = mtl_info.texture_path_.c_str();
	if(tex_file_name.find("cloth_std_01") != std::string::npos)
	{
		eh_mtl.diffuse_weight = 1.0f;
	}
	else if(tex_file_name.find("glass_std_01") != std::string::npos)
	{
	}
	else if(tex_file_name.find("marble_std_01") != std::string::npos)
	{
		eh_mtl.diffuse_weight = 0.7f;
		eh_mtl.glossiness = 90.0f;
		eh_mtl.specular_weight = 0.2f;
	}
	else if(tex_file_name.find("light_std_01") != std::string::npos)
	{
	}
	else if(tex_file_name.find("wood_std") != std::string::npos)
	{
		eh_mtl.diffuse_weight = 0.7;
		eh_mtl.specular_weight = 0.2;
		eh_mtl.glossiness = 90.0;
	}
	else if(tex_file_name.find("cloth_std_01") != std::string::npos)
	{
	}
}

void convert_to_eh_camera(EH_Camera &cam, SUCameraRef su_cam_ref)
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
	cam.near_clip = n;
	cam.far_clip = f * 100; //SketchUp far clip is small
	cam.fov = fov;
	memcpy(cam.view_to_world, &ei_tran.m[0], sizeof(cam.view_to_world));
}

void export_mesh_mtl_from_entities(SUEntitiesRef entities)
{
	size_t faceCount = 0;	
	const size_t MAX_NAME_LENGTH = 128;
	SUEntitiesGetNumFaces(entities, &faceCount);
	if (faceCount > 0) {
		std::vector<SUFaceRef> faces(faceCount);
		SUEntitiesGetFaces(entities, faceCount, &faces[0], &faceCount);

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
					light.intensity = 100000.0f;
					light.type = EH_LIGHT_IES;
					light.ies_filename = "./19.ies";
					//memcpy(light.light_to_world, 
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
					MtlMap::iterator it = g_mtl_map.find(material_name);
					if (it == g_mtl_map.end())
					{
						//Add new material
						EH_Material eh_mat;
						convert_to_eh_mtl(eh_mat, material, uv_scale);
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
			}
			std::vector<SUPoint3D> vertices(num_vertices);
			SUMeshHelperGetVertices(mesh_ref, num_vertices, &vertices[0], &num_vertices);
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
			for (int i = 0; i < num_vertices; ++i)
			{
				pContainVertex->vertices.push_back(ei_vector(vertices[i].x, vertices[i].y, vertices[i].z));
			}

			//Get triangle indices
			size_t num_triangles = 0;
			SUMeshHelperGetNumTriangles(mesh_ref, &num_triangles);
			const size_t num_indices = 3 * num_triangles;
			size_t num_retrieved = 0;
			std::vector<size_t> indices(num_indices);
			SUMeshHelperGetVertexIndices(mesh_ref, num_indices, &indices[0], &num_retrieved);
			for (int i = 0; i < num_indices; ++i)
			{
				pContainVertex->indices.push_back(pContainVertex->vertices.size() - num_vertices + indices[i]);
			}

			//Get UV
			std::vector<SUPoint3D> front_stq(num_vertices);
			size_t count = 0;
			SUMeshHelperGetFrontSTQCoords(mesh_ref, num_vertices, &front_stq[0], &count);
			for (int i = 0; i < num_vertices; ++i)
			{
				pContainVertex->uvs.push_back(ei_vector2(front_stq[i].x * uv_scale.u, front_stq[i].y * uv_scale.v));
			}			
		}
	}
}

void export_light(EH_Context *ctx)
{
	char light_inst_name[64];
	for(int i = 0; i < light_vector.size(); ++i)
	{
		sprintf(light_inst_name, "IES_light_inst%d", i);
		EH_add_light(ctx, light_inst_name, &light_vector[i]);
	}

	light_vector.clear();
}
