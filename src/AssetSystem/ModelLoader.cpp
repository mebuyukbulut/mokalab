#include "AssetSystem/ModelLoader.h"
#include "Builtin.h"
#include "EngineContext.h"
#include "Logger.h"

std::shared_ptr<Asset> ModelLoader::load(std::filesystem::path path, IAssetSettings* settings)
{
    std::shared_ptr<Model> model = std::make_shared<Model>(_ece); 

    model->_path = path; // bunu Model::Load da yapabiliriz belki. 
    std::string pathStr = path.string(); 

    // Sanal yolla model yükleme
    for(const char* key : Builtin::Model::All){
        if(key == pathStr){
            loadDefault(model, pathStr);
            return model;
        }
    }

    // Fiziksel yolla model yükleme
    loadModel(model, pathStr);

    return model;
}

// TO-DO: Load fonksiyonlarını standartlaştırıp tek bir isme topla 
// Neden her şeyi ayrı bir isim ve fonksiyon gerekli olsun ki? 

void ModelLoader::loadDefault(std::shared_ptr<Model> model, std::string pathStr)
{
    model->_loadStatus = AssetLoadStatus::LoadingToCPU;

    // For simplicity, we will just load a default mesh here
    Mesh mesh = MeshFactory::create(pathStr);
    model->meshes.push_back(mesh);

    model->_materials.push_back(_ece->assets->get<Material>(Builtin::Material::DefaultMaterial));

	model->_loadStatus = AssetLoadStatus::Complete;
}

// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void ModelLoader::loadModel(std::shared_ptr<Model> model, const std::string& path)
{
    model->_loadStatus = AssetLoadStatus::LoadingToCPU;

    // read file via ASSIMP 
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        LOG_ERROR( "ASSIMP: {}", std::string(importer.GetErrorString()) );
        model->_loadStatus = AssetLoadStatus::Error;
        return;
    }

    // process ASSIMP's root node recursively
    processNode(model, scene->mRootNode, scene);

    //// if model loading not create a material use default one 
    //if (_materials.empty()) {
    //    _materials.push_back(g_Assets.get<Material>("engine::materials::defaultMaterial")); // TO-DO bu sabiti birden fazla yerde kullandık refactor et!
    //}

    model->_loadStatus = AssetLoadStatus::ReadyToUpload;
}



// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void ModelLoader::processNode(std::shared_ptr<Model> model, aiNode* node, const aiScene* scene)
{
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        model->meshes.push_back(processMesh(model, mesh, scene));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(model, node->mChildren[i], scene);
    }

}



Mesh ModelLoader::processMesh(std::shared_ptr<Model> model, aiMesh* mesh, const aiScene* scene)
{
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture*> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }
        // texture coordinates

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
            //// tangent
            //vector.x = mesh->mTangents[i].x;
            //vector.y = mesh->mTangents[i].y;
            //vector.z = mesh->mTangents[i].z;
            //vertex.Tangent = vector;
            //// bitangent
            //vector.x = mesh->mBitangents[i].x;
            //vector.y = mesh->mBitangents[i].y;
            //vector.z = mesh->mBitangents[i].z;
            //vertex.Bitangent = vector;
        }
        else
            vertex.texCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    AssetHandle<Material> matHandle = _ece->assets->get<Material>("internal::materials::redscarf");   
    auto mat = matHandle.resolve(_ece);  
    AssetHandle<Texture> baseTex = loadMaterialTextures(model, material, aiTextureType_DIFFUSE);
    AssetHandle<Texture> normalTex = loadMaterialTextures(model, material, aiTextureType_NORMALS);
    
    //AssetHandle<Texture> roughTex = loadMaterialTextures(model, material, aiTextureType_GLTF_METALLIC_ROUGHNESS);
    //AssetHandle<Texture> heightTex = loadMaterialTextures(model, material, aiTextureType_HEIGHT);
    //AssetHandle<Texture> aoTex = loadMaterialTextures(model, material, aiTextureType_AMBIENT);
    
    if (baseTex.isValid())
        mat->baseColorTexture = baseTex;
    if(normalTex.isValid())
        mat->normalTexture = normalTex;
    // if (roughTex.isValid())
    //     mat->roughnessTexture = roughTex;
    // if (heightTex.isValid())
    //     mat->heightTexture = heightTex;
    // if (aoTex.isValid())
    //     mat->aoTexture = aoTex;

    model->_materials.push_back(matHandle);

    // return a mesh object created from the extracted mesh data
    Mesh newMesh;
    newMesh.init(vertices, indices);// , textures);
    return newMesh;
}


// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
AssetHandle<Texture> ModelLoader::loadMaterialTextures(std::shared_ptr<Model> model, aiMaterial* mat, aiTextureType type)
{
    // has parent path? 
    //std::string directory = _path.parent_path().().string();
    std::string directory = (model->_path.parent_path() / "").string();

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString textureStr;
        mat->GetTexture(type, i, &textureStr);
        std::string filename = directory + textureStr.C_Str();

        // texture asekron yüklenmek zorunda. Yoksa modelle beraber non-main thread de opengl çağrısına giriyor. 
        AssetHandle<Texture> tex = _ece->assets->get<Texture>(filename, nullptr, true);
        if (tex.isValid()) {
            return tex; // ilk bulduğun texture ile devam et sonra birden fazla texture için destek koyarız
        }
    }
    return AssetHandle<Texture>();
}


