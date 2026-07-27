#include "EMesh.h"


inline uint64_t EMesh::makeEdgeKey(int32_t a, int32_t b){
    return (uint64_t(uint32_t(a)) << 32) | uint32_t(b); 
}

void EMesh::edgeMapAdd(VertexHandle a, VertexHandle b, HalfEdgeHandle he){
    uint64_t key = makeEdgeKey(static_cast<uint32_t>(a), static_cast<uint32_t>(b));
    if(_edgeMap.contains(key))
        LOG_WARNING("[EMesh::edgeMapAdd] The given key is already in _edgeMap V1:{},\tV2:{}",a,b);
    else
        _edgeMap[key] = he;
}
void EMesh::edgeMapDel(VertexHandle a, VertexHandle b){
    uint64_t key = makeEdgeKey(static_cast<uint32_t>(a), static_cast<uint32_t>(b));
    if(_edgeMap.contains(key))
        _edgeMap.erase(key);
    else
        LOG_WARNING("[EMesh::edgeMapDel] The given key is not found in the _edgeMap V1:{},\tV2:{}",a,b);
}

// a ve b valid bir hanndle mı kontrol et 
HalfEdgeHandle EMesh::addHalfEdge(VertexHandle a, VertexHandle b)
{
    _halfEdges.push_back(EHalfEdge());
    HalfEdgeHandle he = _halfEdges.size()-1;
    edgeMapAdd(a,b,he);
    return he;
}
HalfEdgeHandle EMesh::addHalfEdge(VertexHandle a, VertexHandle b, const EHalfEdge& he)
{
    if(a != he.origin)
        LOG_WARNING("[EMesh::addHalfEdge] a != he.origin a: {}, \the.origin: {}", a, he.origin);

    _halfEdges.push_back(he);
    HalfEdgeHandle he_handle = _halfEdges.size()-1;
    _edgeMap[makeEdgeKey(a, b)] = he_handle; 
    return he_handle;
}

int EMesh::countFaceEdges(HalfEdgeHandle he)
{
    constexpr int MAX_ITER = 100; 
    int counter = 0; 
    HalfEdgeHandle start = he;

    do
    {
        he = next(he);
        if(++counter >MAX_ITER) {
            LOG_WARNING("[EMesh::countFaceEdges] Inner loop count is more than MAX_ITER. Count operation aborted.");
            return -1;
        }        
    } while (start != he);
    
    return counter;
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
        HalfEdgeHandle he = innerHEs[(i+1)%innerHEs.size()];
        // half edgeleri eklerken döngüye i-1 den başlıyoruz: VertexHandle prevVertex = verts.back();
        // bu sebeple (i+1) yapmamız lazım yoksa döngü bir kayıyor.
        _vertices[vertex].edge = he;
    }

    // Face'in edge eşleşmesi
    _faces[fh].edge = innerHEs[0]; 
    
    return fh;
}

// Verilen Edge in tam ortasına bir Vertex daha ekler. 
//
// Not 0: h1 ve h4 halfedgeleri bu işlem esnasında silinebilirdi ve devamında 4 tane halfedge oluşturabilirdi.
// Fakat _halfEdges vektöründe 2 adet delik oluşturacaktık. Bunu temizleyen bir mekanizma elimizde şuanda yok. 
// Olsa bile ekstra cost'a girmenin bir anlamı yok. 
// 
// Not 1: Edge case'ler handle edilmedi. Örneğin: V1--V2 doğru parçası uzayda tek başına duruyorsa.
// Yani herhangi bir geometriye bağlı değilse ne yapılacağı şuanda belirsiz. 
VertexHandle EMesh::splitEdge(const HalfEdgeHandle& h1)
{   
    // Outer half edges and all connections
    //        * V0    
    //    h7 / \ h8       h7.twin = h0; h0.next = h1; h1.next = h2;   
    //      /   \         h1.twin = h4; h4.next = h5; h5.next = h6;
    //  V1 *-----* V2
    //      \   /
    //    h9 \ / h10
    //        * V3

    // Operation area (before ==> after)
    //     ---h1-->                ---h1-->    ---hn1-->
    //  V1 -------- V2   ==>    V1 -------- V4 --------- V2
    //     <--h4---                <--h4---    <--hn2---

    // Internal halfedges (before)
    //        * V0
    //    h0 / \ h2
    //      /   \ 
    //  V1 *---->* V2
    //        h1
    //
    //        h4
    //  V1 *<----* V2
    //      \   /
    //    h5 \ / h6
    //        * V3

    // Gerekli Handle'ları oluşturuyoruz
    HalfEdgeHandle h2, h4, h6, hn1, hn2;
    VertexHandle v1, v2, v4;

    // Vertex ve HalfEdge'leri eklemeden önce gerekli bağlantıları kaydediyoruz. 
    h2 = next(h1);
    h4 = twin(h1);
    h6 = prev(h4); 

    v1 = origin(h1);
    v2 = destination(h1);

    // Vertex'imi ekliyoruz
    glm::vec3 v1_pos = _vertices[v1].point;
    glm::vec3 v2_pos = _vertices[v2].point;
    v4 = addVertex((v1_pos+v2_pos)/2.f);

    // Yeni HalfEdge'leri ekliyoruz. 
    hn1 = addHalfEdge(v4, v2);
    hn2 = addHalfEdge(v2, v4);
    EHalfEdge& hn1e = _halfEdges[hn1];
    EHalfEdge& hn2e = _halfEdges[hn2];
    
    hn1e.face = face(h1);
    hn1e.origin = v4;
    hn1e.next = h2;
    hn1e.prev = h1;
    hn1e.twin = hn2;

    hn2e.face = face(h4);
    hn2e.origin = v2;
    hn2e.next = h4;
    hn2e.prev = h6;
    hn2e.twin = hn1;

    // Eski bağlantıları yeni düzene göre güncelliyoruz. 
    _vertices[v4].edge = hn1;
    _vertices[v2].edge = hn2;

    _halfEdges[h1].next = hn1;
    _halfEdges[h2].prev = hn1;
    _halfEdges[h4].origin = v4;
    _halfEdges[h4].prev = hn2;    
    _halfEdges[h6].next = hn2; 

    // Eski h1 ve h4 ü _edgeMap'te güncelliyoruz. 
    edgeMapDel(v1,v2);
    edgeMapDel(v2,v1);
    edgeMapAdd(v1,v4,h1);
    edgeMapAdd(v4,v1,h4);
    
    return v4;
}


// Verilen edge'i uygunsa saat yönünde 1 vertex döndürür. 
void EMesh::flipEdge(const HalfEdgeHandle &h1)
{

    // Outer half edges and all connections
    //        * V0    
    //    h7 / \ h8       h7.twin = h0; h0.next = h1; h1.next = h2;   
    //      /   \         h1.twin = h4; h4.next = h5; h5.next = h6;
    //  V1 *-----* V2
    //      \   /
    //    h9 \ / h10
    //        * V3

    // Operation area (before ==> after)
    //
    //     ---h1-->                   V0
    //  V1 -------- V2   ==>        A | |
    //     <--h4---                 | | |h1       
    //                            h4| | |
    //                              | | v
    //                                V3     

    // Internal halfedges (before)
    //        * V0
    //    h0 / \ h2
    //      /   \ 
    //  V1 *---->* V2
    //        h1
    //
    //        h4
    //  V1 *<----* V2
    //      \   /
    //    h5 \ / h6
    //        * V3

    // Uygunluk kontrolü 
    if(isInvalid(h1) || isInvalid(twin(h1))){
        LOG_ERROR("[EMesh::flipEdge][{}] The halfedge or its twin is invalid. Operation aborted.", h1);
        return;
    }
    if(isInvalid(face(h1)) || isInvalid(face(twin(h1)))){
        LOG_ERROR("[EMesh::flipEdge][{}] Cannot flip a boundary edge. Operation aborted.", h1);
        return;
    }

    int h1LoopEdgeCount = countFaceEdges(h1);
    int h4LoopEdgeCount = countFaceEdges(twin(h1));
    if(h1LoopEdgeCount != 3 || h4LoopEdgeCount != 3){
        LOG_ERROR("[EMesh::flipEdge][{}] The edge is not shared by two triangular faces. Operation aborted.\
            \n h1LoopEdgeCount:{}\t h4LoopEdgeCount:{}", h1, h1LoopEdgeCount, h4LoopEdgeCount);
        return;
    }
    VertexHandle flipV0 = origin(prev(h1)); // V0
    VertexHandle flipV1 = origin(prev(twin(h1))); // V3
    if(hasEdge(flipV0, flipV1)){
        LOG_ERROR("[EMesh::flipEdge][{}] Edge flip would create a duplicate edge. Operation aborted", h1);
        return;
    }

    // Gerekli handle'ların oluşturulması ve curent state'in kaydedilmesi
    HalfEdgeHandle h0, h2, h4, h5, h6;
    FaceHandle f0, f1;
    VertexHandle v0, v1, v2, v3;
    
    h0 = prev(h1);
    h2 = next(h1);
    h4 = twin(h1);

    h5 = next(h4);
    h6 = prev(h4);

    f0 = face(h1);
    f1 = face(h4);

    v0 = origin(h0);
    v1 = origin(h1);
    v2 = origin(h2);
    v3 = origin(h6);

    // halfedge bağlantılarının yapılması
    EHalfEdge &he0 = _halfEdges[h0], &he1 = _halfEdges[h1], &he2 = _halfEdges[h2];
    EHalfEdge &he4 = _halfEdges[h4], &he5 = _halfEdges[h5], &he6 = _halfEdges[h6];

    he1.next = h6;
    he1.prev = h2;
    he1.origin = v0;

    he4.next = h0;
    he4.prev = h5;
    he4.origin = v3;

    he0.next = h5;
    he0.prev = h4;
    he0.face = f1;

    he2.next = h1;
    he2.prev = h6;
    he2.face = f0;

    he5.next = h4;
    he5.prev = h0;
    he5.face = f1;

    he6.next = h2;
    he6.prev = h1;
    he6.face = f0;

    // operasyon sonrası vertex ve face edge'leri değişmiş olabilir 
    // o sebeple bağlantılarını tazeliyoruz. 
    _vertices[v1].edge = h5; 
    _vertices[v2].edge = h2; 

    _faces[f0].edge = h1;
    _faces[f1].edge = h4;

    // edgeMap in güncellenmesi
    edgeMapDel(v1,v2);
    edgeMapDel(v2,v1);
    edgeMapAdd(v0,v3,h1);
    edgeMapAdd(v3,v0,h4);
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
                //LOG_WARNING("counter: {}, h: {}, he: {}, he.orig.edge: {} ", counter, h, he, he_origin_edge);
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