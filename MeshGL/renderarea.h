#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QMatrix4x4>
#include <cmath>
#include "primitives.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

class RenderArea : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    using QOpenGLWidget::QOpenGLWidget;
    enum FaceVariant { DEFAULT, RANDOM, NONE };

    RenderArea(QWidget *parent);

    void resize(int w, int h);

    const QPoint getCenter() const;

    void setScale(const QMatrix4x4 &newScale);

    void setRotate(const QMatrix4x4 &newRotate);

    void setShift(const QMatrix4x4 &newShift);

    void recalc();

    void rotateX(double degree, bool silent = false);

    void rotateY(double degree, bool silent = false);

    void rotateZ(double degree, bool silent = false);

    void doShift(double x, double y, double z, bool silent = false);

    void setFaceVariant(FaceVariant newFaceVariant);

    void setIsPolygonNormals(bool newIsPolygonNormals);

    void setIsNormalMethodEnabled(bool newIsNormalMethodEnabled);

    void setIsZSortingEnabled(bool newIsZSortingEnabled);

    template<class Figure, class... Args>
    void setFigure(Args... args);

    void setIsVertexNormals(bool newIsVertexNormals);

public slots:
    void setIsDrawWireframe(bool newIsDrawWireframe);

    void setSideView();

    void setFrontView();

    void setTopView();

    void setOrthoView();

    void positIsometric();

signals:
    void scaleChanged(QMatrix4x4);

    void rotateChanged();

    void shiftChanged(QMatrix4x4);

    void debug(QMatrix4x4);

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;
    virtual void mousePressEvent  (QMouseEvent *event) override;
    virtual void mouseMoveEvent   (QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent       (QWheelEvent *event) override;

private:
    QMatrix4x4 NormalVecTransf(const QMatrix4x4& m) const;
    void plotAxes(QPainter& painter);
    void plotFigure(QPainter& painter);

public:
    QOpenGLShaderProgram shaders;

private:
    std::unique_ptr<Polyhedron> figure;
    QMatrix4x4 scale;
    QMatrix4x4 shift;
    QPoint prevPos;
    QMatrix4x4 rotate;
    QMatrix4x4 point_WorldTrans;
    QMatrix4x4 vector_WorldTrans;
    QMatrix4x4 point_viewport;
    FaceVariant faceVariant;
    QMatrix4x4 perspect_proj;
    bool isDrawingWireframe;
    bool isPolygonNormals;
    bool isVertexNormals;
    bool isNormalMethodEnabled;
    bool isZSortingEnabled;
};

template<class Figure, class... Args>
void RenderArea::setFigure(Args... args)
{
    figure.reset();
    figure = std::make_unique<Figure>(shaders, args...);
//    figure = std::unique_ptr<Polyhedron>(new Figure(shaders, args...));
    QOpenGLWidget::update();
}

#endif // RENDERAREA_H
