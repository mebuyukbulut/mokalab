#include "Mesh.h"
#include <glad/gl.h>
#include <iostream>
#include <algorithm>
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

    EMesh mesh; 
    VertexHandle v0 = mesh.addVertex({-0.5f, -0.5f,  0.5f});
    VertexHandle v1 = mesh.addVertex({ 0.5f, -0.5f,  0.5f});
    VertexHandle v2 = mesh.addVertex({ 0.5f,  0.5f,  0.5f});
    VertexHandle v3 = mesh.addVertex({-0.5f,  0.5f,  0.5f});

    VertexHandle v4 = mesh.addVertex({ 0.5f, -0.5f, -0.5f});
    VertexHandle v5 = mesh.addVertex({-0.5f, -0.5f, -0.5f});
    VertexHandle v6 = mesh.addVertex({-0.5f,  0.5f, -0.5f});
    VertexHandle v7 = mesh.addVertex({ 0.5f,  0.5f, -0.5f});

    mesh.addFace({v0,v1,v2,v3});
    mesh.addFace({v4,v5,v6,v7});

    mesh.addFace({v5,v4,v1,v0});
    mesh.addFace({v3,v2,v7,v6});

    mesh.addFace({v1,v4,v7,v2});
    mesh.addFace({v0,v3,v6,v5});



    return mesh.construct();

    // std::vector<Vertex> cubeVertices =
    // {
    //     // Front (+Z)
    //     {{-0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {0, 0}},
    //     {{ 0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {1, 0}},
    //     {{ 0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {1, 1}},
    //     {{-0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {0, 1}},

    //     // Back (-Z)
    //     {{ 0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {0, 0}},
    //     {{-0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {1, 0}},
    //     {{-0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {1, 1}},
    //     {{ 0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {0, 1}},

    //     // Left (-X)
    //     {{-0.5f, -0.5f, -0.5f}, {-1,  0,  0}, {0, 0}},
    //     {{-0.5f, -0.5f,  0.5f}, {-1,  0,  0}, {1, 0}},
    //     {{-0.5f,  0.5f,  0.5f}, {-1,  0,  0}, {1, 1}},
    //     {{-0.5f,  0.5f, -0.5f}, {-1,  0,  0}, {0, 1}},

    //     // Right (+X)
    //     {{ 0.5f, -0.5f,  0.5f}, { 1,  0,  0}, {0, 0}},
    //     {{ 0.5f, -0.5f, -0.5f}, { 1,  0,  0}, {1, 0}},
    //     {{ 0.5f,  0.5f, -0.5f}, { 1,  0,  0}, {1, 1}},
    //     {{ 0.5f,  0.5f,  0.5f}, { 1,  0,  0}, {0, 1}},

    //     // Top (+Y)
    //     {{-0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {0, 0}},
    //     {{ 0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {1, 0}},
    //     {{ 0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {1, 1}},
    //     {{-0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {0, 1}},

    //     // Bottom (-Y)
    //     {{-0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {0, 0}},
    //     {{ 0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {1, 0}},
    //     {{ 0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {1, 1}},
    //     {{-0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {0, 1}},
    // };

    // std::vector<uint32_t> cubeIndices =
    // {
    //     0, 1, 2,  0, 2, 3,        // Front
    //     4, 5, 6,  4, 6, 7,        // Back
    //     8, 9,10,  8,10,11,        // Left
    //     12,13,14, 12,14,15,       // Right
    //     16,17,18, 16,18,19,       // Top
    //     20,21,22, 20,22,23        // Bottom
    // };

    // Mesh cubeMesh;
    // cubeMesh.init(cubeVertices, cubeIndices);
    // cubeMesh.upload2GPU();
    // return cubeMesh;


}
Mesh MeshFactory::createCone()
{
    EMesh mesh;
    std::vector <VertexHandle> vhandle;

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
        vhandle.push_back(mesh.addVertex(glm::vec3(x, 0, z)));
    }
    VertexHandle centerIndex = mesh.addVertex(glm::vec3(0, 0, 0));
    VertexHandle tipIndex = mesh.addVertex(glm::vec3(0, height, 0));
    vhandle.push_back(centerIndex); // center of cone floor
    vhandle.push_back(tipIndex); // tip of cone 


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
    std::vector<VertexHandle> face_vhandles;
    for (std::vector<int>& face : faces) {
        face_vhandles.clear();
        for (int& vertexIndex : face)
            face_vhandles.push_back(vhandle[vertexIndex]);
        mesh.addFace(face_vhandles);
    }
    return mesh.construct();

}
Mesh MeshFactory::createCylinder()
{
    EMesh mesh;
    std::vector <VertexHandle> vhandle;

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
        vhandle.push_back(mesh.addVertex({x,  height / 2, z})); // top 
        vhandle.push_back(mesh.addVertex({x, -height / 2, z})); // bottom
    }

    VertexHandle centerIndexTop = mesh.addVertex({0, height / 2, 0});
    VertexHandle centerIndexBottom = mesh.addVertex({0, -height / 2, 0});
    vhandle.push_back(centerIndexTop); // center of cylinder ceil/top
    vhandle.push_back(centerIndexBottom); // center of cylinder floor/bottom



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
    std::vector<VertexHandle> face_vhandles;
    for (std::vector<int>& face : faces) {
        face_vhandles.clear();
        for (int& vertexIndex : face)
            face_vhandles.push_back(vhandle[vertexIndex]);
        mesh.addFace(face_vhandles);
    }

    return mesh.construct();

}
Mesh MeshFactory::createPlane()
{    
    EMesh mesh; 
    VertexHandle v0 = mesh.addVertex({-1.f, 0.f, 1.f});
    VertexHandle v1 = mesh.addVertex({ 1.f, 0.f, 1.f});
    VertexHandle v2 = mesh.addVertex({ 1.f, 0.f,-1.f});
    VertexHandle v3 = mesh.addVertex({-1.f, 0.f,-1.f});
    
    mesh.addFace({v0, v1, v2, v3});

    mesh.validate();
    return mesh.construct();
}
Mesh MeshFactory::createTorus()
{
    EMesh mesh;
    std::vector <VertexHandle> vhandle;

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
            
            vhandle.push_back(mesh.addVertex({x, y, z}));
        }
    }


    // add quad faces
    for (int i = 0; i < radial_resolution; i++) {
        int i_next = (i + 1) % radial_resolution;
        for (int j = 0; j < tubular_resolution; j++) {
            int j_next = (j + 1) % tubular_resolution;
            int i0 = i * tubular_resolution + j;
            int i1 = i * tubular_resolution + j_next;
            int i2 = i_next * tubular_resolution + j_next;
            int i3 = i_next * tubular_resolution + j;
            
            VertexHandle v0 = vhandle[i0];
            VertexHandle v1 = vhandle[i3];
            VertexHandle v2 = vhandle[i2];
            VertexHandle v3 = vhandle[i1];
            mesh.addFace({v0, v1, v2, v3});
        }
    }

    return mesh.construct();
}
Mesh MeshFactory::createUVSphere(){
    int segments = 32; // +Y etrafında kaç parçaya bölüneceği (silindir gibi)
    int rings = 16; // ekvatorla paralel olacak şekilde kaç parçaya bölüneceği 
    float radius = 0.5; // Yarıçap
    
    segments = std::max(3, segments);
    rings = std::max(3, rings);
    radius = std::max(0.0f, radius);

    
    // phi = 0 -> kuzey kutbu; 
    // phi = PI/2 -> ekvator;
    // phi = PI -> güney kutbu;
    // theta = PI * 0/2 -> +x
    // theta = PI * 1/2 -> +z
    // theta = PI * 2/2 -> -x
    // theta = PI * 3/2 -> -z
    // theat = PI * 4/2 -> +x

    float phi, theta;
    float x, y, z;

    std::vector<std::vector<VertexHandle>> vHandle;
    VertexHandle north{InvalidHandle}, south{InvalidHandle}; 

    EMesh mesh;
    north = mesh.addVertex({0.0f,  radius, 0.0f});
    south = mesh.addVertex({0.0f, -radius, 0.0f});
    
    constexpr float PI = 3.1415927f;

    for(int i = 1; i < rings; i++){
        // rings = 3 =>
        // PI*0/3, PI*1/3, PI*2/3, PI*3/3
        // we need just PI*1/3 and PI*2/3 so i=1
        phi = PI * static_cast<float>(i) / rings;

        std::vector<VertexHandle> vLocal{};
        for(int j = 0; j < segments; j++){
            theta = 2*PI * static_cast<float>(j) / segments;

            x = sin(phi) * cos(theta) * radius;
            y = cos(phi) * radius; // yükseklik 
            z = sin(phi) * sin(theta) * radius;

            VertexHandle vh = mesh.addVertex({x, y, z});
            vLocal.push_back(vh);
        }

        vHandle.push_back(vLocal);
    }


    // Orta kısımdaki Quad şeritleri ekliyoruz
    for(int i = 0; i < rings-2; i++){
        std::vector<VertexHandle>& topRing = vHandle[i];
        std::vector<VertexHandle>& bottomRing = vHandle[i+1];

        VertexHandle topPrev = topRing.back();
        VertexHandle bottomPrev = bottomRing.back();
        
        for(int j = 0; j < topRing.size(); j++){
            VertexHandle topB = topRing[j];
            VertexHandle bottomB = bottomRing[j];
            VertexHandle topA = topPrev;
            VertexHandle bottomA = bottomPrev;
            topPrev = topB;
            bottomPrev = bottomB; 

            mesh.addFace({topA, topB, bottomB, bottomA});
        } 
    }

    // Kutuplardaki tris leri ekliyoruz

    std::vector<VertexHandle>& northRing = vHandle.front();
    std::vector<VertexHandle>& southRing = vHandle.back();
    VertexHandle northPrev = northRing.back();
    VertexHandle southPrev = southRing.back();
    
    for(int j = 0; j < northRing.size(); j++){
        VertexHandle northB = northRing[j];
        VertexHandle northA = northPrev;
        northPrev = northB;
        VertexHandle southB = southRing[j];
        VertexHandle southA = southPrev;
        southPrev = southB; 

        mesh.addFace({northB, northA, north});
        mesh.addFace({southA, southB, south});
    } 
    
    mesh.validate();

    return mesh.construct();
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

    // // UV olayını yaptıktan sonra buna devam edebiliriz. 
    // EMesh bgMesh; 
    // VertexHandle v0 = bgMesh.addVertex({{-1,-1, 0}, {0,0,0}, {0,0}});
    // VertexHandle v1 = bgMesh.addVertex({{ 3,-1, 0}, {0,0,0}, {2,0}});
    // VertexHandle v2 = bgMesh.addVertex({{-1, 3, 0}, {0,0,0}, {0,2}});
    // bgMesh.addFace({v0, v1, v2});
    // return bgMesh.construct();
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
    if(true) 
        LOG_TRACE("MeshFactory::create::pathSTR : {}", pathStr);
    
    using BuiltinLoader = Mesh(*)(void);
    static const std::unordered_map<std::string, BuiltinLoader> builtinLoaders = {
        {Builtin::Model::BgPlane,   &MeshFactory::createBgPlane},
        {Builtin::Model::GridPlane, &MeshFactory::createGridPlane},
        {Builtin::Model::Cube,      &MeshFactory::createCube},
        {Builtin::Model::Plane,     &MeshFactory::createPlane},

        {Builtin::Model::Cone,      &MeshFactory::createCone},
        {Builtin::Model::Cylinder,  &MeshFactory::createCylinder},
        {Builtin::Model::Torus,     &MeshFactory::createTorus},
        {Builtin::Model::UVSphere,  &MeshFactory::createUVSphere},
    };

    auto it = builtinLoaders.find(pathStr);
    if (it != builtinLoaders.end()) 
        return it->second();
    else{
        LOG_ERROR("{} not found in MeshFactory create()!", pathStr);
        return Mesh();
    }
}

