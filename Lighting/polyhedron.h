#ifndef POLYHEDRON_H
#define POLYHEDRON_H

#include <QVector>
#include <QVector3D>
#include <QVector4D>
#include <QColor>
#include <cmath>

struct Polygon;

struct Vertex
{
    QVector<Polygon*> polygons;
    QVector4D point_local;
    QVector4D point_world;
    Vertex() = default;
    Vertex(double x, double y, double z) : point_local(x, y, z, 1) { }
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
    void create_normal() {
        normal_local = QVector3D::normal(
            vertices[0]->point_local.toVector3D(),
            vertices[1]->point_local.toVector3D(),
            vertices[2]->point_local.toVector3D()
        ) * 3;
    }
};

struct Polyhedron
{
    QVector<Vertex> vertices;
    QVector<Polygon> polygons;

    static Polyhedron GenerateCube();
    static Polyhedron GeneratePyramid();
    static Polyhedron GenerateConeMesh(double r1, double r2, double h, double k, int h_apr, int v_apr);
};

inline Polyhedron Polyhedron::GenerateCube()
{
    const int L = 50;
    Polyhedron cube;
    for (double x : {-L, L})
        for (double y : {-L, L})
            for (double z : {-L, L})
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
    for (double x : {-L, L})
        for (double z : {-L, L})
                pyramid.vertices.push_back({x, 0, z});
    pyramid.vertices.push_back({0, -3*L, 0});
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

inline Polyhedron Polyhedron::GenerateConeMesh(double r1, double r2, double h, double base_ratio, int h_appr, int v_appr)
{
    Polyhedron cone;
    const double step = 2.0 * acos(-1) / h_appr;
    for (int i = 0; i <= v_appr; i++) {
        double lvl_ratio = 1.0 - (1.0 - base_ratio) * i / v_appr;
        double t = 0;
        for (int j = 0; j < h_appr; j++, t += step) {
            cone.vertices.push_back({ lvl_ratio * r1 * cos(t),
                                      -(h * i / v_appr - h / 2),
                                      lvl_ratio * r2 * sin(t)});
        }
    }
    cone.vertices.push_back({ 0,  h / 2, 0 });
    cone.vertices.push_back({ 0, -h / 2, 0 });
    for (int i = 1; i <= v_appr; i++) {
        for (int j = 0; j < h_appr; j++) {
            cone.polygons.push_back({ { &cone.vertices[   i  * h_appr +  j         ]        //  1
                                      , &cone.vertices[(i-1) * h_appr +  j         ]        //  |\.
                                      , &cone.vertices[(i-1) * h_appr + (j+1)%h_appr] } });  //  2-3
            cone.polygons.back().create_normal();
            cone.polygons.back().color = rand();
            cone.vertices[   i  * h_appr + j          ].polygons.push_back(&cone.polygons.back());
            cone.vertices[(i-1) * h_appr + j          ].polygons.push_back(&cone.polygons.back());
            cone.vertices[(i-1) * h_appr + (j+1)%h_appr].polygons.push_back(&cone.polygons.back());
            cone.polygons.push_back({ { &cone.vertices[(i-1) * h_appr + (j+1)%h_appr]        //  3-2
                                      , &cone.vertices[   i  * h_appr + (j+1)%h_appr]        //   \|
                                      , &cone.vertices[   i  * h_appr +  j         ] } });  //    1
            cone.polygons.back().create_normal();
            cone.polygons.back().color = rand();
            cone.vertices[(i-1) * h_appr + (j+1)%h_appr].polygons.push_back(&cone.polygons.back());
            cone.vertices[   i  * h_appr + (j+1)%h_appr].polygons.push_back(&cone.polygons.back());
            cone.vertices[   i  * h_appr +  j         ].polygons.push_back(&cone.polygons.back());
        }
    }
    for (int i = 0; i < h_appr; i++) {
        cone.polygons.push_back({ { &cone.vertices[cone.vertices.size()-2]
                                  , &cone.vertices[      (i+1)%h_appr     ]
                                  , &cone.vertices[          i           ] } });
        cone.polygons.back().create_normal();
        cone.polygons.back().color = rand();
        cone.vertices[cone.vertices.size()-2].polygons.push_back(&cone.polygons.back());
        cone.vertices[      (i+1)%h_appr     ].polygons.push_back(&cone.polygons.back());
        cone.vertices[          i           ].polygons.push_back(&cone.polygons.back());
    }
    for (int i = 0; i < h_appr; i++) {
        cone.polygons.push_back({ { &cone.vertices.back()
                                  , &cone.vertices[h_appr * v_appr +  i         ]
                                  , &cone.vertices[h_appr * v_appr + (i+1)%h_appr] } });
        cone.polygons.back().create_normal();
        cone.polygons.back().color = rand();
        cone.vertices.back()                      .polygons.push_back(&cone.polygons.back());
        cone.vertices[h_appr * v_appr +  i         ].polygons.push_back(&cone.polygons.back());
        cone.vertices[h_appr * v_appr + (i+1)%h_appr].polygons.push_back(&cone.polygons.back());
    }
    return cone;
}

#endif // POLYHEDRON_H
