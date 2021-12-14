#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QMatrix4x4>
#include <cmath>
#include "primitives.h"

class RenderArea : public QWidget
{
    Q_OBJECT
public:
    enum FaceVariant { DEFAULT, RANDOM, NONE };

    RenderArea(QWidget *parent);

    void resize(int w, int h);

    const QPoint getCenter() const;

    void setScale(const QMatrix4x4 &newScale);

    void setRotate(const QMatrix4x4 &newRotate);

    void setShift(const QMatrix4x4 &newShift);

    void update();

    void rotateX(double degree, bool silent = false);

    void rotateY(double degree, bool silent = false);

    void rotateZ(double degree, bool silent = false);

    void lighter_rotateX(double degree, bool silent = false);

    void lighter_rotateY(double degree, bool silent = false);

    void lighter_rotateZ(double degree, bool silent = false);

    void doShift(int x, int y, int z, bool silent = false);

    void setFaceVariant(FaceVariant newFaceVariant);

    void setIsPolygonNormals(bool newIsPolygonNormals);

    void setIsNormalMethodEnabled(bool newIsNormalMethodEnabled);

    void setIsZSortingEnabled(bool newIsZSortingEnabled);

    void setFigure(Polyhedron *newFigure);

    void setLighterAmbient(QVector3D ia);

    void setLighterIntensity(QVector3D il);

    void setLighterMd(double md);

    void setLighterMk(double mk);

    void setFigureAmbient(QVector3D ka);

    void setFigureDiffuse(QVector3D kd);

    void setIsVertexNormals(bool newIsVertexNormals);

public slots:
    void setIsDrawWireframe(bool newIsDrawWireframe);

    void setSideView();

    void setFrontView();

    void setTopView();

    void setOrthoView();

    void positIsometric();

    void setLighterDistance(double d);

signals:
    void scaleChanged(QMatrix4x4);

    void rotateChanged();

    void shiftChanged(QMatrix4x4);

    void LighterDistanceChanged(double);

    void debug(QMatrix4x4);

protected:
    virtual void paintEvent       (QPaintEvent *event) override;
    virtual void mousePressEvent  (QMouseEvent *event) override;
    virtual void mouseMoveEvent   (QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent       (QWheelEvent *event) override;

private:
    QMatrix4x4 NormalVecTransf(const QMatrix4x4& m);
    void plotAxes(QPainter& painter);
    void plotFigure(QPainter& painter);
    void plotLighter(QPainter& painter);

private:
    Lighter lighter;
    std::unique_ptr<Polyhedron> figure;
    QMatrix4x4 scale;
    QMatrix4x4 rotate;
    QMatrix4x4 shift;
    QPoint prevPos;
    QMatrix4x4 point_WorldTrans;
    QMatrix4x4 vector_WorldTrans;
    QMatrix4x4 point_viewport;
    FaceVariant faceVariant;
    QBrush polygonPainting;
    bool isDrawingWireframe;
    bool isPolygonNormals;
    bool isVertexNormals;
    bool isNormalMethodEnabled;
    bool isZSortingEnabled;
    static const QMatrix4x4 viewSide;
    static const QMatrix4x4 viewTop;
    static const QMatrix4x4 viewFront;
    static const QMatrix4x4 viewOrtho;
    Q_PROPERTY(QMatrix4x4 scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QMatrix4x4 rotate WRITE setRotate NOTIFY rotateChanged)
    Q_PROPERTY(QMatrix4x4 shift WRITE setShift NOTIFY shiftChanged)
};

#endif // RENDERAREA_H
