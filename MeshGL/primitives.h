#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <QVector>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QColor>
#include <cmath>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

struct VertexGL
{
//public:
    QVector3D position_local;
    QVector3D normal_local;
    QVector3D color; // TODO: 1 byte
};

struct Polygon;

struct Vertex
{
    QVector<std::shared_ptr<Polygon> > polygons;
    QVector4D point_local;
    QVector4D normal_local;
    Vertex() = default;
    Vertex(double x, double y, double z) : point_local(x, y, z, 1) { }
    void create_normal();
//    Vertex& operator+= (const Vertex& other) {
//        this->point_world += other.point_world;
//        return *this;
//    }
//    friend Vertex operator+ (Vertex lhs, const Vertex& rhs) {
//        return lhs += rhs;
//    }
//    Vertex& operator/= (const double other) {
//        this->point_world /= other;
//        return *this;
//    }
//    friend Vertex operator/ (Vertex lhs, int rhs) {
//        return lhs /= rhs;
//    }
};

struct Polygon
{
    QVector<std::shared_ptr<Vertex> > vertices;
    QVector4D normal_local;
    QVector4D normal_world;
    QColor color;
    Polygon() = default;
    Polygon(const QVector<std::shared_ptr<Vertex> >& vs) : vertices(vs) { }
//    Vertex mid() const {
//        return std::accumulate(vertices.begin(), vertices.end(), Vertex(),
//            [](const Vertex& s, const std::shared_ptr<Vertex>& v){ return s + *v;
//                }) / vertices.size();
//    }
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

class Polyhedron : protected QOpenGLFunctions
{
protected:
//    const int L = 50; // 2*L -- average local length of figure
    QOpenGLShaderProgram* shaders;
public:
    Polyhedron(QOpenGLShaderProgram& shaders) : shaders(&shaders) { initializeOpenGLFunctions(); }
    virtual ~Polyhedron() {
        shaders->disableAttributeArray("qt_Vertex");
        shaders->disableAttributeArray("qt_Normal");
        shaders->disableAttributeArray("qt_Color");
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }
    virtual void drawPolyhedron() = 0;
    QVector<std::shared_ptr<Vertex> > vertices;
    QVector<std::shared_ptr<Polygon> > polygons;
    QVector<VertexGL> verticesGL;
//    std::vector<VertexGL> verticesGL;
    QVector<GLuint> indices;
//    std::vector<GLuint> indices;
};

class Cube : public Polyhedron {
public:
    Cube(QOpenGLShaderProgram& shaders) : Polyhedron(shaders) {
//                       x   y   z    nx  ny  nz    r  g  b
        verticesGL = {{{ 1, -1,  1}, { 1, -1,  1}, {0, 0, 0} },
                     { {-1, -1,  1}, {-1, -1,  1}, {0, 0, 1} },
                     { { 1, -1, -1}, { 1, -1, -1}, {0, 1, 0} },
                     { {-1, -1, -1}, {-1, -1, -1}, {0, 1, 1} },
                     { { 1,  1,  1}, { 1,  1,  1}, {1, 0, 0} },
                     { {-1,  1,  1}, {-1,  1,  1}, {1, 0, 1} },
                     { {-1,  1, -1}, {-1,  1, -1}, {1, 1, 0} },
                     { { 1,  1, -1}, { 1,  1, -1}, {1, 1, 1}}};
        for (auto& [_,norm,__] : verticesGL) norm.normalize();
        indices = { 1, 0, 5, 4, 7, 0, 2, 1, 3, 5, 6, 7, 3, 2};

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(VertexGL), &verticesGL[0].position_local);
        glNormalPointer(GL_FLOAT, sizeof(VertexGL), &verticesGL[0].normal_local);
        glColorPointer(3, GL_FLOAT, sizeof(VertexGL), &verticesGL[0].color);

        int vertexLocation = shaders.attributeLocation("qt_Vertex");
        shaders.enableAttributeArray(vertexLocation);
        shaders.setAttributeArray(vertexLocation, GL_FLOAT, &verticesGL[0].position_local, 3, sizeof(VertexGL));
        int normalLocation = shaders.attributeLocation("qt_Normal");
        shaders.enableAttributeArray(normalLocation);
        shaders.setAttributeArray(normalLocation, GL_FLOAT, &verticesGL[0].normal_local, 3, sizeof(VertexGL));
        int colorLocation = shaders.attributeLocation("qt_Color");
        shaders.enableAttributeArray(colorLocation);
        shaders.setAttributeArray(colorLocation, GL_FLOAT, &verticesGL[0].color, 3, sizeof(VertexGL));
    }

    virtual void drawPolyhedron() override
    {
        glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, indices.data());
    }
};

class Pyramid : public Polyhedron {
public:
    Pyramid(QOpenGLShaderProgram& shaders) : Polyhedron(shaders) {
//                       x   y   z    nx  ny  nz    r  g  b
//        verticesGL = {{{ 1, -1,  1}, { 1, -1,  1}, {1, 0, 0} },
//                     { {-1, -1,  1}, {-1, -1,  1}, {0, 0, 1} },
//                     { { 1, -1, -1}, { 1, -1, -1}, {1, 0, 0} },
//                     { {-1, -1, -1}, {-1, -1, -1}, {0, 0, 1} },
//                     { { 1,  1,  1}, { 1,  1,  1}, {1, 0, 0} },
//                     { {-1,  1,  1}, {-1,  1,  1}, {0, 0, 1} },
//                     { {-1,  1, -1}, {-1,  1, -1}, {1, 0, 0} },
//                     { { 1,  1, -1}, { 1,  1, -1}, {0, 0, 1}}};
//        for (auto& [_,norm,__] : verticesGL) norm.normalize();
//        indices = { 1, 0, 5, 4, 7, 0, 2, 1, 3, 5, 6, 7, 3, 2};
        verticesGL = {{{ 1, -1,  1}, { 1, -1,  1}, {1, 1, 1} },
                     { {-1, -1,  1}, {-1, -1,  1}, {1, 1, 1} },
                     { {-1, -1, -1}, {-1, -1, -1}, {1, 1, 1} },
                     { { 1, -1, -1}, { 1, -1, -1}, {1, 1, 1} },
                     { { 0,  1,  0}, { 0,  1,  0}, {1, 1, 1}}};
        for (auto& [_,norm,__] : verticesGL) norm.normalize();
//        indices = { 1, 0, 5, 4, 7, 0, 2, 1, 3, 5, 6, 7, 3, 2 };
        indices = { 4, 3, 2, 1, 0, 3 };

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(VertexGL), &verticesGL[0].position_local);
        glNormalPointer(GL_FLOAT, sizeof(VertexGL), &verticesGL[0].normal_local);
        glColorPointer(3, GL_FLOAT, sizeof(VertexGL), &verticesGL[0].color);
        int vertexLocation = shaders.attributeLocation("qt_Vertex");
        shaders.enableAttributeArray(vertexLocation);
        shaders.setAttributeArray(vertexLocation, GL_FLOAT, &verticesGL[0].position_local, 3, sizeof(VertexGL));
        int normalLocation = shaders.attributeLocation("qt_Normal");
        shaders.enableAttributeArray(normalLocation);
        shaders.setAttributeArray(normalLocation, GL_FLOAT, &verticesGL[0].normal_local, 3, sizeof(VertexGL));
        int colorLocation = shaders.attributeLocation("qt_Color");
        shaders.enableAttributeArray(colorLocation);
        shaders.setAttributeArray(colorLocation, GL_FLOAT, &verticesGL[0].color, 3, sizeof(VertexGL));
    }

    virtual void drawPolyhedron() override
    {
//        glDrawElements(GL_LINE_STRIP, indices.size(), GL_UNSIGNED_INT, indices.data());
        glDrawElements(GL_TRIANGLE_FAN, indices.size(), GL_UNSIGNED_INT, indices.data());
//        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, indices.data());
    }
};

//class Pyramid : public Polyhedron {
//public:
//    Pyramid() : Polyhedron() {
//        for (double x : {-L, L})
//            for (double z : {-L, L})
//                vertices.push_back(std::make_shared<Vertex>(x, 1.5*L, z));
//        vertices.push_back(std::make_shared<Vertex>(0, -1.5*L, 0));
//        QVector<QVector<int> > planes = { { 0, 1, 3 },
//                                          { 0, 3, 2 },
//                                          { 0, 2, 4 },
//                                          { 0, 4, 1 },
//                                          { 1, 4, 3 },
//                                          { 2, 3, 4 } };
//        for (const auto& plane : planes) {
//            polygons.push_back(std::make_shared<Polygon>(QVector{ vertices[plane[0]],
//                                                                  vertices[plane[1]],
//                                                                  vertices[plane[2]] }));
//            vertices[plane[0]]->polygons.push_back(polygons.back());
//            vertices[plane[1]]->polygons.push_back(polygons.back());
//            vertices[plane[2]]->polygons.push_back(polygons.back());
//            polygons.back()->create_normal();
//            polygons.back()->color = (rand() & 0x00FFFFFF);
//        }
//        for (auto& v : vertices) v->create_normal();
//    }
//};

//class ConeMesh : public Polyhedron {
//public:
//    ConeMesh(double r1, double r2, double h, double base_ratio,
//             int h_appr, int v_appr, int r_appr) : Polyhedron() {
//        const double step = 2.0 * acos(-1) / h_appr;
//        // create side vertices: (v_appr + 1) * h_appr
//        for (int i = 0; i <= v_appr; i++) {
//            double lvl_ratio = 1.0 - (1.0 - base_ratio) * i / v_appr;
//            double t = 0;
//            for (int j = 0; j < h_appr; j++, t += step) {
//                vertices.push_back(std::make_shared<Vertex>(lvl_ratio * r1 * cos(t),
//                                                            -(h * i / v_appr - h / 2),
//                                                            lvl_ratio * r2 * sin(t)));
//            }
//        }
//        // create bottom vertices: (r_appr - 1) * h_appr + 1
//        for (int i = 0; i < r_appr - 1; i++) {
//            double t = 0;
//            double k = (double)(r_appr-i-1) / r_appr;
//            for (int j = 0; j < h_appr; j++, t += step) {
//                vertices.push_back(std::make_shared<Vertex>(r1 * k * cos(t),
//                                                            h / 2,
//                                                            r2 * k * sin(t)));
//            }
//        }
//        vertices.push_back(std::make_shared<Vertex>(0,  h / 2, 0));
//        // create top vertices: (r_appr - 1) * h_appr + 1
//        for (int i = 0; i < r_appr - 1; i++) {
//            double t = 0;
//            double k = base_ratio * (r_appr-i-1) / r_appr;
//            for (int j = 0; j < h_appr; j++, t += step) {
//                vertices.push_back(std::make_shared<Vertex>(r1 * k * cos(t),
//                                                            - h / 2,
//                                                            r2 * k * sin(t)));
//            }
//        }
//        vertices.push_back(std::make_shared<Vertex>(0, -h / 2, 0));
//        // create side polygons
//        for (int i = 1; i <= v_appr; i++) {
//            for (int j = 0; j < h_appr; j++) {
//                polygons.push_back(std::make_shared<Polygon>(QVector
//                                           { vertices[ i    * h_appr + j           ]       //  1
//                                           , vertices[(i-1) * h_appr + j           ]       //  |\.
//                                           , vertices[(i-1) * h_appr + (j+1)%h_appr] }));  //  2-3
//                polygons.back()->create_normal();
//                polygons.back()->color = (rand() & 0x00FFFFFF);
//                vertices[   i  * h_appr + j           ]->polygons.push_back(polygons.back());
//                vertices[(i-1) * h_appr + j           ]->polygons.push_back(polygons.back());
//                vertices[(i-1) * h_appr + (j+1)%h_appr]->polygons.push_back(polygons.back());
//                polygons.push_back(std::make_shared<Polygon>(QVector
//                                           { vertices[   i  * h_appr +  j          ]       //  1-3
//                                           , vertices[(i-1) * h_appr + (j+1)%h_appr]       //   \|
//                                           , vertices[   i  * h_appr + (j+1)%h_appr] }));  //    2
//                polygons.back()->create_normal();
//                polygons.back()->color = (rand() & 0x00FFFFFF);
//                vertices[   i  * h_appr +  j          ]->polygons.push_back(polygons.back());
//                vertices[(i-1) * h_appr + (j+1)%h_appr]->polygons.push_back(polygons.back());
//                vertices[   i  * h_appr + (j+1)%h_appr]->polygons.push_back(polygons.back());
//            }
//        }
//        QVector<std::shared_ptr<Vertex> > last(vertices.begin(), vertices.begin() + h_appr);
//        // create bottom edge polygons
//        int offset = (v_appr + 1) * h_appr;
//        if (r_appr > 1)
//            for (int i = 0; i < h_appr; i++) {
//                polygons.push_back(std::make_shared<Polygon>(QVector
//                                                   { vertices[           i         ]
//                                                   , vertices[      offset + i     ]
//                                                   , vertices[offset + (i+1)%h_appr]}));
//                polygons.back()->create_normal();
//                polygons.back()->color = (rand() & 0x00FFFFFF);
//                vertices[           i         ]->polygons.push_back(polygons.back());
//                vertices[      offset + i     ]->polygons.push_back(polygons.back());
//                vertices[offset + (i+1)%h_appr]->polygons.push_back(polygons.back());
//                polygons.push_back(std::make_shared<Polygon>(QVector
//                                                   { vertices[          i          ]
//                                                   , vertices[offset + (i+1)%h_appr]
//                                                   , vertices[   (i + 1) % h_appr  ]}));
//                polygons.back()->create_normal();
//                polygons.back()->color = (rand() & 0x00FFFFFF);
//                vertices[          i          ]->polygons.push_back(polygons.back());
//                vertices[offset + (i+1)%h_appr]->polygons.push_back(polygons.back());
//                vertices[   (i + 1) % h_appr  ]->polygons.push_back(polygons.back());
//                last[i] = vertices[offset + i];
//            }
//        // create bottom polygons
//        for (int i = 1; i < r_appr - 1; i++) {
//            for (int j = 0; j < h_appr; j++) {
//                polygons.push_back(std::make_shared<Polygon>(QVector
//                                              {     last[                j               ]
//                                              , vertices[      offset + i*h_appr + j     ]
//                                              , vertices[offset + i*h_appr + (j+1)%h_appr]}));
//                polygons.back()->create_normal();
//                polygons.back()->color = (rand() & 0x00FFFFFF);
//                    last[                j               ]->polygons.push_back(polygons.back());
//                vertices[      offset + i*h_appr + j     ]->polygons.push_back(polygons.back());
//                vertices[offset + i*h_appr + (j+1)%h_appr]->polygons.push_back(polygons.back());
//                polygons.push_back(std::make_shared<Polygon>(QVector
//                                              {     last[                j               ]
//                                              , vertices[offset + i*h_appr + (j+1)%h_appr]
//                                              ,     last[          (j + 1)%h_appr        ]}));
//                polygons.back()->create_normal();
//                polygons.back()->color = (rand() & 0x00FFFFFF);
//                    last[                j               ]->polygons.push_back(polygons.back());
//                vertices[offset + i*h_appr + (j+1)%h_appr]->polygons.push_back(polygons.back());
//                    last[          (j + 1)%h_appr        ]->polygons.push_back(polygons.back());
//            }
//            last = { vertices.begin() + offset + i*h_appr,
//                     vertices.begin() + offset + i*h_appr + h_appr };
//        }
//        // create bottom center polygons
//        offset = (v_appr+1)*h_appr + h_appr*(r_appr-1);
//        for (int i = 0; i < h_appr; i++) {
//            polygons.push_back(std::make_shared<Polygon>(QVector
//                                          { vertices[     offset     ]
//                                          ,     last[(i + 1) % h_appr]
//                                          ,     last[        i       ]}));
//            polygons.back()->create_normal();
//            polygons.back()->color = (rand() & 0x00FFFFFF);
//            vertices[     offset     ]->polygons.push_back(polygons.back());
//                last[(i + 1) % h_appr]->polygons.push_back(polygons.back());
//                last[        i       ]->polygons.push_back(polygons.back());
//        }
//        last = { vertices.begin() + v_appr * h_appr,
//                 vertices.begin() + v_appr * h_appr + h_appr };
//        // create top edge polygons
//        offset = (v_appr + 1) * h_appr + (r_appr - 1) * h_appr + 1;
//        if (r_appr > 1) {
//            for (int i = 0; i < h_appr; i++) {
//                polygons.push_back(std::make_shared<Polygon>(QVector
//                                                   { vertices[ offset + i ]
//                                                   ,     last[      i     ]
//                                                   ,     last[(i+1)%h_appr]}));
//                polygons.back()->create_normal();
//                polygons.back()->color = (rand() & 0x00FFFFFF);
//                vertices[ offset + i ]->polygons.push_back(polygons.back());
//                    last[      i     ]->polygons.push_back(polygons.back());
//                    last[(i+1)%h_appr]->polygons.push_back(polygons.back());
//                polygons.push_back(std::make_shared<Polygon>(QVector
//                                                   { vertices[     offset + i    ]
//                                                   ,     last[  (i + 1) % h_appr ]
//                                                   , vertices[offset+(i+1)%h_appr]}));
//                polygons.back()->create_normal();
//                polygons.back()->color = (rand() & 0x00FFFFFF);
//                vertices[     offset + i    ]->polygons.push_back(polygons.back());
//                    last[  (i + 1) % h_appr ]->polygons.push_back(polygons.back());
//                vertices[offset+(i+1)%h_appr]->polygons.push_back(polygons.back());
//            }
//            last = { vertices.begin() + offset, vertices.begin() + offset + h_appr };
//        }
//        // create top polygons
//        offset = (v_appr + 1) * h_appr + (r_appr - 1) * h_appr + 1;
//        for (int i = 1; i < r_appr - 1; i++) {
//            for (int j = 0; j < h_appr; j++) {
//                polygons.push_back(std::make_shared<Polygon>(QVector
//                                              { vertices[offset + i*h_appr + j]
//                                              ,     last[          j          ]
//                                              ,     last[   (j + 1) % h_appr  ]}));
//                polygons.back()->create_normal();
//                polygons.back()->color = (rand() & 0x00FFFFFF);
//                vertices[offset + i*h_appr + j]->polygons.push_back(polygons.back());
//                    last[          j          ]->polygons.push_back(polygons.back());
//                    last[   (j + 1) % h_appr  ]->polygons.push_back(polygons.back());
//                polygons.push_back(std::make_shared<Polygon>(QVector
//                                              { vertices[      offset + i*h_appr + j     ]
//                                              ,     last[        (j + 1) % h_appr        ]
//                                              , vertices[offset + i*h_appr + (j+1)%h_appr]}));
//                polygons.back()->create_normal();
//                polygons.back()->color = (rand() & 0x00FFFFFF);
//                vertices[      offset + i*h_appr + j     ]->polygons.push_back(polygons.back());
//                    last[        (j + 1) % h_appr        ]->polygons.push_back(polygons.back());
//                vertices[offset + i*h_appr + (j+1)%h_appr]->polygons.push_back(polygons.back());
//            }
//            last = { vertices.begin() + offset + i*h_appr,
//                     vertices.begin() + offset + i*h_appr + h_appr };
//        }
//        // create top center polygons
//        offset = vertices.size() - 1;
//        for (int i = 0; i < h_appr; i++) {
//            polygons.push_back(std::make_shared<Polygon>(QVector
//                                          { vertices[     offset     ]
//                                          ,     last[        i       ]
//                                          ,     last[(i + 1) % h_appr]}));
//            polygons.back()->create_normal();
//            polygons.back()->color = (rand() & 0x00FFFFFF);
//            vertices[     offset     ]->polygons.push_back(polygons.back());
//                last[        i       ]->polygons.push_back(polygons.back());
//                last[(i + 1) % h_appr]->polygons.push_back(polygons.back());
//        }
//        for (auto& v : vertices) v->create_normal();
//    }
//};

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

inline QVector4D reflectVector(QVector4D vec, QVector4D nor)
{
    return vec - 2 * nor * QVector4D::dotProduct(vec, nor);
}

#endif // PRIMITIVES_H
