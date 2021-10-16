#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QPaintEvent>
#include "graph.h"

class RenderArea : public QWidget
{
    Q_OBJECT
public:
    RenderArea(QWidget *parent, Graph *graph, QPointF scl,
               QPoint sh, double angle);

    ~RenderArea();

    void resize(int w, int h);

    void setScale(const QTransform &newScale);

    void setShift(const QTransform &newShift);

    void setRotate(const QTransform &newRotate);

    const QPoint getCenter() const;

public slots:
    void update();

signals:
    void scaleChanged(QTransform);

    void shiftChanged(QTransform);

    void rotateChanged(QTransform);

protected:
    virtual void paintEvent       (QPaintEvent *event) override;
    virtual void mousePressEvent  (QMouseEvent *event) override;
    virtual void mouseMoveEvent   (QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent       (QWheelEvent *event) override;

private:
    QPoint     startPos;
    QTransform startShift;
    QTransform startRotate;

private:
    Graph     *graph;
    QTransform scale;
    QTransform shift;
    QTransform rotate;
    QTransform world_trans;
    Q_PROPERTY(QTransform scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QTransform shift WRITE setShift NOTIFY shiftChanged)
    Q_PROPERTY(QTransform rotate WRITE setRotate NOTIFY rotateChanged)
};

#endif // RENDERAREA_H
