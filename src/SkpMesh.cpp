#include "SkpMesh.h"
#include "Material.h"
#include <ei_matrix.h>
#include <ei_vector.h>
#include <ei_vector2.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/mesh_helper.h>
#include <SketchUpAPI/model/scene.h>
#include <SketchUpAPI/model/camera.h>

#include <unordered_map>

const std::string DEFAULT_MTL_NAME = "default_mtl";
const int default_width = 600;
const int default_height = 400;

struct Vertex
{
	std::vector<eiVector> vertices;
	std::vector<eiVector2> uvs;
	std::vector<uint_t> indices;
};

typedef std::unordered_map<std::string, Vertex*> MtlVertexMap;
MtlVertexMap g_mtl_to_vertex_map;
typedef std::unordered_map<std::string, EH_Material> MtlMap;
MtlMap g_mtl_map;

void export_mesh_and_mtl(EH_Context *ctx, const std::string &mtl_name, Vertex *vertex, const std::string &poly_name);
void convert_to_eh_mtl(EH_Material &eh_mtl, SUMaterialRef skp_mtl);
void convert_to_eh_camera(EH_Camera &cam, SUCameraRef su_cam_ref);

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

	// Get all materials
	GetAllMaterials(model);

	// Get all the faces from the entities object
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
			std::string mat_name = DEFAULT_MTL_NAME;
			SUMaterialRef material = SU_INVALID;
			if (SUFaceGetFrontMaterial(face_data, &material) == SU_ERROR_NONE)
			{
				CSUString name;
				SU_CALL(SUMaterialGetNameLegacyBehavior(material, name));
				std::string material_name = name.utf8();
				int material_index = g_material_container.FindIndexWithString(material_name);
				if (material_index != -1)
				{
					// TODO: do something with material index
					MtlMap::iterator it = g_mtl_map.find(material_name);
					if (it == g_mtl_map.end())
					{
						//Add new material
						EH_Material eh_mat;
						convert_to_eh_mtl(eh_mat, material);
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
				return false;
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
				pContainVertex->uvs.push_back(ei_vector2(front_stq[i].x, front_stq[i].y));
			}			
		}
	}

	int poly_index = 0;
	for (MtlVertexMap::iterator iter = g_mtl_to_vertex_map.begin();
		iter != g_mtl_to_vertex_map.end(); ++iter)
	{
		char poly_name[128];
		sprintf(poly_name, "ploy_mesh_%d", poly_index);
		export_mesh_and_mtl(ctx, iter->first, iter->second, poly_name);

		delete iter->second;

		++poly_index;
	}
	g_mtl_to_vertex_map.clear();
	g_mtl_map.clear();

	// Must release the model or there will be memory leaks
	SUModelRelease(&model);

	// Always terminate the API when done using it
	SUTerminate();

	return true;
}

void export_mesh_and_mtl(EH_Context *ctx, const std::string &mtl_name, Vertex *vertex, const std::string &poly_name)
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

void convert_to_eh_mtl(EH_Material &eh_mtl, SUMaterialRef skp_mtl)
{

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