#include "EMesh.h"


inline uint64_t EMesh::makeEdgeKey(int32_t a, int32_t b){
    return (uint64_t(uint32_t(a)) << 32) | uint32_t(b); 
}

VertexHandle EMesh::addVertex(const glm::vec3 &pos)
{
    _vertices.push_back({pos});
    return static_cast<int32_t>(_vertices.size()-1);
}


// Default orientation is CCW
FaceHandle EMesh::addFace(const std::vector<VertexHandle> &verts)
{

    for(int32_t i : verts)
        if(i<0 || i >= _vertices.size()){
            LOG_ERROR("EMesh::addFace : One of the given vertex indexes is out of bounds.");
            return InvalidHandle;
        }

    if(verts.size()<2){
        LOG_ERROR("EMesh::addFace : The given vertices count is not enough for create a face.");
        return InvalidHandle;
    }
    if(verts.size()>4){
        LOG_ERROR("EMesh::addFace : Ngons not supported for now.");
        return InvalidHandle;
    }

    _faces.push_back(EFace());
    FaceHandle fh = _faces.size()-1;


    std::vector<HalfEdgeHandle> innerHEs;
    std::vector<HalfEdgeHandle> outerHEs;

    VertexHandle prevVertex = verts.back();
    for(VertexHandle b : verts){ // b  0 -> N
        VertexHandle a = prevVertex;   // a -1 -> N-1
        prevVertex = b;

        if(_edgeMap.contains(makeEdgeKey(a, b))){ // varsa listeye ekle 
            HalfEdgeHandle he = _edgeMap[makeEdgeKey(a, b)];
            if(_halfEdges[he].face != InvalidHandle)
                LOG_ERROR("EMesh::addFace : Invalid geometry.");
            
            _halfEdges[he].face = fh;
            innerHEs.push_back(he);

        }
        else{ // yoksa oluşturup listeye ekle 
            _halfEdges.push_back(EHalfEdge());
            HalfEdgeHandle he = _halfEdges.size()-1;
            _edgeMap[makeEdgeKey(a, b)] = he; 
            
            _halfEdges[he].face = fh;
            _halfEdges[he].origin = a;
            innerHEs.push_back(he);
        }
        // aynı olayın tersini twinler için yap 

        if(_edgeMap.contains(makeEdgeKey(b, a))){ // varsa listeye ekle 
            HalfEdgeHandle he = _edgeMap[makeEdgeKey(b, a)];            
            outerHEs.push_back(he);
        }
        else{ // yoksa oluşturup listeye ekle 
            _halfEdges.push_back(EHalfEdge());
            HalfEdgeHandle he = _halfEdges.size()-1;            
            _edgeMap[makeEdgeKey(b, a)] = he; 

            _halfEdges[he].face = InvalidHandle;
            _halfEdges[he].origin = b;
            outerHEs.push_back(he);
        }

    }

    // Twin eşleşmesi
    for(int a = 0, b = 0 ; a < innerHEs.size(); a++, b++){
        HalfEdgeHandle he = innerHEs[a];
        HalfEdgeHandle ht = outerHEs[b];
        _halfEdges[he].twin = ht;
        _halfEdges[ht].twin = he;
    }


    // Outer halfedges prev-next eşleşmesi
    HalfEdgeHandle prevHE = innerHEs.back();
    for(HalfEdgeHandle he_next : innerHEs){ // b  0 -> N
        HalfEdgeHandle he = prevHE;   // a -1 -> N-1
        prevHE = he_next;

        HalfEdgeHandle a = InvalidHandle;
        HalfEdgeHandle b = InvalidHandle;

        HalfEdgeHandle he_twin = twin(he);
        HalfEdgeHandle he_next_twin = twin(he_next);

        // invalid -> invalid
        if(face(he_twin) == InvalidHandle && face(he_next_twin) == InvalidHandle ){
            a = he_next_twin;
            b = he_twin;
        }
        // invalid -> valid
        else if(face(he_twin) == InvalidHandle && face(he_next_twin) != InvalidHandle ){
            a = prev(he_next);
            b = he_twin;
        }
        // valid -> invalid
        else if(face(he_twin) != InvalidHandle && face(he_next_twin) == InvalidHandle ){
            a = twin(he_next);
            b = next(he);
        }
        // valid -> valid için bir şey yapmamız gerek yok. 
        else
            continue;


        if(true){
            LOG_TRACE("A: {} \t B: {}", a, b);
        }

        // outer he's arası bağlantı yapılır: 
        _halfEdges[a].next = b;
        _halfEdges[b].prev = a; 
    }


    // Inner halfedges prev-next eşleşmesi
    prevHE = innerHEs.back();
    for(HalfEdgeHandle he_next : innerHEs){ // b  0 -> N
        HalfEdgeHandle he = prevHE;   // a -1 -> N-1
        prevHE = he_next;

        _halfEdges[he].next = he_next;
        _halfEdges[he_next].prev = he;        
    }

    // Vertexlerin outer'ları eşleştirme
    for(int i = 0; i<verts.size(); i++){
        VertexHandle vertex = verts[i];
        HalfEdgeHandle he = innerHEs[i];

        _vertices[vertex].edge = he;
    }

    // Face'in edge eşleşmesi
    _faces[fh].edge = innerHEs[0]; 
    


    // // Face eşleşmesi 
    // EHalfEdge h1, h2, h3; 
    // h1.face = fh;
    // h2.face = fh;
    // h3.face = fh;
    
    // EHalfEdge ht1, ht2, ht3; 
    // ht1.face = InvalidHandle;
    // ht2.face = InvalidHandle;
    // ht3.face = InvalidHandle;

    // // Origin eşleşmesi
    // h1.origin = verts[0];
    // h2.origin = verts[1];
    // h3.origin = verts[2];
    
    // ht1.origin = verts[1];
    // ht2.origin = verts[2];
    // ht3.origin = verts[0];

    // // half edge handle'ların oluşturulması
    // HalfEdgeHandle h_h1, h_h2, h_h3, h_ht1, h_ht2, h_ht3;

    // _halfEdges.push_back(h1);
    // h_h1 = _halfEdges.size();
    // _halfEdges.push_back(h2);
    // h_h2 = _halfEdges.size();
    // _halfEdges.push_back(h3);
    // h_h3 = _halfEdges.size();

    // _halfEdges.push_back(ht1);
    // h_ht1 = _halfEdges.size();
    // _halfEdges.push_back(ht2);
    // h_ht2 = _halfEdges.size();
    // _halfEdges.push_back(ht3);
    // h_ht3 = _halfEdges.size();

    // // Twin eşleşmesi
    // _halfEdges[h_h1].twin = h_ht1;
    // _halfEdges[h_h2].twin = h_ht2;
    // _halfEdges[h_h3].twin = h_ht3;

    // _halfEdges[h_ht1].twin = h_h1;
    // _halfEdges[h_ht2].twin = h_h2;
    // _halfEdges[h_ht3].twin = h_h3;
    
    // // Next eşleşmesi
    // _halfEdges[h_h1].next = h_h2;
    // _halfEdges[h_h2].next = h_h3;
    // _halfEdges[h_h3].next = h_h1;

    // _halfEdges[h_ht1].next = h_ht2;
    // _halfEdges[h_ht2].next = h_ht3;
    // _halfEdges[h_ht3].next = h_ht1;

    // // Prev eşleşmesi
    // _halfEdges[h_h1].prev = h_h3;
    // _halfEdges[h_h2].prev = h_h1;
    // _halfEdges[h_h3].prev = h_h2;

    // _halfEdges[h_ht1].prev = h_ht3;
    // _halfEdges[h_ht2].prev = h_ht1;
    // _halfEdges[h_ht3].prev = h_ht2;
    
    // // Vertex'leri edge eşleşmesi
    // _vertices[verts[0]].edge = h_h1;
    // _vertices[verts[1]].edge = h_h2;
    // _vertices[verts[2]].edge = h_h3;
    
    // // Face'in edge eşleşmesi
    // _faces[fh].edge = h_h1;


    


    return fh;
}

Mesh EMesh::construct()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;


    // 🔹 Flat shading: her yüzey için ayrı vertex oluştur
    for (const EFace& fh : _faces)
    {
        EHalfEdge& he = _halfEdges[fh.edge];
        EHalfEdge& he_next = _halfEdges[he.next];
        EHalfEdge& he_prev = _halfEdges[he.prev];

        // Normali hesapla
        glm::vec3 A = _vertices[he.origin].point;
        glm::vec3 B = _vertices[he_next.origin].point;
        glm::vec3 C = _vertices[he_prev.origin].point;

        glm::vec3 AB = B-A;
        glm::vec3 AC = C-A;

        glm::vec3 faceNormal = glm::cross(AB, AC);

        //OpenMesh::Vec3f faceNormal = mesh.normal(*f_it);
        std::vector<unsigned int> local_indices;

        HalfEdgeHandle heIter = fh.edge;
        do
        {
            glm::vec3 pos = _vertices[_halfEdges[heIter].origin].point;
            vertices.push_back(Vertex{
                pos,
                faceNormal,
                glm::vec2(0.0f, 0.0f)
            });

            unsigned int index = static_cast<unsigned int>(vertices.size())-1;
            local_indices.push_back(index);


            heIter = next(heIter); 
        } while (heIter != fh.edge);
        

        // for (auto fv_it = mesh.fv_iter(*f_it); fv_it.is_valid(); ++fv_it)
        // {
        //     auto p = mesh.point(*fv_it);

        //     unsigned int index = static_cast<unsigned int>(vertices.size());
        //     vertices.push_back(Vertex{
        //         glm::vec3(p[0], p[1], p[2]),
        //         glm::vec3(faceNormal[0], faceNormal[1], faceNormal[2]),
        //         glm::vec2(0.0f, 0.0f)
        //         });

        //     local_indices.push_back(index);
        // }

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
