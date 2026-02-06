/*
	The Mesh contains the info about how to render a mesh and also how to parse it from a file.
*/

#pragma once

#include <vector>
#include "framework.h"
#include "camera.h"
#include "main/includes.h"

class Mesh
{
	// LAB mapping:
	// - 2.2: Les dades de la malla (vèrtexs) s'utilitzen per projectar i rasteritzar triangles
	// - 2.4: Les mateixes dades s'utilitzen per crear múltiples entitats animades

	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::vector<Vector2> uvs;

public:

	Mesh();
	void Clear();
	void Render(int primitive = GL_TRIANGLES);

	void CreatePlane(float size);
	void CreateCube(float size);
	void CreateQuad();

	bool LoadOBJ(const char* filename);

	const std::vector<Vector3>& GetVertices() { return vertices; }
	const std::vector<Vector3>& GetNormals() { return normals; }
	const std::vector<Vector2>& GetUVs() { return uvs; }
};
