//
// Created by xuhongxu on 16/5/2.
//

#ifndef OPENGL_HALFEDGE_H
#define OPENGL_HALFEDGE_H
#include <vector>
#include <map>
#include <glm/glm.hpp>


class Edge;
class Vert;
class Face;
class HalfEdge;
class Mesh;

class Edge {
    HalfEdge *halfEdges[2] = {nullptr, nullptr};

    friend class Face;
    friend class Mesh;
};

class Vert {
    HalfEdge* halfEdge = nullptr;
    glm::vec3 norm;

    friend class HalfEdge;
    friend class Mesh;
public:
    float x, y, z;
    HalfEdge* getHalfEdge() const {return halfEdge;}
    glm::vec3 getNormal() const {return norm;}
};

class Face {
    Mesh *mesh = nullptr;
    HalfEdge *halfEdge = nullptr;
    glm::vec3 norm;

    void createEdge(HalfEdge *he, bool copy = false);
    void calcNormal();
public:
    Face(Mesh *m, Vert *a, Vert *b, Vert *c, bool copy = false);
    glm::vec3 getNorm() const {return norm;}
    HalfEdge *getHalfEdge() const {return halfEdge;}
};

class HalfEdge {
    HalfEdge *prev = nullptr;
    HalfEdge *next = nullptr;
    Vert *source = nullptr;
    Vert *target = nullptr;
    HalfEdge *twin = nullptr;
    Edge *edge = nullptr;
    Face *face = nullptr;

    friend class Face;
    friend class Mesh;
public:
    void setVert(Vert *s, Vert *t);
    Face* getFace() const {return face;}
    Vert* getSource() const {return source;}
    Vert* getTarget() const {return target;}
    HalfEdge* getNext() const {return next;}

};

class Mesh {
    std::vector<Vert*> vertices;
    std::vector<HalfEdge*> halfEdges;
    std::vector<Edge*> edges;
    std::vector<Face*> faces;
    std::vector<HalfEdge*> _halfEdges;
    std::vector<Edge*> _edges;
    std::vector<Face*> _faces;
    std::map<std::pair<Vert*, Vert*>, Edge*> edgeMap;
    std::map<Edge*, Vert*> edgeDivide;

    void calcVertexNormal();
    Vert* subdivideEdge(Edge *, float w = 0);
    void subdivideEdgeWithExtraordinaryVertex(int valency, HalfEdge *h3, Vert *newVert);

    friend class Face;
    friend class Butterfly;
    const double PI = 3.1415926535;
public:
    Mesh() {};
    Mesh(const char *fileName);
    Edge *findEdge(Vert *s, Vert *t);

    std::vector<Vert*> getVertices() const {return vertices;}
    std::vector<HalfEdge*> getHalfEdges() const {return halfEdges;}
    std::vector<Edge*> getEdges() const {return edges;}
    std::vector<Face*> getFaces() const {return faces;}

    void butterfly(bool debug = false, float w = 0);

    ~Mesh();
};


#endif //OPENGL_HALFEDGE_H
