#include "EMesh.h"


inline uint64_t EMesh::makeEdgeKey(int32_t a, int32_t b){
    return (uint64_t(uint32_t(a)) << 32) | uint32_t(b); 
}

// Bir vertexden çıkan tüm halfedge'ler
std::vector<HalfEdgeHandle> EMesh::outgoingHalfEdges(VertexHandle v)
{
    std::vector<HalfEdgeHandle> HEs{};
    HalfEdgeHandle start = _vertices[v].edge;
    HalfEdgeHandle he = start;
    
    do{
        HEs.push_back(he);        
        he = next(twin(he));

        if (HEs.size() > _halfEdges.size()){
            LOG_ERROR("[EMesh::outgoingHalfEdges()][{}] \n \
            v.outgoingHalfEdges.size() > allHalfedges.size()  ",he);
            break;
        }
        
    }while(he!=start);

    return HEs;
}

// Bir face'e ait tüm vertex'ler
std::vector<VertexHandle> EMesh::verticesOfFace(FaceHandle f)
{
    HalfEdgeHandle start = edgeofFace(f);
    if(start == InvalidHandle) return{};

    HalfEdgeHandle he = start;
    std::vector<VertexHandle> VHs{};

    do
    {   
        VHs.push_back(origin(he));
        he = next(he);
    } while (he != start);
    

    return VHs;
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
            if(_halfEdges[he].face != InvalidHandle){
                LOG_ERROR("EMesh::addFace : Invalid geometry.");
                assert(false);
            }
            
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


        if(false){
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

        glm::vec3 faceNormal = glm::normalize(glm::cross(AB, AC));

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


// Validate whole mesh 
void EMesh::validate(){
    static const char* vPrefix = "[EMesh::validate]";
    const int MAX_ITER = 100; 
    // 1. Twin of the twin of a halfedge must be equal to the halfedge

    for(HalfEdgeHandle i = 0; i < _halfEdges.size(); i++){
        const HalfEdgeHandle he = i ;
        HalfEdgeHandle h = InvalidHandle;
        int counter = 0;

        int allHalfedgesCount = _halfEdges.size();
        int allVerticesCount = _vertices.size();
        int allFaceCount = _faces.size();

        HalfEdgeHandle he_twin = twin(he);
        HalfEdgeHandle he_next = next(he);
        HalfEdgeHandle he_prev = prev(he);
        VertexHandle he_origin = origin(he);
        FaceHandle he_face = face(he);  

        if(he_twin != InvalidHandle && (he_twin<0 || he_twin>=allHalfedgesCount))
            LOG_ERROR("{} [{}] he.twin is out of bounds", vPrefix, he_twin);
        if(he_next != InvalidHandle && (he_next<0 || he_next>=allHalfedgesCount))
            LOG_ERROR("{} [{}] he.next is out of bounds", vPrefix, he_next);
        if(he_prev != InvalidHandle && (he_prev<0 || he_prev>=allHalfedgesCount))
            LOG_ERROR("{} [{}] he.prev is out of bounds", vPrefix, he_prev);

        if(he_origin != InvalidHandle && (he_origin<0 || he_origin>=allVerticesCount))
            LOG_ERROR("{} [{}] he.origin is out of bounds", vPrefix, he_origin);

        if(he_face != InvalidHandle && (he_face<0 || he_face>=allFaceCount))
            LOG_ERROR("{} [{}] he.face is out of bounds", vPrefix, he_face);


        if(twin(twin(he)) != he)
            LOG_ERROR("{} [{}] Twin of the twin of a halfedge must be equal to the halfedge", vPrefix, he);

        if(origin(he) != destination(twin(he)))
            LOG_ERROR("{} [{}] he.orig != he.twin.destination", vPrefix, he);


        if(prev(next(he)) != he)
            LOG_ERROR("{} [{}] he.next.prev != he", vPrefix, he);

        if(next(prev(he)) != he)
            LOG_ERROR("{} [{}] he.prev.next != he", vPrefix, he);


        // -----<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>-----
        
        h = he;
        counter = 0; 
        FaceHandle f1 = face(he);
        do
        {
            if(counter++>MAX_ITER){
                LOG_ERROR("{} [{}] edge loop does not close or too big!", vPrefix, he);
                break;
            }
            if(f1 != face(h)){                
                LOG_ERROR("{} [{}] face of inner loop edges does not equal!", vPrefix, he);
                break;
            }
            h = next(h);
        } while (h!=he);

        // -----<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>-----

        HalfEdgeHandle he_face_edge = edgeofFace(face(he));
        if(he_face_edge != InvalidHandle){ // boundary he değilse
            h = he;
            counter = 0; 
            do
            {
                if(counter++>MAX_ITER){
                    LOG_ERROR("{} [{}] he.face.edge is not inside edge loop!", vPrefix, he);
                    break;
                }
                h = next(h);
            } while (h!=he_face_edge);
        }
        
        // -----<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>-----

        HalfEdgeHandle he_origin_edge = edgeofVertex(origin(he));
        if(he_origin_edge != InvalidHandle){
            h = he;
            counter = 0; 
            do
            {
                if(counter++>MAX_ITER){
                    LOG_ERROR("{} [{}] he.origin.edge is not inside outgoing halfedges!", vPrefix, he);
                    break;
                }
                h = next(twin(h));
            } while (h!=he_origin_edge);
        }
        else{
            LOG_ERROR("{} [{}] he.origin.edge is Invalid!", vPrefix, he);
        }

    }

}