#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include <QVector>
#include <QVector3D>
#include <QVector4D>
#include <QColor>

struct Polygon;

struct Vertex
{
    QVector<Polygon*> polygons;
    QVector4D point_local;
    QVector4D point_world;
    Vertex() = default;
    Vertex(int x, int y, int z) : point_local(x, y, z, 1) { }
};

struct Polygon
{
    QVector<Vertex*> vertices;
    QVector4D normal_local;
    QVector4D normal_world;
    QColor color;
    Polygon() = default;
    Polygon(QVector<Vertex*> v) : vertices(v) { }
    QVector4D mid() const {
        return std::accumulate(vertices.begin(), vertices.end(), QVector4D(),
            [](QVector4D s, Vertex* v){ return s += v->point_world;
                }) / vertices.size();
    }
};

struct Polyhedron
{
    QVector<Vertex> vertices;
    QVector<Polygon> polygons;

    static Polyhedron GenerateCube();
    static Polyhedron GeneratePyramid();
};

inline Polyhedron Polyhedron::GenerateCube()
{
    const int L = 50;
    Polyhedron cube;
    for (int x : {-L, L})
        for (int y : {-L, L})
            for (int z : {-L, L})
                cube.vertices.push_back({x, y, z});
    QVector<QVector<int> > planes = {
        { 0, 1, 3, 2 },
        { 0, 2, 6, 4 },
        { 0, 4, 5, 1 },
        { 1, 5, 7, 3 },
        { 2, 3, 7, 6 },
        { 4, 6, 7, 5 },
    };
    cube.polygons.resize(planes.size());
    for (int i = 0; i < planes.size(); i++) {
        for (int p : planes[i]) {
            cube.polygons[i].vertices.push_back(&cube.vertices[p]);
            cube.vertices[p].polygons.push_back(&cube.polygons[i]);
        }
        cube.polygons[i].normal_local = QVector3D::normal(
            cube.polygons[i].vertices[0]->point_local.toVector3D(),
            cube.polygons[i].vertices[1]->point_local.toVector3D(),
            cube.polygons[i].vertices[2]->point_local.toVector3D()
        ) * L * 0.3;
        cube.polygons[i].normal_local.setW(0);
        cube.polygons[i].color = rand();
    }
    return cube;
}

inline Polyhedron Polyhedron::GeneratePyramid()
{
    const int L = 50;
    Polyhedron pyramid;
    for (int x : {-L, L})
        for (int z : {-L, L})
                pyramid.vertices.push_back({x, 0, z});
    pyramid.vertices.push_back({0, -L, 0});
    QVector<QVector<int> > planes = {
        { 0, 1, 3, 2 },
        { 0, 2, 4 },
        { 0, 4, 1 },
        { 1, 4, 3 },
        { 2, 3, 4 }
    };
    pyramid.polygons.resize(planes.size());
    for (int i = 0; i < planes.size(); i++) {
        for (int p : planes[i]) {
            pyramid.polygons[i].vertices.push_back(&pyramid.vertices[p]);
            pyramid.vertices[p].polygons.push_back(&pyramid.polygons[i]);
        }
        pyramid.polygons[i].normal_local = QVector3D::normal(
            pyramid.polygons[i].vertices[0]->point_local.toVector3D(),
            pyramid.polygons[i].vertices[1]->point_local.toVector3D(),
            pyramid.polygons[i].vertices[2]->point_local.toVector3D()
        ) * L * 0.3;
        pyramid.polygons[i].normal_local.setW(0);
        pyramid.polygons[i].color = rand();
    }
    return pyramid;
}

#endif // POLYHEDRON_H
