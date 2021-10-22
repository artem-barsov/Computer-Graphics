#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include <QVector>
#include <QVector4D>

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
    Polygon() = default;
    Polygon(QVector<Vertex*> v) : vertices(v) { }
};

struct Polyhedron
{
    QVector<Vertex> vertices;
    QVector<Polygon> polygons;

    static Polyhedron GenerateCube() {
        Polyhedron cube;
        for (int x : {-50, 50})
            for (int y : {-50, 50})
                for (int z : {-50, 50})
                    cube.vertices.push_back({x, y, z});
        cube.polygons.resize(6);
        for (int p : {0, 1, 3, 2}) {
            cube.polygons[0].vertices.push_back(&cube.vertices[p]);
            cube.vertices[p].polygons.push_back(&cube.polygons[0]);
        }
        for (int p : {0, 1, 5, 4}) {
            cube.polygons[1].vertices.push_back(&cube.vertices[p]);
            cube.vertices[p].polygons.push_back(&cube.polygons[1]);
        }
        for (int p : {0, 2, 6, 4}) {
            cube.polygons[2].vertices.push_back(&cube.vertices[p]);
            cube.vertices[p].polygons.push_back(&cube.polygons[2]);
        }
        for (int p : {1, 3, 7, 5}) {
            cube.polygons[3].vertices.push_back(&cube.vertices[p]);
            cube.vertices[p].polygons.push_back(&cube.polygons[3]);
        }
        for (int p : {2, 3, 7, 6}) {
            cube.polygons[4].vertices.push_back(&cube.vertices[p]);
            cube.vertices[p].polygons.push_back(&cube.polygons[4]);
        }
        for (int p : {4, 5, 7, 6}) {
            cube.polygons[5].vertices.push_back(&cube.vertices[p]);
            cube.vertices[p].polygons.push_back(&cube.polygons[5]);
        }
        return cube;
    }
};

#endif // POLYHEDRON_H
