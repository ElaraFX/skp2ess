#include "SkpMesh.h"
#include <ei_matrix.h>
#include <ei_vector.h>
#include <ei_vector2.h>
#include <SketchUpAPI/model/mesh_helper.h>

bool skp_to_ess_mesh(const char *skp_file_name, EH_Context *ctx)
{
	// Always initialize the API before using it
	SUInitialize();

	// Load the model from a file
	SUModelRef model = SU_INVALID;
	SUResult res = SUModelCreateFromFile(&model, skp_file_name);
	printf("Read %s finish!\n", skp_file_name);

	// It's best to always check the return code from each SU function call.
	// Only showing this check once to keep this example short.
	if (res != SU_ERROR_NONE)
		return false;

	// Get the entity container of the model.
	SUEntitiesRef entities = SU_INVALID;
	SUModelGetEntities(model, &entities);

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
			std::vector<eiVector> convert_vertices(num_vertices);
			for (int i = 0; i < convert_vertices.size(); ++i)
			{
				convert_vertices[i].x = vertices[i].x;
				convert_vertices[i].y = vertices[i].y;
				convert_vertices[i].z = vertices[i].z;
			}

			//Get triangle indices
			size_t num_triangles = 0;
			SUMeshHelperGetNumTriangles(mesh_ref, &num_triangles);
			const size_t num_indices = 3 * num_triangles;
			size_t num_retrieved = 0;
			std::vector<size_t> indices(num_indices);
			SUMeshHelperGetVertexIndices(mesh_ref, num_indices, &indices[0], &num_retrieved);
			std::vector<uint_t> convert_indices(num_indices);
			for (int i = 0; i < convert_indices.size(); ++i)
			{
				convert_indices[i] = indices[i];
			}

			//Get UV
			std::vector<SUPoint3D> front_stq(num_vertices);
			size_t count = 0;
			SUMeshHelperGetFrontSTQCoords(mesh_ref, num_vertices, &front_stq[0], &count);
			std::vector<eiVector2> convert_uv(num_vertices);
			for (int i = 0; i < convert_uv.size(); ++i)
			{
				convert_uv[i].x = front_stq[i].x;
				convert_uv[i].y = front_stq[i].y;
			}

			//Fill with data in EH_Mesh
			EH_Mesh eh_mesh_data;
			eh_mesh_data.verts = (EH_Vec *)&convert_vertices[0];
			eh_mesh_data.face_indices = (uint_t*)&convert_indices[0];
			eh_mesh_data.uvs = (EH_Vec2 *)&convert_uv[0];
			eh_mesh_data.num_verts = num_vertices;
			eh_mesh_data.num_faces = num_triangles;
			char export_mesh_name[MAX_NAME_LENGTH];

			sprintf(export_mesh_name, "test_mesh_%d", face_count_i);
			EH_add_mesh(ctx, export_mesh_name, &eh_mesh_data);

			EH_Material mat;
			char export_mat_name[MAX_NAME_LENGTH];
			sprintf(export_mat_name, "test_mat_%d", face_count_i);
			EH_add_material(ctx, export_mat_name, &mat);
			EH_MeshInstance inst;
			inst.mesh_name = export_mesh_name;
			inst.mtl_names[0] = export_mat_name;
			eiMatrix inst_tran = ei_matrix(
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1
				);			
			memcpy(inst.mesh_to_world, inst_tran.m, sizeof(inst.mesh_to_world));
			char export_inst_name[MAX_NAME_LENGTH];
			sprintf(export_inst_name, "test_inst_%d", face_count_i);
			EH_add_mesh_instance(ctx, export_inst_name, &inst);
		}
	}	

	// Must release the model or there will be memory leaks
	SUModelRelease(&model);

	// Always terminate the API when done using it
	SUTerminate();

	return true;
}