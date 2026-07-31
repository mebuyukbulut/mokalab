#pragma once 
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Logger.h"
#include "Mesh.h"


using VertexHandle = int32_t;
using HalfEdgeHandle = int32_t;
using FaceHandle = int32_t;

constexpr int32_t InvalidHandle = -1;
constexpr int32_t TombstoneHandle = -404;

struct EHalfEdge;
struct EVertex{
    glm::vec3 point; 
    HalfEdgeHandle edge = InvalidHandle; // Vertex'den dışarıya doğru giden random bir edge 
};

struct EFace{
    HalfEdgeHandle edge = InvalidHandle; // Herhangibir boundary edge
};

struct EHalfEdge{
    HalfEdgeHandle prev = InvalidHandle;
    HalfEdgeHandle next = InvalidHandle;
    HalfEdgeHandle twin = InvalidHandle;
    VertexHandle origin = InvalidHandle;
    FaceHandle face     = InvalidHandle;
};

class EMesh{
    // ==========================
    // Storage
    // ==========================
    std::vector<EVertex> _vertices;
    std::vector<EHalfEdge> _halfEdges;
    std::vector<EFace> _faces;

    std::vector<VertexHandle> _freeVertices;
    std::vector<HalfEdgeHandle> _freeHalfEdges;
    std::vector<FaceHandle> _freeFaces;

    std::vector<glm::vec3> _faceNormals;
    std::unordered_map<uint64_t, HalfEdgeHandle> _edgeMap;


    // ==========================
    // Allocation
    // ==========================

    VertexHandle allocVertex();
    HalfEdgeHandle allocHalfEdge();
    FaceHandle allocFace();

    void freeVertex(VertexHandle v);
    void freeHalfEdge(HalfEdgeHandle h);
    void freeFace(FaceHandle f);

    // ==========================
    // Edge Map
    // ==========================

    inline uint64_t makeEdgeKey(int32_t a, int32_t b);
    void edgeMapAdd(VertexHandle a, VertexHandle b, HalfEdgeHandle he);
    void edgeMapDel(VertexHandle a, VertexHandle b);
    inline bool hasEdge(VertexHandle a, VertexHandle b);


        
    HalfEdgeHandle addHalfEdge(VertexHandle a, VertexHandle b);
    HalfEdgeHandle addHalfEdge(VertexHandle a, VertexHandle b, const EHalfEdge& he);


    int countFaceEdges(HalfEdgeHandle he);


    // ==========================
    // Utility
    // ==========================

    inline HalfEdgeHandle prev(HalfEdgeHandle he){return _halfEdges[he].prev;}
    inline HalfEdgeHandle next(HalfEdgeHandle he){return _halfEdges[he].next;}
    inline HalfEdgeHandle twin(HalfEdgeHandle he){return _halfEdges[he].twin;}
    inline VertexHandle origin(HalfEdgeHandle he){return _halfEdges[he].origin;}
    inline FaceHandle face(HalfEdgeHandle he){return _halfEdges[he].face;}
    inline VertexHandle destination(HalfEdgeHandle he){return origin(next(he));}

    inline HalfEdgeHandle edgeofFace(FaceHandle f){return f == InvalidHandle ? InvalidHandle : _faces[f].edge;}
    inline HalfEdgeHandle edgeofVertex(VertexHandle v){return v == InvalidHandle ? InvalidHandle : _vertices[v].edge;}


    std::vector<VertexHandle> intersectHandles(std::vector<int32_t> a, std::vector<int32_t> b);
    std::vector<VertexHandle> differenceHandles(std::vector<int32_t> a, std::vector<int32_t> b);


    // ==========================
    // Validation
    // ==========================
    inline bool isInvalid(const uint32_t handle);
    inline bool isValidVertex(const VertexHandle v);
    inline bool isValidFace(const FaceHandle f);
    inline bool isValidHalfEdge(const HalfEdgeHandle h);


    std::vector<HalfEdgeHandle> outgoingHalfEdges(VertexHandle v);
    std::vector<VertexHandle> adjacentVertices(VertexHandle v);
    std::vector<VertexHandle> verticesOfFace(FaceHandle f);


public:
    VertexHandle addVertex(const glm::vec3& pos);
    FaceHandle addFace(const std::vector<VertexHandle>& verts);

    VertexHandle splitEdge(const HalfEdgeHandle& h1);
    void flipEdge(const HalfEdgeHandle& h1);
    void collapseEdge(const HalfEdgeHandle& h1);

    void computeFaceNormal();

    Mesh construct();

    void validate();

};