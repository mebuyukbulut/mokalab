#include "EMesh.h"
#include <algorithm>

// ==========================
// Allocation
// ==========================
VertexHandle EMesh::allocVertex()
{
    VertexHandle idx = InvalidHandle;

    if (!_freeVertices.empty())
    {
        idx = _freeVertices.back();
        _freeVertices.pop_back();
        _vertices[idx].edge = InvalidHandle;
    }
    else
    {
        idx = _vertices.size();
        _vertices.push_back(EVertex());
    }

    return idx;
}

HalfEdgeHandle EMesh::allocHalfEdge()
{
    HalfEdgeHandle idx = InvalidHandle;

    if (!_freeHalfEdges.empty())
    {
        idx = _freeHalfEdges.back();
        _freeHalfEdges.pop_back();
        _halfEdges[idx].origin = InvalidHandle;
    }
    else
    {
        idx = _halfEdges.size();
        _halfEdges.push_back(EHalfEdge());
    }

    return idx;
}

FaceHandle EMesh::allocFace()
{
    FaceHandle idx = InvalidHandle;

    if (!_freeFaces.empty())
    {
        idx = _freeFaces.back();
        _freeFaces.pop_back();
        _faces[idx].edge = InvalidHandle;
    }
    else
    {
        idx = _faces.size();
        _faces.push_back(EFace());
    }

    return idx;
}

void EMesh::freeVertex(VertexHandle v)
{ // Boundary check
    if (!isValidVertex(v))
    {
        LOG_ERROR("[EMesh::freeVertex][{}] VertexHandle is invalid: out of bounds or deleted. \
            Operation aborted!",
                  v);
        return;
    }

    // Datayı overwrite et
    _vertices[v].point = glm::vec3(0.f, 0.f, 0.f);
    _vertices[v].edge = TombstoneHandle;

    // freelist'e ekle
    _freeVertices.push_back(v);
}

void EMesh::freeHalfEdge(HalfEdgeHandle h)
{ // Boundary check
    if (!isValidHalfEdge(h))
    {
        LOG_ERROR("[EMesh::freeHalfEdge][{}] HalfEdgeHandle is invalid: out of bounds or deleted. \
            Operation aborted!",
                  h);
        return;
    }

    // Datayı overwrite et
    _halfEdges[h].face = InvalidHandle;
    _halfEdges[h].next = InvalidHandle;
    _halfEdges[h].prev = InvalidHandle;
    _halfEdges[h].twin = InvalidHandle;

    _halfEdges[h].origin = TombstoneHandle;

    // freelist'e ekle
    _freeHalfEdges.push_back(h);
}

void EMesh::freeFace(FaceHandle f)
{ // Boundary check
    if (!isValidFace(f))
    {
        LOG_ERROR("[EMesh::freeFace][{}] FaceHandle is invalid: out of bounds or deleted. \
            Operation aborted!",
                  f);
        return;
    }

    // Datayı overwrite et
    _faces[f].edge = TombstoneHandle;

    // freelist'e ekle
    _freeFaces.push_back(f);
}

// ==========================
// Edge Map
// ==========================
inline uint64_t EMesh::makeEdgeKey(int32_t a, int32_t b)
{
    return (uint64_t(uint32_t(a)) << 32) | uint32_t(b);
}

void EMesh::edgeMapAdd(VertexHandle a, VertexHandle b, HalfEdgeHandle he)
{
    uint64_t key = makeEdgeKey(static_cast<uint32_t>(a), static_cast<uint32_t>(b));
    if (_edgeMap.contains(key))
        LOG_WARNING("[EMesh::edgeMapAdd] The given key is already in _edgeMap V1:{},\tV2:{}", a, b);
    else
        _edgeMap[key] = he;
}
void EMesh::edgeMapDel(VertexHandle a, VertexHandle b)
{
    uint64_t key = makeEdgeKey(static_cast<uint32_t>(a), static_cast<uint32_t>(b));
    if (_edgeMap.contains(key))
        _edgeMap.erase(key);
    else
        LOG_WARNING("[EMesh::edgeMapDel] The given key is not found in the _edgeMap V1:{},\tV2:{}", a, b);
}

inline bool EMesh::hasEdge(VertexHandle a, VertexHandle b)
{
    return _edgeMap.contains(makeEdgeKey(static_cast<uint32_t>(a), static_cast<uint32_t>(b)));
}

// a ve b valid bir handle mı kontrol et
HalfEdgeHandle EMesh::addHalfEdge(VertexHandle a, VertexHandle b)
{
    HalfEdgeHandle he = allocHalfEdge();
    edgeMapAdd(a, b, he);
    return he;
}
HalfEdgeHandle EMesh::addHalfEdge(VertexHandle a, VertexHandle b, const EHalfEdge &he)
{
    if (a != he.origin)
        LOG_WARNING("[EMesh::addHalfEdge] a != he.origin a: {}, \the.origin: {}", a, he.origin);

    // HalfEdgeHandle he_handle = allocHalfEdge();
    _halfEdges.push_back(he);
    HalfEdgeHandle he_handle = _halfEdges.size() - 1;
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
        if (++counter > MAX_ITER)
        {
            LOG_WARNING("[EMesh::countFaceEdges] Inner loop count is more than MAX_ITER. Count operation aborted.");
            return -1;
        }
    } while (start != he);

    return counter;
}

std::vector<VertexHandle> EMesh::intersectHandles(std::vector<int32_t> a, std::vector<int32_t> b)
{
    std::vector<VertexHandle> common;

    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());

    std::set_intersection(
        a.begin(), a.end(),
        b.begin(), b.end(),
        std::back_inserter(common));

    return common;
}

std::vector<VertexHandle> EMesh::differenceHandles(std::vector<int32_t> a, std::vector<int32_t> b)
{
    std::vector<VertexHandle> diff;

    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());

    std::set_difference(
        a.begin(), a.end(),
        b.begin(), b.end(),
        std::back_inserter(diff));

    return diff;
}

// ==========================
// Validation
// ==========================
bool EMesh::isInvalid(const uint32_t& handle)
{
    return handle == InvalidHandle;
}
bool EMesh::isValidVertex(const VertexHandle& v)
{
    return !(v < 0 || v >= _vertices.size() || _vertices[v].edge == TombstoneHandle);
}
bool EMesh::isValidHalfEdge(const HalfEdgeHandle& h)
{
    return !(h < 0 || h >= _halfEdges.size() || _halfEdges[h].origin == TombstoneHandle);
}
bool EMesh::isValidFace(const FaceHandle& f)
{
    return !(f < 0 || f >= _faces.size() || _faces[f].edge == TombstoneHandle);
}

bool EMesh::isValidFace(const EFace& f)
{
    return f.edge != TombstoneHandle;
}

// Bir vertexden çıkan tüm halfedge'ler
std::vector<HalfEdgeHandle> EMesh::outgoingHalfEdges(VertexHandle v)
{
    std::vector<HalfEdgeHandle> HEs{};
    HalfEdgeHandle start = _vertices[v].edge;
    HalfEdgeHandle he = start;

    do
    {
        HEs.push_back(he);
        he = next(twin(he));

        if (HEs.size() > _halfEdges.size())
        {
            LOG_ERROR("[EMesh::outgoingHalfEdges()][{}] \n \
            v.outgoingHalfEdges.size() > allHalfedges.size()  ",
                      he);
            break;
        }

    } while (he != start);

    return HEs;
}

std::vector<VertexHandle> EMesh::adjacentVertices(VertexHandle v)
{
    std::vector<VertexHandle> adjVerts{};

    for (HalfEdgeHandle h : outgoingHalfEdges(v))
        adjVerts.push_back(destination(h));

    return adjVerts;
}

// Bir face'e ait tüm vertex'ler
std::vector<VertexHandle> EMesh::verticesOfFace(FaceHandle f)
{
    HalfEdgeHandle start = edgeofFace(f);
    if (start == InvalidHandle)
        return {};

    HalfEdgeHandle he = start;
    std::vector<VertexHandle> VHs{};

    do
    {
        VHs.push_back(origin(he));
        he = next(he);
    } while (he != start);

    return VHs;
}

// ==========================
// Construction
// ==========================

// Üçgenin yüzey normalini hesaplar
glm::vec3 EMesh::trisNormal(const glm::vec3 &A, const glm::vec3 &B, const glm::vec3 &C)
{
    constexpr float GeometryEpsilon = 1e-6f;

    glm::vec3 AB = B - A;
    glm::vec3 AC = C - A;

    glm::vec3 n = glm::cross(AB, AC);

    // Eğer cross product sıfıra yakın ise
    if (glm::dot(n, n) < GeometryEpsilon * GeometryEpsilon)
        return glm::vec3(0.0f);

    return glm::normalize(n);
}

// bir üçgenin içindeki en küçük açıyı radian cinsinden döndürür.
float EMesh::trisMinAngle(const glm::vec3 &A, const glm::vec3 &B, const glm::vec3 &C)
{
    //    A
    //   / \ 
    //  /   \ 
    // B-----C
    // CAB açısının hesaplanması:
    // theta = arccos((AB•AC) / (|AB||AC|))

    glm::vec3 AB = glm::normalize(B - A);
    glm::vec3 AC = glm::normalize(C - A);

    glm::vec3 BC = glm::normalize(C - B);
    glm::vec3 BA = -AB;

    glm::vec3 CA = -AC;
    glm::vec3 CB = -BC;

    // arccos öncesi numerik hataları önlemek için clamp yapmalıyız:
    float dotCAB = glm::clamp(glm::dot(AB, AC), -1.0f, 1.0f);
    float dotABC = glm::clamp(glm::dot(BC, BA), -1.0f, 1.0f);
    float dotBCA = glm::clamp(glm::dot(CA, CB), -1.0f, 1.0f);

    float CAB = acos(dotCAB);
    float ABC = acos(dotABC);
    float BCA = acos(dotBCA);

    return std::min({CAB, ABC, BCA});
}

// Verilen quad'ı triangulate etmek için en iyi diagonalı hesaplar.
// - Eğer en iyi diagonal AC ise true,
// - En iyi diagonal BD ise false döndürür.
// A -- D
// | \  |
// |  \ |
// B -- C
bool EMesh::useDiagonalAC(const glm::vec3 &A, const glm::vec3 &B, const glm::vec3 &C, const glm::vec3 &D)
{
    const float CoplanarThreshold = 1 - 0.1; // 1 -> maks value; 0.1 ->delta value

    // İki üçgenin kırılma açısını buluyoruz. Yani tris normallerinin arasındaki açı

    glm::vec3 abcNormal = trisNormal(A, B, C);
    glm::vec3 acdNormal = trisNormal(A, C, D);

    glm::vec3 abdNormal = trisNormal(A, B, D);
    glm::vec3 bcdNormal = trisNormal(B, C, D);

    float dotAC = glm::dot(abcNormal, acdNormal);
    float dotBD = glm::dot(abdNormal, bcdNormal);

    if (dotAC > CoplanarThreshold && dotBD > CoplanarThreshold)
    { // üçgenler yeterince paralel ise minimum açıyı bul.
        // minimum açıyı buluyoruz çünkü slim tris'ler istemiyoruz!
        float acMinAngle = std::min(trisMinAngle(A, B, C), trisMinAngle(A, C, D));
        float bdMinAngle = std::min(trisMinAngle(A, B, D), trisMinAngle(B, C, D));

        return acMinAngle > bdMinAngle;
    }
    else
    {                         // Yeterince coplanar değiller.
        return dotAC > dotBD; // Kırılma açısı daha küçük olan triangulation'ı seç.
    }
}

VertexHandle EMesh::addVertex(const glm::vec3 &pos)
{
    VertexHandle v = allocVertex();
    _vertices[v].point = pos;
    return v;
}

// Default orientation is CCW
FaceHandle EMesh::addFace(const std::vector<VertexHandle> &verts)
{
    for (int32_t i : verts)
        if (i < 0 || i >= _vertices.size())
        {
            LOG_ERROR("EMesh::addFace : One of the given vertex indexes is out of bounds.");
            return InvalidHandle;
        }

    if (verts.size() < 2)
    {
        LOG_ERROR("EMesh::addFace : The given vertices count is not enough for create a face.");
        return InvalidHandle;
    }
    if (verts.size() > 4)
    {
        LOG_ERROR("EMesh::addFace : Ngons not supported for now.");
        return InvalidHandle;
    }

    FaceHandle fh = allocFace();

    std::vector<HalfEdgeHandle> innerHEs;
    std::vector<HalfEdgeHandle> outerHEs;

    VertexHandle prevVertex = verts.back();
    for (VertexHandle b : verts)
    {                                // b  0 -> N
        VertexHandle a = prevVertex; // a -1 -> N-1
        prevVertex = b;

        if (_edgeMap.contains(makeEdgeKey(a, b)))
        { // varsa listeye ekle
            HalfEdgeHandle he = _edgeMap[makeEdgeKey(a, b)];
            if (_halfEdges[he].face != InvalidHandle)
            {
                LOG_ERROR("EMesh::addFace : Invalid geometry.");
                assert(false);
            }

            _halfEdges[he].face = fh;
            innerHEs.push_back(he);
        }
        else
        { // yoksa oluşturup listeye ekle
            HalfEdgeHandle he = allocHalfEdge();
            _edgeMap[makeEdgeKey(a, b)] = he;

            _halfEdges[he].face = fh;
            _halfEdges[he].origin = a;
            innerHEs.push_back(he);
        }
        // aynı olayın tersini twinler için yap

        if (_edgeMap.contains(makeEdgeKey(b, a)))
        { // varsa listeye ekle
            HalfEdgeHandle he = _edgeMap[makeEdgeKey(b, a)];
            outerHEs.push_back(he);
        }
        else
        { // yoksa oluşturup listeye ekle
            HalfEdgeHandle he = allocHalfEdge();
            _edgeMap[makeEdgeKey(b, a)] = he;

            _halfEdges[he].face = InvalidHandle;
            _halfEdges[he].origin = b;
            outerHEs.push_back(he);
        }
    }

    // Twin eşleşmesi
    for (int a = 0, b = 0; a < innerHEs.size(); a++, b++)
    {
        HalfEdgeHandle he = innerHEs[a];
        HalfEdgeHandle ht = outerHEs[b];
        _halfEdges[he].twin = ht;
        _halfEdges[ht].twin = he;
    }

    // Outer halfedges prev-next eşleşmesi
    HalfEdgeHandle prevHE = innerHEs.back();
    for (HalfEdgeHandle he_next : innerHEs)
    {                               // b  0 -> N
        HalfEdgeHandle he = prevHE; // a -1 -> N-1
        prevHE = he_next;

        HalfEdgeHandle a = InvalidHandle;
        HalfEdgeHandle b = InvalidHandle;

        HalfEdgeHandle he_twin = twin(he);
        HalfEdgeHandle he_next_twin = twin(he_next);

        // invalid -> invalid
        if (face(he_twin) == InvalidHandle && face(he_next_twin) == InvalidHandle)
        {
            a = he_next_twin;
            b = he_twin;
        }
        // invalid -> valid
        else if (face(he_twin) == InvalidHandle && face(he_next_twin) != InvalidHandle)
        {
            a = prev(he_next);
            b = he_twin;
        }
        // valid -> invalid
        else if (face(he_twin) != InvalidHandle && face(he_next_twin) == InvalidHandle)
        {
            a = twin(he_next);
            b = next(he);
        }
        // valid -> valid için bir şey yapmamız gerek yok.
        else
            continue;

        if (false)
        {
            LOG_TRACE("A: {} \t B: {}", a, b);
        }

        // outer he's arası bağlantı yapılır:
        _halfEdges[a].next = b;
        _halfEdges[b].prev = a;
    }

    // Inner halfedges prev-next eşleşmesi
    prevHE = innerHEs.back();
    for (HalfEdgeHandle he_next : innerHEs)
    {                               // b  0 -> N
        HalfEdgeHandle he = prevHE; // a -1 -> N-1
        prevHE = he_next;

        _halfEdges[he].next = he_next;
        _halfEdges[he_next].prev = he;
    }

    // Vertexlerin outer'ları eşleştirme
    for (int i = 0; i < verts.size(); i++)
    {
        VertexHandle vertex = verts[i];
        HalfEdgeHandle he = innerHEs[(i + 1) % innerHEs.size()];
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
VertexHandle EMesh::splitEdge(const HalfEdgeHandle &h1)
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
    v4 = addVertex((v1_pos + v2_pos) / 2.f);

    // Yeni HalfEdge'leri ekliyoruz.
    hn1 = addHalfEdge(v4, v2);
    hn2 = addHalfEdge(v2, v4);
    EHalfEdge &hn1e = _halfEdges[hn1];
    EHalfEdge &hn2e = _halfEdges[hn2];

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
    edgeMapDel(v1, v2);
    edgeMapDel(v2, v1);
    edgeMapAdd(v1, v4, h1);
    edgeMapAdd(v4, v1, h4);

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
    if (isInvalid(h1) || isInvalid(twin(h1)))
    {
        LOG_ERROR("[EMesh::flipEdge][{}] The halfedge or its twin is invalid. Operation aborted.", h1);
        return;
    }
    if (isInvalid(face(h1)) || isInvalid(face(twin(h1))))
    {
        LOG_ERROR("[EMesh::flipEdge][{}] Cannot flip a boundary edge. Operation aborted.", h1);
        return;
    }

    int h1LoopEdgeCount = countFaceEdges(h1);
    int h4LoopEdgeCount = countFaceEdges(twin(h1));
    if (h1LoopEdgeCount != 3 || h4LoopEdgeCount != 3)
    {
        LOG_ERROR("[EMesh::flipEdge][{}] The edge is not shared by two triangular faces. Operation aborted.\
            \n h1LoopEdgeCount:{}\t h4LoopEdgeCount:{}",
                  h1, h1LoopEdgeCount, h4LoopEdgeCount);
        return;
    }
    VertexHandle flipV0 = origin(prev(h1));       // V0
    VertexHandle flipV1 = origin(prev(twin(h1))); // V3
    if (hasEdge(flipV0, flipV1))
    {
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
    edgeMapDel(v1, v2);
    edgeMapDel(v2, v1);
    edgeMapAdd(v0, v3, h1);
    edgeMapAdd(v3, v0, h4);
}

void EMesh::collapseEdge(const HalfEdgeHandle &h1)
{
    // Uygunluk kontrolü
    std::vector<VertexHandle> v1Adj = adjacentVertices(origin(h1));
    std::vector<VertexHandle> v2Adj = adjacentVertices(destination(h1));

    int commonVertices = intersectHandles(v1Adj, v2Adj).size();

    if (commonVertices > 2)
    {
        LOG_ERROR("[EMesh::collapseEdge][{}] Adjacent vertices cannot be greater than 2. Operation aborted!", h1);
        return;
    }

    if (commonVertices != 2)
    {
        LOG_WARNING("[EMesh::collapseEdge][{}] Not implemented for 0 or 1 adjacent vertices. Operation aborted!", h1);
        return;
    }

    int f0EdgeCount = verticesOfFace(face(h1)).size();
    int f1EdgeCount = verticesOfFace(face(twin(h1))).size();
    if (f0EdgeCount != 3 || f1EdgeCount != 3)
    {
        LOG_WARNING("[EMesh::collapseEdge][{}] Not implemented for non-tris faces. Operation aborted!", h1);
        return;
    }

    // Gerekli handle'ların oluşturulması ve curent state'in kaydedilmesi
    HalfEdgeHandle h0, h2, h4, h5, h6, h7, h8, h9, h10;
    VertexHandle v0, v1, v2, v3;
    FaceHandle f0, f1;
    std::vector<HalfEdgeHandle> v1Outs, hOuts;

    h0 = prev(h1);
    h2 = next(h1);

    h4 = twin(h1);
    h5 = next(h4);
    h6 = prev(h4);

    h7 = twin(h0);
    h8 = twin(h2);

    h9 = twin(h5);
    h10 = twin(h6);

    v0 = origin(h0);
    v1 = origin(h1);
    v2 = origin(h2);
    v3 = origin(h6);

    f0 = face(h1);
    f1 = face(h4);

    v1Outs = outgoingHalfEdges(v1);
    hOuts = differenceHandles(v1Outs, {h1}); // h1 hariç v1 outgoing half edges

    // Yeni bağlantıların yapılması

    _halfEdges[h7].twin = h8;
    _halfEdges[h8].twin = h7;
    _halfEdges[h9].twin = h10;
    _halfEdges[h10].twin = h9;

    for (HalfEdgeHandle h : hOuts)
        _halfEdges[h].origin = v2;

    _vertices[v0].edge = h8;
    _vertices[v2].edge = h10;
    _vertices[v3].edge = h9;

    // Çöp dataların silinmesi

    for (auto h : {h0, h1, h2, h4, h5, h6})
        freeHalfEdge(h);

    freeVertex(v1);

    freeFace(f0);
    freeFace(f1);

    // edgeMap güncellemesi

    for (HalfEdgeHandle hOut : hOuts)
    {
        VertexHandle vDest = destination(hOut);

        edgeMapDel(v1, vDest);
        edgeMapDel(vDest, v1);

        edgeMapAdd(v2, vDest, hOut);
        edgeMapAdd(vDest, v2, twin(hOut));
    }

    edgeMapDel(v1, v2);
    edgeMapDel(v2, v1);
}

Mesh EMesh::construct()
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // 🔹 Flat shading: her yüzey için ayrı vertex oluştur
    for (const EFace &fh : _faces)
    {
        if(!isValidFace(fh)) continue;

        std::vector<Vertex> localVertices;

        HalfEdgeHandle heIter = fh.edge;
        do
        {
            glm::vec3 pos = _vertices[_halfEdges[heIter].origin].point;
            localVertices.push_back(Vertex{
                pos,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec2(0.0f, 0.0f)});

            heIter = next(heIter);
        } while (heIter != fh.edge);

        const int edgeCount = localVertices.size();
        if (edgeCount == 3) // tris
        {
            Vertex &A = localVertices[0];
            Vertex &B = localVertices[1];
            Vertex &C = localVertices[2];

            unsigned int vertexIndex = vertices.size();  
            indices.insert(indices.end(), {
                vertexIndex + 0,
                vertexIndex + 1,
                vertexIndex + 2,
            });
              
            glm::vec3 n = trisNormal(A.position, B.position, C.position);
            // n could be zero add warning!
            A.normal = n; B.normal = n; C.normal = n;

            vertices.push_back(A);
            vertices.push_back(B);
            vertices.push_back(C);

        }
        else if (edgeCount == 4) // quad
        {

            Vertex &A = localVertices[0];
            Vertex &B = localVertices[1];
            Vertex &C = localVertices[2];
            Vertex &D = localVertices[3];

            unsigned int vertexIndex = vertices.size();  
            indices.insert(indices.end(), {
                vertexIndex + 0,
                vertexIndex + 1,
                vertexIndex + 2,
                vertexIndex + 3,
                vertexIndex + 4,
                vertexIndex + 5,
            });
            

            bool b = useDiagonalAC(A.position, B.position, C.position, D.position);
            // A -- D
            // | \  |
            // |  \ |
            // B -- C
            if(b) // Use AC as diagonal 
            {                    
                // ABC and ACD
                glm::vec3 nABC = trisNormal(A.position, B.position, C.position);
                glm::vec3 nACD = trisNormal(A.position, C.position, D.position);
                // n could be zero add warning!

                A.normal = nABC; B.normal = nABC; C.normal = nABC;
                vertices.push_back(A);
                vertices.push_back(B);
                vertices.push_back(C);

                A.normal = nACD; C.normal = nACD; D.normal = nACD;
                vertices.push_back(A);
                vertices.push_back(C);
                vertices.push_back(D);        
            }
            // A -- D
            // |  / |
            // | /  |
            // B -- C
            else // use BD as diagonal
            {
                // ABD and BCD
                glm::vec3 nABD = trisNormal(A.position, B.position, D.position);
                glm::vec3 nBCD = trisNormal(B.position, C.position, D.position);
                // n could be zero add warning!

                A.normal = nABD; B.normal = nABD; D.normal = nABD;
                vertices.push_back(A);
                vertices.push_back(B);
                vertices.push_back(D);

                B.normal = nBCD; C.normal = nBCD; D.normal = nBCD;
                vertices.push_back(B);
                vertices.push_back(C);
                vertices.push_back(D);

            }
        }
    }

    Mesh m;
    m.init(vertices, indices);
    m.upload2GPU();
    return m;
}

// Validate whole mesh
void EMesh::validate()
{
    static const char *vPrefix = "[EMesh::validate]";
    const int MAX_ITER = 100;
    // 1. Twin of the twin of a halfedge must be equal to the halfedge

    for (HalfEdgeHandle i = 0; i < _halfEdges.size(); i++)
    {
        const HalfEdgeHandle he = i;
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

        if (he_twin != InvalidHandle && (he_twin < 0 || he_twin >= allHalfedgesCount))
            LOG_ERROR("{} [{}] he.twin is out of bounds", vPrefix, he_twin);
        if (he_next != InvalidHandle && (he_next < 0 || he_next >= allHalfedgesCount))
            LOG_ERROR("{} [{}] he.next is out of bounds", vPrefix, he_next);
        if (he_prev != InvalidHandle && (he_prev < 0 || he_prev >= allHalfedgesCount))
            LOG_ERROR("{} [{}] he.prev is out of bounds", vPrefix, he_prev);

        if (he_origin != InvalidHandle && (he_origin < 0 || he_origin >= allVerticesCount))
            LOG_ERROR("{} [{}] he.origin is out of bounds", vPrefix, he_origin);

        if (he_face != InvalidHandle && (he_face < 0 || he_face >= allFaceCount))
            LOG_ERROR("{} [{}] he.face is out of bounds", vPrefix, he_face);

        if (twin(twin(he)) != he)
            LOG_ERROR("{} [{}] Twin of the twin of a halfedge must be equal to the halfedge", vPrefix, he);

        if (origin(he) != destination(twin(he)))
            LOG_ERROR("{} [{}] he.orig != he.twin.destination", vPrefix, he);

        if (prev(next(he)) != he)
            LOG_ERROR("{} [{}] he.next.prev != he", vPrefix, he);

        if (next(prev(he)) != he)
            LOG_ERROR("{} [{}] he.prev.next != he", vPrefix, he);

        // -----<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>-----

        h = he;
        counter = 0;
        FaceHandle f1 = face(he);
        do
        {
            if (counter++ > MAX_ITER)
            {
                LOG_ERROR("{} [{}] edge loop does not close or too big!", vPrefix, he);
                break;
            }
            if (f1 != face(h))
            {
                LOG_ERROR("{} [{}] face of inner loop edges does not equal!", vPrefix, he);
                break;
            }
            h = next(h);
        } while (h != he);

        // -----<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>-----

        HalfEdgeHandle he_face_edge = edgeofFace(face(he));
        if (he_face_edge != InvalidHandle)
        { // boundary he değilse
            h = he;
            counter = 0;
            do
            {
                if (counter++ > MAX_ITER)
                {
                    LOG_ERROR("{} [{}] he.face.edge is not inside edge loop!", vPrefix, he);
                    break;
                }
                h = next(h);
            } while (h != he_face_edge);
        }

        // -----<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>---<X>---<O>-----

        HalfEdgeHandle he_origin_edge = edgeofVertex(origin(he));
        if (he_origin_edge != InvalidHandle)
        {
            h = he;
            counter = 0;
            do
            {
                // LOG_WARNING("counter: {}, h: {}, he: {}, he.orig.edge: {} ", counter, h, he, he_origin_edge);
                if (counter++ > MAX_ITER)
                {
                    LOG_ERROR("{} [{}] he.origin.edge is not inside outgoing halfedges!", vPrefix, he);
                    break;
                }
                h = next(twin(h));
            } while (h != he_origin_edge);
        }
        else
        {
            LOG_ERROR("{} [{}] he.origin.edge is Invalid!", vPrefix, he);
        }
    }
}