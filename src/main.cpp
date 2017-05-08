#include <SketchUpAPI/color.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/slapi.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/texture.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/vertex.h>
#include <SketchUpAPI/unicodestring.h>
#include <vector>
#include "Material.h"

#define SKT_FILENAME "test.skp"


int main() {
	// Always initialize the API before using it
	SUInitialize();

	// Load the model from a file
	SUModelRef model = SU_INVALID;
	SUResult res = SUModelCreateFromFile(&model, SKT_FILENAME);

	// It's best to always check the return code from each SU function call.
	// Only showing this check once to keep this example short.
	if (res != SU_ERROR_NONE)
		return 1;

	// Get the entity container of the model.
	SUEntitiesRef entities = SU_INVALID;
	SUModelGetEntities(model, &entities);

	// Get all materials
	GetAllMaterials(model);

	// Get all the faces from the entities object
	size_t faceCount = 0;
	SUEntitiesGetNumFaces(entities, &faceCount);
	if (faceCount > 0) {
		std::vector<SUFaceRef> faces(faceCount);
		SUEntitiesGetFaces(entities, faceCount, &faces[0], &faceCount);

		// Get all the edges in this face
		for (size_t i = 0; i < faceCount; i++) {
			// get face's material
			SUMaterialRef material = SU_INVALID;
			if (SUFaceGetFrontMaterial(faces[i], &material) == SU_ERROR_NONE)
			{
				CSUString name;
				SU_CALL(SUMaterialGetNameLegacyBehavior(material, name));
				std::string material_name = name.utf8();
				int material_index = g_material_container.FindIndexWithString(material_name);
				if (material_index != -1)
				{
					// TODO: do something with material index
				}
			}

			size_t edgeCount = 0;
			SUFaceGetNumEdges(faces[i], &edgeCount);
			if (edgeCount > 0) {
				std::vector<SUEdgeRef> edges(edgeCount);
				SUFaceGetEdges(faces[i], edgeCount, &edges[0], &edgeCount);

				// Get the vertex positions for each edge
				for (size_t j = 0; j < edgeCount; j++) {
					SUVertexRef startVertex = SU_INVALID;
					SUVertexRef endVertex = SU_INVALID;
					SUEdgeGetStartVertex(edges[j], &startVertex);
					SUEdgeGetEndVertex(edges[j], &endVertex);
					SUPoint3D start;
					SUPoint3D end;
					SUVertexGetPosition(startVertex, &start);
					SUVertexGetPosition(endVertex, &end);
					// TODO: do something with the point data
				}
			}
		}
	}

	// Get model name
	SUStringRef name = SU_INVALID;
	SUStringCreate(&name);
	SUModelGetName(model, &name); 
	size_t name_length = 0;
	SUStringGetUTF8Length(name, &name_length);
	char* name_utf8 = new char[name_length + 1];
	SUStringGetUTF8(name, name_length + 1, name_utf8, &name_length);
	// Now we have the name in a form we can use

	// release all resource
	SUStringRelease(&name);
	delete[]name_utf8;

	// Must release the model or there will be memory leaks
	SUModelRelease(&model);

	// Always terminate the API when done using it
	SUTerminate();
	return 0;
}
