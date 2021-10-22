#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QMatrix4x4>
#include "polyhedron.h"

class RenderArea : public QWidget
{
    Q_OBJECT
public:
    RenderArea(QWidget *parent);

    ~RenderArea();

    void resize(int w, int h);

    const QPoint getCenter() const;

    void setScale(const QMatrix4x4 &newScale);

    void setRotate(const QMatrix4x4 &newRotate);

    void setShift(const QMatrix4x4 &newShift);

    void setRotateX(const QMatrix4x4 &newRotateX);

    void setRotateY(const QMatrix4x4 &newRotateY);

    void setRotateZ(const QMatrix4x4 &newRotateZ);

public slots:
    void update();

signals:
    void scaleChanged();

    void rotateChanged();

    void shiftChanged();

    void rotateXChanged(QMatrix4x4);

    void rotateYChanged(QMatrix4x4);

    void rotateZChanged();

protected:
    virtual void paintEvent       (QPaintEvent *event) override;
    virtual void mousePressEvent  (QMouseEvent *event) override;
    virtual void mouseMoveEvent   (QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent       (QWheelEvent *event) override;

private:
    QPoint startPos;
    QMatrix4x4 startRotateX, startRotateY;

private:
    Polyhedron cube;
    QMatrix4x4 scale;
    QMatrix4x4 rotateX, rotateY, rotateZ;
    QMatrix4x4 rotate;
    QMatrix4x4 shift;
    QMatrix4x4 world_trans;
    Q_PROPERTY(QMatrix4x4 scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QMatrix4x4 rotate WRITE setRotate NOTIFY rotateChanged)
    Q_PROPERTY(QMatrix4x4 shift WRITE setShift NOTIFY shiftChanged)
    Q_PROPERTY(QMatrix4x4 rotateX WRITE setRotateX NOTIFY rotateXChanged)
    Q_PROPERTY(QMatrix4x4 rotateY WRITE setRotateY NOTIFY rotateYChanged)
    Q_PROPERTY(QMatrix4x4 rotateZ WRITE setRotateZ NOTIFY rotateZChanged)
};

#endif // RENDERAREA_H
