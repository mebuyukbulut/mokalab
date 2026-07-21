#include "Mesh.h"
#include <glad/gl.h>
#include <iostream>
#include "Builtin.h"
#include "Logger.h"
#include <EMesh.h>



void Mesh::setupMesh()
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * _indices.size(), _indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
}


//void Mesh::init(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
//{
//    _vertices = vertices;
//    _indices = indices;
//    _textures = textures;
//    setupMesh();
//}

void Mesh::init(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
    _vertices = vertices;
    _indices = indices;
}

void Mesh::upload2GPU(){
    setupMesh();
}

void Mesh::draw(Shader* shader)
{
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
}


void Mesh::terminate(){
    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
}




Mesh MeshFactory::createCube()
{
    std::vector<Vertex> cubeVertices =
    {
        // Front (+Z)
        {{-0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {0, 0}},
        {{ 0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {1, 0}},
        {{ 0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {1, 1}},
        {{-0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {0, 1}},

        // Back (-Z)
        {{ 0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {1, 0}},
        {{-0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {1, 1}},
        {{ 0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {0, 1}},

        // Left (-X)
        {{-0.5f, -0.5f, -0.5f}, {-1,  0,  0}, {0, 0}},
        {{-0.5f, -0.5f,  0.5f}, {-1,  0,  0}, {1, 0}},
        {{-0.5f,  0.5f,  0.5f}, {-1,  0,  0}, {1, 1}},
        {{-0.5f,  0.5f, -0.5f}, {-1,  0,  0}, {0, 1}},

        // Right (+X)
        {{ 0.5f, -0.5f,  0.5f}, { 1,  0,  0}, {0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, { 1,  0,  0}, {1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, { 1,  0,  0}, {1, 1}},
        {{ 0.5f,  0.5f,  0.5f}, { 1,  0,  0}, {0, 1}},

        // Top (+Y)
        {{-0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {0, 0}},
        {{ 0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {1, 1}},
        {{-0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {0, 1}},

        // Bottom (-Y)
        {{-0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {1, 0}},
        {{ 0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {1, 1}},
        {{-0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {0, 1}},
    };

    std::vector<uint32_t> cubeIndices =
    {
        0, 1, 2,  0, 2, 3,        // Front
        4, 5, 6,  4, 6, 7,        // Back
        8, 9,10,  8,10,11,        // Left
        12,13,14, 12,14,15,       // Right
        16,17,18, 16,18,19,       // Top
        20,21,22, 20,22,23        // Bottom
    };

    Mesh cubeMesh;
    cubeMesh.init(cubeVertices, cubeIndices);
    cubeMesh.upload2GPU();
    return cubeMesh;


}
OMesh MeshFactory::createCone()
{
    OMesh mesh;
    std::vector <OMesh::VertexHandle> vhandle;

    // parametrelerimiz 
    int resolution = 32; // number of edges 
    float radius = 1; 
    float height = 2; 


    // vertexleri oluşturuyoruz 
    std::vector<Vertex> base_vertices;
    for (int i{}; i < resolution; i++) {
        float ratio = static_cast<float>(i) / (resolution);
        float angle = ratio * (M_PI * 2.0);
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        vhandle.push_back(mesh.add_vertex(OMesh::Point(x, 0, z)));
    }
    int centerIndex = vhandle.size();
    vhandle.push_back(mesh.add_vertex(OMesh::Point(0, 0, 0))); // center of cone floor
    int tipIndex = vhandle.size();
    vhandle.push_back(mesh.add_vertex(OMesh::Point(0, height, 0))); // tip of cone 


    // Yüzeylerin vertex indexlerini belirliyoruz:
    std::vector<std::vector<int>> faces;    

    for (int i{}; i < resolution-1; i++) {
        faces.push_back({i, i + 1, centerIndex }); // flor
        faces.push_back({i + 1, i, tipIndex }); // sidewall
    }
    // Son trisleri ekliyoruz:
    faces.push_back({resolution - 1, 0, centerIndex });
    faces.push_back({0, resolution - 1, tipIndex });


    // Belirlediğimiz indexlerdeki vertexlerden face oluşturup mesh e ekliyoruz:
    std::vector<OMesh::VertexHandle> face_vhandles;
    for (std::vector<int>& face : faces) {
        face_vhandles.clear();
        for (int& vertexIndex : face)
            face_vhandles.push_back(vhandle[vertexIndex]);
        mesh.add_face(face_vhandles);
    }
    return mesh;
}
OMesh MeshFactory::createCylinder()
{
    OMesh mesh;
    std::vector <OMesh::VertexHandle> vhandle;

    // parametrelerimiz 
    int resolution = 32; // number of edges 
    float radius = 1;
    float height = 2;

    // vertexleri oluşturuyoruz 
    std::vector<Vertex> base_vertices;
    for (int i{}; i < resolution; i++) {
        float ratio = static_cast<float>(i) / (resolution);
        float angle = ratio * (M_PI * 2.0);
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        vhandle.push_back(mesh.add_vertex(OMesh::Point(x, height / 2, z))); // top 
        vhandle.push_back(mesh.add_vertex(OMesh::Point(x, -height / 2, z))); // bottom
    }

    int centerIndexTop = vhandle.size();
    vhandle.push_back(mesh.add_vertex(OMesh::Point(0, height / 2, 0))); // center of cylinder ceil/top
    int centerIndexBottom = vhandle.size();
    vhandle.push_back(mesh.add_vertex(OMesh::Point(0, -height / 2, 0))); // center of cylinder floor/bottom



    // Yüzeylerin vertex indexlerini belirliyoruz:
    std::vector<std::vector<int>> faces;

    // iff resolution = 4 ->
    // 0 2 4 6 // top
    // 1 3 5 7 // bottom
    int lastTopIndex = resolution * 2 - 2;
    int lastBottomIndex = resolution * 2 - 1;
    for (int i{}, j{1}; i < lastTopIndex; i += 2, j+=2) {
        faces.push_back({ i + 2, i, centerIndexTop }); // top
        faces.push_back({ j, j + 2, centerIndexBottom }); // bottom

        faces.push_back({ i, i+2, j+2, j }); // sidewall
    }
    // Son trisleri ekliyoruz:
    faces.push_back({ 0, lastTopIndex,  centerIndexTop });
    faces.push_back({ lastBottomIndex, 1, centerIndexBottom });
    faces.push_back({ 0, 1, lastBottomIndex, lastTopIndex });



    // Belirlediğimiz indexlerdeki vertexlerden face oluşturup mesh e ekliyoruz:
    std::vector<OMesh::VertexHandle> face_vhandles;
    for (std::vector<int>& face : faces) {
        face_vhandles.clear();
        for (int& vertexIndex : face)
            face_vhandles.push_back(vhandle[vertexIndex]);
        mesh.add_face(face_vhandles);
    }
    return mesh;
}
Mesh MeshFactory::createPlane()
{    
    EMesh tris; 
    VertexHandle v0 = tris.addVertex({-1.f, 0.f, 1.f});
    VertexHandle v1 = tris.addVertex({ 1.f, 0.f, 1.f});
    VertexHandle v2 = tris.addVertex({ 1.f, 0.f,-1.f});
    VertexHandle v3 = tris.addVertex({-1.f, 0.f,-1.f});
    
    tris.addFace({v0, v1, v2});
    tris.addFace({v0, v2, v3});

    return tris.construct();
}
OMesh MeshFactory::createTorus()
{
    OMesh mesh;
    std::vector <OMesh::VertexHandle> vhandle;

    // parametrelerimiz 
    int radial_resolution = 16;  // küçük dairenin kenar sayısı
    int tubular_resolution = 32; // büyük dairenin kenar sayısı 
    float radius = 1;
    float thickness = 0.3; 
    float height = 2;

    // vertexleri oluşturuyoruz 
    for (size_t i = 0; i < radial_resolution; i++) {
        for (size_t j = 0; j < tubular_resolution; j++) {
            float u = (float)j / tubular_resolution * M_PI * 2.0;
            float v = (float)i / radial_resolution * M_PI * 2.0;
            float x = (radius + thickness * std::cos(v)) * std::cos(u);
            float z = (radius + thickness * std::cos(v)) * std::sin(u);
            float y = thickness * std::sin(v); 
            
            vhandle.push_back(mesh.add_vertex(OMesh::Point(x, y, z)));
        }
    }


    // add quad faces
    std::vector<OMesh::VertexHandle> face_vhandles;
    for (int i = 0; i < radial_resolution; i++) {
        int i_next = (i + 1) % radial_resolution;
        for (int j = 0; j < tubular_resolution; j++) {
            int j_next = (j + 1) % tubular_resolution;
            int i0 = i * tubular_resolution + j;
            int i1 = i * tubular_resolution + j_next;
            int i2 = i_next * tubular_resolution + j_next;
            int i3 = i_next * tubular_resolution + j;

            face_vhandles.clear();
            face_vhandles.push_back(vhandle[i0]);
            face_vhandles.push_back(vhandle[i3]); 
            face_vhandles.push_back(vhandle[i2]);
            face_vhandles.push_back(vhandle[i1]);
            mesh.add_face(face_vhandles);
        }
    }

    return mesh;
}

Mesh MeshFactory::createBgPlane()
{
    // Initialize background
    Mesh _bgMesh;
    std::vector<Vertex> bgVertices{
        {{-1,-1, 0}, {0,0,0}, {0,0}},
        {{ 3,-1, 0}, {0,0,0}, {2,0}},
        {{-1, 3, 0}, {0,0,0}, {0,2}},
    };
    std::vector<unsigned int> bgIndices{ 0, 1, 2 };
    _bgMesh.init(bgVertices, bgIndices);
    _bgMesh.upload2GPU();
    return _bgMesh;
}

Mesh MeshFactory::createGridPlane()
{
    // Initialize grid 
    Mesh _gridMesh;
    std::vector<Vertex> gridVertices{
        {{ -10, 0, -10}, {0,0,0}, {0,0}}, // a 0
        {{ -10, 0,  10}, {0,0,0}, {0,0}}, // b 1    d-c
        {{  10, 0,  10}, {0,0,0}, {0,0}}, // c 2    a-b
        {{  10, 0, -10}, {0,0,0}, {0,0}}, // d 3
    };
    std::vector<unsigned int> gridIndices{ 0, 1, 2, 0, 2, 3 };
    _gridMesh.init(gridVertices, gridIndices);
    _gridMesh.upload2GPU();
    return _gridMesh;
}

Mesh MeshFactory::create(std::string pathStr)
{
    if(pathStr == Builtin::Model::BgPlane)
        return createBgPlane();
    else if(pathStr == Builtin::Model::GridPlane)
        return createGridPlane();
    else if(pathStr == Builtin::Model::Cube)
        return createCube();
    else if(pathStr == Builtin::Model::Plane)
        return createPlane();

    //typedef OMesh(*BuiltinLoader)(void);
    using BuiltinLoader = OMesh(*)(void);

    static const std::unordered_map<std::string, BuiltinLoader> builtinLoaders = {
        {Builtin::Model::Cone, &MeshFactory::createCone},
        {Builtin::Model::Cylinder, &MeshFactory::createCylinder},
        //{Builtin::Model::Sphere, &MeshFactory::createSphere},
        {Builtin::Model::Torus, &MeshFactory::createTorus},
    };

    OMesh mesh;

    auto it = builtinLoaders.find(pathStr);
    if (it != builtinLoaders.end()) 
        mesh = it->second();
    else{
        LOG_ERROR("{} not found in MeshFactory create()!", pathStr);
        mesh = createCone();
    }
    






    // 🔹 Normalleri hesapla
    mesh.request_face_normals();
    mesh.update_face_normals();
    //mesh.triangulate(); // OpenGL için üçgenlere ayır




    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;


    // 🔹 Flat shading: her yüzey için ayrı vertex oluştur
    for (auto f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it)
    {
        OpenMesh::Vec3f faceNormal = mesh.normal(*f_it);
        std::vector<unsigned int> local_indices;

        for (auto fv_it = mesh.fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
        {
            auto p = mesh.point(*fv_it);

            unsigned int index = static_cast<unsigned int>(vertices.size());
            vertices.push_back(Vertex{
                glm::vec3(p[0], p[1], p[2]),
                glm::vec3(faceNormal[0], faceNormal[1], faceNormal[2]),
                glm::vec2(0.0f, 0.0f)
                });

            local_indices.push_back(index);
        }

        // CCW
        //  3●-----●2
        //  |    / |
        //  |  /   |
        //  0●-----●1

        // yüz üçgen mi?
        if (local_indices.size() == 3) {
            indices.push_back(local_indices[0]);
            indices.push_back(local_indices[1]);
            indices.push_back(local_indices[2]);
        }
        // quad ise triangulate et
        else if (local_indices.size() == 4) {
            indices.push_back(local_indices[0]);
            indices.push_back(local_indices[1]);
            indices.push_back(local_indices[2]);
            indices.push_back(local_indices[0]);
            indices.push_back(local_indices[2]);
            indices.push_back(local_indices[3]);
        }
    }

    Mesh m; 
    m.init(vertices, indices);
    m.upload2GPU();
    return m;
}
