#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include "Shader.h"

// #ifndef _USE_MATH_DEFINES
// #define _USE_MATH_DEFINES
// #endif
// #include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
// typedef OpenMesh::PolyMesh_ArrayKernelT<>  OMesh;


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

class Mesh
{
	std::vector<Vertex> _vertices;
	std::vector<unsigned int> _indices;

	unsigned int _vao, _vbo, _ebo;
	void setupMesh();
public:
	void init(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	void upload2GPU();
	void draw(Shader* shader);
	void terminate();
};

class MeshFactory {
	static Mesh createCone();
	static Mesh createCylinder();
	static Mesh createTorus();

	static Mesh createCube();
	static Mesh createPlane();
	static Mesh createBgPlane();
	static Mesh createGridPlane();

public:
	static Mesh create(std::string shape);
};