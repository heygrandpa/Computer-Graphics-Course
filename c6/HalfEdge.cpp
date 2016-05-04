//
// Created by xuhongxu on 16/5/2.
//

#include "HalfEdge.h"
#include <iostream>

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
        // std::cout << buf<<i << std::endl;
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
        HalfEdge *he = v->getHalfEdge();
        if (!he) continue;
        HalfEdge *nextHe = he;
        glm::vec3 avgNorm(0.0f);
        do {
            glm::vec3 faceNorm = nextHe->face->getNorm();
            //std::cout << faceNorm.x << " " << faceNorm.y << " " << faceNorm.z << std::endl;
            avgNorm += faceNorm;
            nextHe = nextHe->next->twin;
        } while(nextHe && nextHe != he);
        //std::cout << "----" << std::endl;
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

Face::Face(Mesh *m, Vert *a, Vert *b, Vert *c) {
    mesh = m;

    HalfEdge *he0 = new HalfEdge;
    HalfEdge *he1 = new HalfEdge;
    HalfEdge *he2 = new HalfEdge;

    mesh->halfEdges.push_back(he0);
    mesh->halfEdges.push_back(he1);
    mesh->halfEdges.push_back(he2);

    glm::vec3 v1(a->x, a->y, a->z);
    glm::vec3 v2(b->x, b->y, b->z);
    glm::vec3 v3(c->x, c->y, c->z);
    norm = glm::normalize((v2 - v1) * (v3 - v1));

    he0->setVert(a, b);
    he0->face = this;
    he0->next = he1;
    he0->prev = he2;
    createEdge(he0);

    he1->setVert(b, c);
    he1->face = this;
    he1->next = he2;
    he1->prev = he0;
    createEdge(he1);

    he2->setVert(c, a);
    he2->face = this;
    he2->next = he0;
    he2->prev = he1;
    createEdge(he2);

    halfEdge = he0;
}

void Face::createEdge(HalfEdge *he) {
    Edge *edge;
    if ((edge = mesh->findEdge(he->source, he->target)) != nullptr) {
        edge->halfEdges[1] = he;
        he->twin = edge->halfEdges[0];
    } else {
        edge = new Edge;
        edge->halfEdges[0] = he;
        mesh->edges.push_back(edge);
    }
    he->edge = edge;
    mesh->edgeMap[std::make_pair(he->source, he->target)] = edge;
}

void HalfEdge::setVert(Vert *s, Vert *t) {
    source = s;
    target = t;
    s->halfEdge = this;
    t->halfEdge = this;
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