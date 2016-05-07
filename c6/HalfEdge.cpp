//
// Created by xuhongxu on 16/5/2.
//

#include "HalfEdge.h"
#include <iostream>
#define output_p(a) std::cout << a->x << " " << a->y << " " << a->z << std::endl;

Mesh::Mesh(const char *fileName) {

    FILE *file = nullptr;
    file = fopen(fileName, "r");
    if (!file) {
        throw "Failed to read OBJ file";
    }

    char buf[256];

    int i = 0;
    while (fscanf(file, "%s", buf) != EOF) {
        ++i;
        switch (buf[0]) {
            case '#':
                // comment
                fgets(buf, sizeof(buf), file);
                break;
            case 'v':
                switch (buf[1]) {
                    case '\0':
                        //vertex
                    {
                        Vert *v = new Vert;
                        fscanf(file, "%f %f %f", &v->x, &v->y, &v->z);
                        vertices.push_back(v);
                    }
                        break;
                    default:
                        // TODO: Other types of vertex
                        break;
                }
                break;
            case 'f':
                // face
            {
                int a, b, c;
                fscanf(file, "%d %d %d", &a, &b, &c);
                --a; --b; --c;
                Face *face = new Face(this, vertices[a], vertices[b], vertices[c]);
                faces.push_back(face);
            }
                break;
            default:
                // TODO: Other commands
                fgets(buf, sizeof(buf), file);
                break;
        }
    }
    calcVertexNormal();
}

void Mesh::calcVertexNormal() {
    for (auto it = vertices.begin(); it != vertices.end(); ++it) {
        Vert *v = *it;
        HalfEdge *he0 = v->getHalfEdge();
        HalfEdge *he = he0;
        if (!he) continue;
        HalfEdge *nextHe = he;
        glm::vec3 avgNorm(0.0f);

        do {
            he = nextHe;
            nextHe = nextHe->prev->twin;
        } while(nextHe && nextHe != he0);
        nextHe = he;

        do {
            glm::vec3 faceNorm = nextHe->face->getNorm();
            avgNorm += faceNorm;
            nextHe = nextHe->twin;
        } while(nextHe && (nextHe = nextHe->next) != he);
        v->norm = glm::normalize(avgNorm);
    }
}

Edge *Mesh::findEdge(Vert *s, Vert *t) {
    if (edgeMap.count(std::make_pair(s, t))) {
        return edgeMap[std::make_pair(s, t)];
    } else if (edgeMap.count(std::make_pair(t, s))) {
        return edgeMap[std::make_pair(t, s)];
    }
    return nullptr;
}

void Face::calcNormal() {
    Vert *a = halfEdge->getSource();
    Vert *b = halfEdge->getTarget();
    Vert *c = halfEdge->next->getTarget();
    glm::vec3 v1(a->x, a->y, a->z);
    glm::vec3 v2(b->x, b->y, b->z);
    glm::vec3 v3(c->x, c->y, c->z);
    norm = glm::normalize(glm::cross(v2 - v1, v3 - v1));
}

Face::Face(Mesh *m, Vert *a, Vert *b, Vert *c, bool copy) {
    mesh = m;

    HalfEdge *he0 = new HalfEdge;
    HalfEdge *he1 = new HalfEdge;
    HalfEdge *he2 = new HalfEdge;

    if (copy) {
        mesh->_halfEdges.push_back(he0);
        mesh->_halfEdges.push_back(he1);
        mesh->_halfEdges.push_back(he2);
    } else {
        mesh->halfEdges.push_back(he0);
        mesh->halfEdges.push_back(he1);
        mesh->halfEdges.push_back(he2);
    }

    he0->setVert(a, b);
    he0->face = this;
    he0->next = he1;
    he0->prev = he2;
    createEdge(he0, copy);

    he1->setVert(b, c);
    he1->face = this;
    he1->next = he2;
    he1->prev = he0;
    createEdge(he1, copy);

    he2->setVert(c, a);
    he2->face = this;
    he2->next = he0;
    he2->prev = he1;
    createEdge(he2, copy);

    halfEdge = he0;

    calcNormal();
}

void Face::createEdge(HalfEdge *he, bool copy) {
    Edge *edge;
    if ((edge = mesh->findEdge(he->source, he->target)) != nullptr) {
        edge->halfEdges[1] = he;
        edge->halfEdges[0]->twin = he;
        he->twin = edge->halfEdges[0];
    } else {
        edge = new Edge;
        edge->halfEdges[0] = he;
        if (copy) {
            mesh->_edges.push_back(edge);
        } else {
            mesh->edges.push_back(edge);
        }
    }
    he->edge = edge;
    mesh->edgeMap[std::make_pair(he->source, he->target)] = edge;
}

void HalfEdge::setVert(Vert *s, Vert *t) {
    source = s;
    target = t;
    s->halfEdge = this;
}

Mesh::~Mesh() {
    for (auto it = vertices.begin(); it != vertices.end(); ++it)
        delete *it;
    for (auto it = halfEdges.begin(); it != halfEdges.end(); ++it)
        delete *it;
    for (auto it = edges.begin(); it != edges.end(); ++it)
        delete *it;
    for (auto it = faces.begin(); it != faces.end(); ++it)
        delete *it;
}

void Mesh::subdivideEdgeWithExtraordinaryVertex(int valency, HalfEdge *he3, Vert *newVert) {
    float fs = 3.0f / 4;
    Vert *s = he3->source;
    Vert *t = he3->target;

    HalfEdge *nextHe = he3;
    int n = 0;
    do {
        ++n;
        nextHe = nextHe->twin;
    } while(nextHe && (nextHe = nextHe->next) != he3);
    nextHe = he3;
    if (n == 3) {
        Vert *e1, *e2;
        float ft = 5.0f / 12;
        float fe = -1.0f / 12;
        if (he3->twin)
            e1 = he3->twin->next->target;
        else
            return;
        if (he3->twin && he3->twin->next->twin)
            e2 = he3->twin->next->twin->next->target;
        else
            return;
#define butterfly_trans2(x)  newVert->x = fs * s->x + \
                             ft * t->x + \
                             fe * (e1->x + e2->x);
        butterfly_trans2(x);
        butterfly_trans2(y);
        butterfly_trans2(z);
#undef butterfly_trans2
    } else if (n == 4) {
        Vert *e2;
        float ft = 3.0f / 8;
        float fe2 = -1.0f / 8;
        if (he3->twin && he3->twin->next->twin)
            e2 = he3->twin->next->twin->next->target;
        else
            return;
#define butterfly_trans3(x)  newVert->x = fs * s->x + \
                             ft * t->x + \
                             fe2 * e2->x;
        butterfly_trans3(x);
        butterfly_trans3(y);
        butterfly_trans3(z);
#undef butterfly_trans3
    } else if (n >= 5) {
        Vert *ev;
        bool goNext = (he3->twin && he3->twin->next->twin);
        if (!goNext) return;
        int j = 0;
        newVert->x = fs * s->x;
        newVert->y = fs * s->y;
        newVert->z = fs * s->z;
#define butterfly_trans4(x)  newVert->x += fe * ev->x;
        do {
            ev = nextHe->target;
            double fe = (1.0 / 4 + cos(2 * PI * j / n) + 1.0 / 2 * cos(4 * PI * j / n)) / n;
            butterfly_trans4(x);
            butterfly_trans4(y);
            butterfly_trans4(z);
            ++j;
            nextHe = nextHe->twin;
        } while(nextHe && (nextHe = nextHe->next) != he3);
#undef butterfly_trans4
    }
}

Vert* Mesh::subdivideEdge(Edge *e, float w) {
    if (edgeDivide.count(e)) {
        return edgeDivide[e];
    }

    HalfEdge *he = e->halfEdges[0];
    Vert *s = he->source;
    Vert *t = he->target;
    Vert *newVert = new Vert;

    newVert->x = (s->x + t->x) / 2;
    newVert->y = (s->y + t->y) / 2;
    newVert->z = (s->z + t->z) / 2;

    int sn = 0;
    int tn = 0;
    {
        HalfEdge *nextHe = he;
        do {
            ++sn;
            nextHe = nextHe->twin;
        } while (nextHe && (nextHe = nextHe->next) != he);
        nextHe = he->twin;
        if (nextHe) {
            do {
                ++tn;
                nextHe = nextHe->twin;
            } while (nextHe && (nextHe = nextHe->next) != he->twin);
        }
    }

    if (HalfEdge *he2 = e->halfEdges[1]) {
        if (sn == 6 && tn == 6) {
            // 1. The edge connects two vertices of valence 6;
            float fa = 1.0f / 2 - w;
            float fb = 1.0f / 8 + 2 * w;
            float fc = -1.0f / 16 - w;
            float fd = w;
            Vert *b1, *b2, *c1, *c2, *c3, *c4, *d1, *d2;
            b1 = he->next->target;
            b2 = he2->next->target;
            if (HalfEdge *t1 = he->next->twin)
                c1 = t1->next->target;
            else
                c1 = he2->prev->twin->prev->twin->prev->twin->prev->source;
            if (HalfEdge *t1 = he2->next->twin)
                c2 = t1->next->target;
            else
                c2 = he->prev->twin->prev->twin->prev->twin->prev->source;
            if (HalfEdge *t1 = he->prev->twin)
                c3 = t1->next->target;
            else
                c3 = he2->next->twin->next->twin->next->twin->next->target;
            if (HalfEdge *t1 = he2->prev->twin)
                c4 = t1->next->target;
            else
                c4 = he->next->twin->next->twin->next->twin->next->target;

            if (he->next->twin && he->next->twin->next->twin) {
                d1 = he->next->twin->next->twin->next->target;
            } else {
                d1 = he2->prev->twin->prev->twin->prev->source;
            }
            if (he2->next->twin && he2->next->twin->next->twin) {
                d2 = he2->next->twin->next->twin->next->target;
            } else {
                d2 = he->prev->twin->prev->twin->prev->source;
            }
#define butterfly_trans1(x)  newVert->x = fa * (s->x + t->x) + \
                             fb * (b1->x + b2->x) + \
                             fc * (c1->x + c2->x + c3->x + c4->x) + \
                             fd * (d1->x + d2->x);
            butterfly_trans1(x);
            butterfly_trans1(y);
            butterfly_trans1(z);
#undef butterfly_trans1
        } else if (sn == 6 || tn == 6) {
            // 2. The edge connects a K-vertex (K  = 6) and a 6-vertex;
            if (tn == 6) {
                subdivideEdgeWithExtraordinaryVertex(sn, he, newVert);
            } else {
                subdivideEdgeWithExtraordinaryVertex(tn, he2, newVert);
            }
        } else {
            // 3. The edge connects two extraordinary vertices;
            Vert v1 = *newVert, v2 = *newVert;
            subdivideEdgeWithExtraordinaryVertex(sn, he, &v1);
            subdivideEdgeWithExtraordinaryVertex(tn, he2, &v2);
            newVert->x = (v1.x + v2.x) / 2;
            newVert->y = (v1.y + v2.y) / 2;
            newVert->z = (v1.z + v2.z) / 2;
        }
    } else {
        // boundary edge
    }
    vertices.push_back(newVert);
    edgeDivide[e] = newVert;
    return newVert;
}

void Mesh::butterfly(bool debug, float w) {
    edgeMap.clear();
    edgeDivide.clear();
    for (auto it = faces.begin(); it != faces.end(); ++it) {
        Face *&f = *it;
        HalfEdge *he1 = f->getHalfEdge();
        HalfEdge *he2 = he1->next;
        HalfEdge *he3 = he2->next;
        Vert *a = he1->source;
        Vert *b = he1->target;
        Vert *c = he2->target;
        Vert *v1 = subdivideEdge(he1->edge, w);
        Vert *v2 = subdivideEdge(he2->edge, w);
        Vert *v3 = subdivideEdge(he3->edge, w);
        if (debug) {
            output_p(a)
            output_p(b)
            output_p(c)
            output_p(v1)
            output_p(v2)
            output_p(v3)
            std::cout << "--" << std::endl;
        }

        Face *f1 = new Face(this, a, v1, v3, true);
        Face *f2 = new Face(this, b, v2, v1, true);
        Face *f3 = new Face(this, c, v3, v2, true);
        Face *f4 = new Face(this, v1, v2, v3, true);
        _faces.push_back(f1);
        _faces.push_back(f2);
        _faces.push_back(f3);
        _faces.push_back(f4);
    }
    for (auto it = halfEdges.begin(); it != halfEdges.end(); ++it)
        delete *it;
    for (auto it = edges.begin(); it != edges.end(); ++it)
        delete *it;
    for (auto it = faces.begin(); it != faces.end(); ++it)
        delete *it;
    halfEdges = _halfEdges;
    edges = _edges;
    faces = _faces;
    _halfEdges.clear();
    _edges.clear();
    _faces.clear();

    calcVertexNormal();

}
