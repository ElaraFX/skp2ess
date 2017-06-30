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
#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <limits>

const std::string DEFAULT_MTL_NAME = "default_mtl";
const int default_width = 1280;
const int default_height = 720;
const float REMOVE_VERTEX_EPS = 0.00000001;
const float COMBINE_NORMAL_THRESHOLD = std::cos(radians(85));

const std::string MAT_PATH = "./materials";

struct Vertex
{
	std::vector<eiVector> vertices;
	std::vector<eiVector2> uvs;
	std::vector<uint_t> indices;
	std::vector<eiVector> normals;
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
};
typedef std::unordered_map<int, VertexCacheData> VertexCacheDataMap;

typedef std::unordered_map<std::string, Vertex*> MtlVertexMap;
MtlVertexMap g_mtl_to_vertex_map;
typedef std::unordered_map<std::string, EH_Material> MtlMap;
MtlMap g_mtl_map;
typedef std::unordered_map<int, VertexCacheDataMap> VertexCacheMap;
//VertexMap g_vertex_map;
typedef std::unordered_map<std::string, VertexCacheMap*> MtlVertexCacheMap;
MtlVertexCacheMap g_mtl_vertex_cache_map;

std::vector<std::string> mat_list;

typedef std::vector<EH_Light> LightsVector;
LightsVector light_vector;

void export_mesh_mtl_from_entities(SUEntitiesRef entities);
void convert_mesh_and_mtl(EH_Context *ctx, const std::string &mtl_name, Vertex *vertex, const std::string &poly_name);
void convert_to_eh_mtl(EH_Material &eh_mtl, SUMaterialRef skp_mtl, UVScale &uv_scale);
void convert_to_eh_camera(EH_Camera &cam, SUCameraRef su_cam_ref);
void export_light(EH_Context *ctx);
void import_mat_list();
void release_all_res();
EH_Camera create_camera_from_pos_normal(const eiVector &pos, const eiVector &normal);
EH_Sun create_sun_dir_light(const eiVector &dir);

#ifdef _MSC_VER
std::wstring to_utf16(std::string str);
#endif

void import_mat_list()
{
	std::locale global_loc = std::locale(); 
	std::locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet); 
	boost::filesystem::path::imbue(loc); 
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
}

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

	//import material list
	import_mat_list();

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
	//SUShadowInfoRef shadow_info;
	//SUResult get_sun_dir_ret;
	//SUTypedValueRef dir_val;
	//dir_val.ptr = NULL;
	//SUResult create_val_ret = SUTypedValueCreate(&dir_val);
	//static char *shadow_key = "SunDirection";
	//if(SUModelGetShadowInfo(model, &shadow_info) == SU_ERROR_NONE)
	//{
	//	size_t shadow_key_count = 0;
	//	SUShadowInfoGetNumKeys(shadow_info, &shadow_key_count);
	//	if(shadow_key_count > 0)
	//	{
	//		get_sun_dir_ret = SUShadowInfoGetValue(shadow_info, "SunDirection", &dir_val);
	//	}

	//}

	//if(get_sun_dir_ret == SU_ERROR_NONE)
	//{		
	//	double vector3d_val[3];
	//	SUTypedValueGetVector3d(dir_val, vector3d_val);
	//	printf("x = %f, y = %f, z = %f\n", vector3d_val[0], vector3d_val[1], vector3d_val[2]);

	//	EH_Sun sun;
	//	sun.dir[0] = std::acos(vector3d_val[2]);
	//	float phi = std::atan(vector3d_val[1]/vector3d_val[0]);
	//	if(vector3d_val[0] > 0.0f)
	//	{
	//		phi = phi;
	//	}
	//	else
	//	{
	//		phi = phi + EI_PI;
	//	}

	//	sun.dir[1] = phi; 
	//	//printf("theta = %f, phi = %f\n", sun.dir[0] * (180.0/EI_PI), sun.dir[1] * (180.0/EI_PI));
	//	float color[3] = {0.94902, 0.776471, 0.619608};
	//	memcpy(sun.color, color, sizeof(color));
	//	sun.intensity = 30.4;
	//	sun.soft_shadow = 1.0f;
	//	EH_set_sun(ctx, &sun);
	//}
	//else
	//{
	//	printf("Get sun direction error ! error code = %d\n", get_sun_dir_ret);
	//}
	//SUTypedValueRelease(&dir_val);

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

	release_all_res();

	return true;
}

void convert_mesh_and_mtl(EH_Context *ctx, const std::string &mtl_name, Vertex *vertex, const std::string &poly_name)
{
	//vertex->normals.reserve(vertex->vertices.size());
	//Generate normals	
	eiVector zero_val =  ei_vector(0.0f, 0.0f, 0.0f);
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
	}

	EH_Material mat = g_mtl_map[mtl_name];
	//create camera
	if (mat.diffuse_tex.filename)
	{		
		std::string tex_filename = mat.diffuse_tex.filename;
		if (tex_filename.find("camera_instance") != std::string::npos)
		{			
			eiVector normal = ei_vector(-vertex->normals[0].x, -vertex->normals[0].y, -vertex->normals[0].z);
			eiVector pos = vertex->vertices[0];
			EH_Camera cam = create_camera_from_pos_normal(pos, normal);
			EH_set_camera(ctx, &cam);

			printf("create camera !!!\n");
			return;
		}
		else if (tex_filename.find("sun_direction") != std::string::npos)
		{
			eiVector sun_dir = ei_vector(-vertex->normals[0].x, -vertex->normals[0].y, -vertex->normals[0].z);
			EH_Sun eh_sun = create_sun_dir_light(sun_dir);
			EH_set_sun(ctx, &eh_sun);

			printf("create sun dir light !\n");
		}
	}

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
	if (mat.diffuse_tex.filename)
	{
		std::string tex_filename = mat.diffuse_tex.filename;				
		for(int i = 0; i < mat_list.size(); ++i)
		{
			if (tex_filename.find(mat_list[i]) != std::string::npos)
			{
				find_map_material = true;
				printf("tex = %s\n", tex_filename.c_str());
				import_ess_filename = MAT_PATH + "/" + mat_list[i] + ".ess";
				break;
			}
		}
	}

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
		eh_mtl.transp_weight = 1.0 - mtl_info.alpha_;
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
					light.intensity = 25000.0f;
					light.type = EH_LIGHT_IES;
					light.ies_filename = "./19.ies";
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

			//Get UV
			std::vector<SUPoint3D> front_stq(num_vertices);
			size_t count = 0;
			SUMeshHelperGetFrontSTQCoords(mesh_ref, num_vertices, &front_stq[0], &count);

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
			std::vector<size_t> vert_indices;
			//vert_indices.reserve(num_triangles * 3);
			//pContainVertex->vertices.reserve(num_vertices);
			//pContainVertex->uvs.reserve(num_vertices);
			for (int i = 0; i < num_vertices; ++i)
			{
				//Whether vertice is redundancy
				eiVector curr_vertex = ei_vector(vertices[i].x, vertices[i].y, vertices[i].z);
				eiVector2 curr_uv = ei_vector2(front_stq[i].x * uv_scale.u, front_stq[i].y * uv_scale.v);
				bool is_redundancy = false;

				int key = (int)(vertices[i].x * vertices[i].y * vertices[i].z);
				if (p_vertex_cache_map->find(key) != p_vertex_cache_map->end())
				{
					VertexCacheDataMap &vs_map = (*p_vertex_cache_map)[key];
					for(VertexCacheDataMap::iterator iter = vs_map.begin();
						iter != vs_map.end(); ++iter)
					{
						const eiVector &compare_vert = iter->second.vs;
						const eiVector2 &compare_uv = iter->second.uv;
						if(std::abs(curr_vertex.x - compare_vert.x) < REMOVE_VERTEX_EPS &&
							std::abs(curr_vertex.y - compare_vert.y) < REMOVE_VERTEX_EPS &&
							std::abs(curr_vertex.z - compare_vert.z) < REMOVE_VERTEX_EPS &&
							std::abs(curr_uv.x - compare_uv.x) < REMOVE_VERTEX_EPS &&
							std::abs(curr_uv.y - compare_uv.y) < REMOVE_VERTEX_EPS)
						{							
							size_t offset = i % 3;
							size_t vertex_start_index = i - offset;
							eiVector p0 = ei_vector(vertices[vertex_start_index].x, vertices[vertex_start_index].y, vertices[vertex_start_index].z);
							eiVector p1 = ei_vector(vertices[vertex_start_index+1].x, vertices[vertex_start_index+1].y, vertices[vertex_start_index+1].z);
							eiVector p2 = ei_vector(vertices[vertex_start_index+2].x, vertices[vertex_start_index+2].y, vertices[vertex_start_index+2].z);
							eiVector a = normalize(p2 - p1);
							eiVector b = normalize(p1 - p0);
							eiVector curr_normal = normalize(cross(b, a));

							size_t index = iter->first;
							offset = index % 3;
							size_t combine_vertice_start_index = index - offset;
							p0 = ei_vector(
								pContainVertex->vertices[combine_vertice_start_index].x, 
								pContainVertex->vertices[combine_vertice_start_index].y, 
								pContainVertex->vertices[combine_vertice_start_index].z);
							p1 = ei_vector(
								pContainVertex->vertices[combine_vertice_start_index+1].x, 
								pContainVertex->vertices[combine_vertice_start_index+1].y, 
								pContainVertex->vertices[combine_vertice_start_index+1].z);
							p2 = ei_vector(
								pContainVertex->vertices[combine_vertice_start_index+2].x, 
								pContainVertex->vertices[combine_vertice_start_index+2].y, 
								pContainVertex->vertices[combine_vertice_start_index+2].z);
							a = normalize(p2 - p1);
							b = normalize(p1 - p0);
							eiVector combine_vertice_normal = normalize(cross(b, a));

							if (dot(curr_normal, combine_vertice_normal) > COMBINE_NORMAL_THRESHOLD)
							{
								is_redundancy = true;
								vert_indices.push_back(iter->first);
								break;
							}							
						}
					}
				}
				else
				{
					(*p_vertex_cache_map)[key] = VertexCacheDataMap();
				}


				if(is_redundancy == false)
				{
					size_t index = pContainVertex->vertices.size();
					VertexCacheData v_cache_data;
					v_cache_data.vs = curr_vertex;
					v_cache_data.uv = curr_uv;
					(*p_vertex_cache_map)[key].insert(std::make_pair(index, v_cache_data));

					vert_indices.push_back(index);
					pContainVertex->vertices.push_back(curr_vertex);
					pContainVertex->uvs.push_back(curr_uv);
				}				
			}

			const size_t num_indices = 3 * num_triangles;
			size_t num_retrieved = 0;
			std::vector<size_t> local_indices(num_indices);
			SUMeshHelperGetVertexIndices(mesh_ref, num_indices, &local_indices[0], &num_retrieved);
			for (int i = 0; i < num_indices; ++i)
			{
				pContainVertex->indices.push_back(vert_indices[local_indices[i]]);
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

#ifdef _MSC_VER
std::wstring to_utf16(std::string str)
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

void release_all_res()
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

EH_Camera create_camera_from_pos_normal(const eiVector &pos, const eiVector &normal)
{
	EH_Camera cam;
	eiVector zaxis = normal;
	//printf("zaxis = %f, %f, %f\n", zaxis.x, zaxis.y, zaxis.z);
	eiVector xaxis = normalize(cross(ei_vector(0.0f, 0.0f, 1.0f), zaxis));
	if (xaxis.x < EI_SCALAR_EPS && xaxis.y < EI_SCALAR_EPS && xaxis.z < EI_SCALAR_EPS) //z axis parallel to (0, 1, 0)
	{
		xaxis = ei_vector(-1.0f, 0.0f, 0.0f);
	}
	//printf("xaxis = %f, %f, %f\n", xaxis.x, xaxis.y, xaxis.z);
	eiVector yaxis = cross(zaxis, xaxis);
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

EH_Sun create_sun_dir_light(const eiVector &dir)
{
	EH_Sun sun;
	sun.dir[0] = std::acos(dir.z);
	float phi = std::atan(dir.y/dir.x);
	if(dir.x > 0.0f)
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

	return sun;
}