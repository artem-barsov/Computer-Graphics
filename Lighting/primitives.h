#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <QVector>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QColor>
#include <cmath>

struct Polygon;

struct Vertex
{
    QVector<std::shared_ptr<Polygon> > polygons;
    QVector4D point_local;
    QVector4D point_world;
    QVector4D normal_local;
    QVector4D normal_world;
    QVector3D ambient;
    QVector3D diffuse;
    QVector3D light;
    Vertex() = default;
    Vertex(double x, double y, double z) : point_local(x, y, z, 1) { }
    void create_normal();
    Vertex& operator+= (const Vertex& other) {
        this->point_world += other.point_world;
        this->light += other.light;
        return *this;
    }
    friend Vertex operator+ (Vertex lhs, const Vertex& rhs) {
        return lhs += rhs;
    }
    Vertex& operator/= (const double other) {
        this->point_world /= other;
        this->light /= other;
        return *this;
    }
    friend Vertex operator/ (Vertex lhs, int rhs) {
        return lhs /= rhs;
    }
};

struct Polygon
{
    QVector<std::shared_ptr<Vertex> > vertices;
    QVector4D normal_local;
    QVector4D normal_world;
    QColor color;
    Polygon() = default;
    Polygon(QVector<std::shared_ptr<Vertex> > v) : vertices(v) { }
    Vertex mid() const {
        return std::accumulate(vertices.begin(), vertices.end(), Vertex(),
            [](const Vertex& s, const std::shared_ptr<Vertex>& v){ return s + *v;
                }) / vertices.size();
    }
    void create_normal() {
        normal_local = QVector3D::normal(
            vertices[0]->point_local.toVector3D(),
            vertices[1]->point_local.toVector3D(),
            vertices[2]->point_local.toVector3D()
        );
    }
};

inline void Vertex::create_normal()
{
    normal_local = std::accumulate(polygons.begin(), polygons.end(), QVector4D(),
                   [](const QVector4D& n, const std::shared_ptr<Polygon>& p)
                   { return n + p->normal_local; } ) / polygons.size();
}

struct Lighter
{
    QVector4D pos_world;
    QMatrix4x4 rotate;
    int distance;
    QVector3D ambient;
    QVector3D intensity;
    double md, mk;
    Lighter() : distance(0), md(0), mk(1) {
        QMatrix4x4 E;
        E.rotate(-150, {0,1,0});
        E.rotate(  30, {1,0,0});
        rotate = E;
        distanceFunc = [this](double d) { return 1.0 / (md * d + mk); };
    }
    std::function<double(double)> distanceFunc;
};

class Polyhedron
{
protected:
    const int L = 50; // 2*L -- average local length of figure
public:
    QVector<std::shared_ptr<Vertex> > vertices;
    QVector<std::shared_ptr<Polygon> > polygons;
};

class Cube : public Polyhedron {
public:
    Cube() : Polyhedron() {
        for (double x : {-L, L})
            for (double y : {-L, L})
                for (double z : {-L, L})
                    vertices.push_back(std::make_shared<Vertex>(x, y, z));
        QVector<QVector<int> > planes = { { 0, 1, 3 },
                                          { 0, 3, 2 },
                                          { 0, 2, 6 },
                                          { 0, 6, 4 },
                                          { 0, 4, 5 },
                                          { 0, 5, 1 },
                                          { 1, 5, 7 },
                                          { 1, 7, 3 },
                                          { 2, 3, 7 },
                                          { 2, 7, 6 },
                                          { 4, 6, 7 },
                                          { 4, 7, 5 } };
        for (const auto& plane : planes) {
            polygons.push_back(std::make_shared<Polygon>(QVector{ vertices[plane[0]],
                                                                  vertices[plane[1]],
                                                                  vertices[plane[2]] }));
            vertices[plane[0]]->polygons.push_back(polygons.back());
            vertices[plane[1]]->polygons.push_back(polygons.back());
            vertices[plane[2]]->polygons.push_back(polygons.back());
            polygons.back()->create_normal();
            polygons.back()->color = rand();
        }
        for (auto& v : vertices) v->create_normal();
    }
};

class Pyramid : public Polyhedron {
public:
    Pyramid() : Polyhedron() {
        for (double x : {-L, L})
            for (double z : {-L, L})
                vertices.push_back(std::make_shared<Vertex>(x, 1.5*L, z));
        vertices.push_back(std::make_shared<Vertex>(0, -1.5*L, 0));
        QVector<QVector<int> > planes = { { 0, 1, 3 },
                                          { 0, 3, 2 },
                                          { 0, 2, 4 },
                                          { 0, 4, 1 },
                                          { 1, 4, 3 },
                                          { 2, 3, 4 } };
        for (const auto& plane : planes) {
            polygons.push_back(std::make_shared<Polygon>(QVector{ vertices[plane[0]],
                                                                  vertices[plane[1]],
                                                                  vertices[plane[2]] }));
            vertices[plane[0]]->polygons.push_back(polygons.back());
            vertices[plane[1]]->polygons.push_back(polygons.back());
            vertices[plane[2]]->polygons.push_back(polygons.back());
            polygons.back()->create_normal();
            polygons.back()->color = rand();
        }
        for (auto& v : vertices) v->create_normal();
    }
};

class ConeMesh : public Polyhedron {
public:
    ConeMesh(double r1, double r2, double h, double base_ratio,
             int h_appr, int v_appr) : Polyhedron() {
        const double step = 2.0 * acos(-1) / h_appr;
        for (int i = 0; i <= v_appr; i++) {
            double lvl_ratio = 1.0 - (1.0 - base_ratio) * i / v_appr;
            double t = 0;
            for (int j = 0; j < h_appr; j++, t += step) {
                vertices.push_back(std::make_shared<Vertex>(lvl_ratio * r1 * cos(t),
                                                            -(h * i / v_appr - h / 2),
                                                            lvl_ratio * r2 * sin(t)));
            }
        }
        vertices.push_back(std::make_shared<Vertex>(0,  h / 2, 0));
        vertices.push_back(std::make_shared<Vertex>(0, -h / 2, 0));
        for (int i = 1; i <= v_appr; i++) {
            for (int j = 0; j < h_appr; j++) {
                polygons.push_back(std::make_shared<Polygon>(QVector
                                           { vertices[ i    * h_appr + j           ]       //  1
                                           , vertices[(i-1) * h_appr + j           ]       //  |\.
                                           , vertices[(i-1) * h_appr + (j+1)%h_appr] }));  //  2-3
                polygons.back()->create_normal();
                polygons.back()->color = rand();
                vertices[   i  * h_appr + j           ]->polygons.push_back(polygons.back());
                vertices[(i-1) * h_appr + j           ]->polygons.push_back(polygons.back());
                vertices[(i-1) * h_appr + (j+1)%h_appr]->polygons.push_back(polygons.back());
                polygons.push_back(std::make_shared<Polygon>(QVector
                                           { vertices[(i-1) * h_appr + (j+1)%h_appr]       //  3-2
                                           , vertices[   i  * h_appr + (j+1)%h_appr]       //   \|
                                           , vertices[   i  * h_appr +  j          ] }));  //    1
                polygons.back()->create_normal();
                polygons.back()->color = rand();
                vertices[(i-1) * h_appr + (j+1)%h_appr]->polygons.push_back(polygons.back());
                vertices[   i  * h_appr + (j+1)%h_appr]->polygons.push_back(polygons.back());
                vertices[   i  * h_appr +  j          ]->polygons.push_back(polygons.back());
            }
        }
        for (int i = 0; i < h_appr; i++) {
            polygons.push_back(std::make_shared<Polygon>(QVector{ vertices[vertices.size()-2]
                                                                , vertices[   (i+1)%h_appr  ]
                                                                , vertices[        i        ] }));
            polygons.back()->create_normal();
            polygons.back()->color = rand();
            vertices[vertices.size()-2]->polygons.push_back(polygons.back());
            vertices[   (i+1)%h_appr  ]->polygons.push_back(polygons.back());
            vertices[        i        ]->polygons.push_back(polygons.back());
        }
        for (int i = 0; i < h_appr; i++) {
            polygons.push_back(std::make_shared<Polygon>(QVector
                                                    { vertices.back()
                                                    , vertices[h_appr * v_appr +  i          ]
                                                    , vertices[h_appr * v_appr + (i+1)%h_appr] }));
            polygons.back()->create_normal();
            polygons.back()->color = rand();
            vertices.back()                         ->polygons.push_back(polygons.back());
            vertices[h_appr * v_appr +  i          ]->polygons.push_back(polygons.back());
            vertices[h_appr * v_appr + (i+1)%h_appr]->polygons.push_back(polygons.back());
        }
        for (auto& v : vertices) v->create_normal();
    }
};

inline QMatrix4x4 scalingMtrx(double x, double y, double z)
{
    QMatrix4x4 E;
    E.scale(x, y, z);
    return E;
}

inline QMatrix4x4 shiftingMtrx(double x, double y, double z)
{
    QMatrix4x4 E;
    E.translate(x, y, z);
    return E;
}

#endif // PRIMITIVES_H
