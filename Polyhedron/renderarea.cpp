#include "renderarea.h"

RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    QWidget::resize(parent->size());
    cube = Polyhedron::GenerateCube();
    update();
}

RenderArea::~RenderArea() {  }

void RenderArea::update()
{
    rotate = rotateX * rotateY * rotateZ;
//    world_trans = scale * rotate * shift;
//    world_trans = scale * shift * rotate;
//    world_trans = rotate * shift * scale;
//    world_trans = rotate * scale * shift;
    world_trans = shift * rotate * scale;
//    world_trans = shift * scale * rotate;
    QWidget::update();
}

const QPoint RenderArea::getCenter() const
{
    return { width() / 2, height() / 2 };
}

void RenderArea::paintEvent(QPaintEvent*)
{
    QPainter painter;
    painter.begin(this);
    painter.setPen(Qt::GlobalColor::gray);
    painter.drawRect(0, 0, width()-1, height()-1);

    // to screen space
    painter.translate(getCenter());

    // transform
    for (auto& v : cube.vertices) {
        v.point_world = world_trans * v.point_local;
    }

    // plot cube // from world space
    painter.setPen(Qt::GlobalColor::black);
    for (auto p : qAsConst(cube.polygons)) {
        QVector<QPointF> proj;
        for (auto v : qAsConst(p.vertices)) {
            proj.push_back({v->point_world.x(), v->point_world.y()});
        }
        painter.drawPolygon(proj);
    }
    painter.end();
}

void RenderArea::mousePressEvent(QMouseEvent *event)
{
    startPos = event->pos();
//    startShift = shift;
    startRotateX = rotateX;
    startRotateY = rotateY;
    setCursor(Qt::CursorShape::DragMoveCursor);
}

void RenderArea::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->pos() - startPos;
    QMatrix4x4 Ex;
    Ex.rotate(delta.y() * 0.01, 1, 0, 0);
    setRotateX(rotateX * Ex);
    QMatrix4x4 Ey;
    Ey.rotate(delta.x() * 0.01, 0, 1, 0);
    setRotateY(rotateY * Ey);
}

void RenderArea::mouseReleaseEvent(QMouseEvent*)
{
    unsetCursor();
}

void RenderArea::wheelEvent(QWheelEvent *event)
{
//    if (!event->pixelDelta().isNull())
//        setScale(QTransform(scale.m11() + event->pixelDelta().x()*0.1, 0, 0,
//                            scale.m22() + event->pixelDelta().y()*0.1, 0, 0));
//    else if (!event->angleDelta().isNull())
//        setScale(QTransform(scale.m11() + event->angleDelta().x()*0.1, 0, 0,
    //                            scale.m22() + event->angleDelta().y()*0.1, 0, 0));
}

void RenderArea::setRotateZ(const QMatrix4x4 &newRotateZ)
{
    if (rotateZ == newRotateZ)
        return;
    rotateZ = newRotateZ;
    update();
    emit rotateZChanged();
}

void RenderArea::setRotateY(const QMatrix4x4 &newRotateY)
{
    if (rotateY == newRotateY)
        return;
    rotateY = newRotateY;
    update();
    emit rotateYChanged(rotateY);
}

void RenderArea::setRotateX(const QMatrix4x4 &newRotateX)
{
    if (rotateX == newRotateX)
        return;
    rotateX = newRotateX;
    update();
    emit rotateXChanged(rotateX);
}

void RenderArea::setShift(const QMatrix4x4 &newShift)
{
    if (shift == newShift)
        return;
    shift = newShift;
    shift.optimize();
    update();
    emit shiftChanged();
}

void RenderArea::setRotate(const QMatrix4x4 &newRotate)
{
    if (rotate == newRotate)
        return;
    rotate = newRotate;
    update();
    emit rotateChanged();
}

void RenderArea::setScale(const QMatrix4x4 &newScale)
{
    if (scale == newScale)
        return;
    scale = newScale;
    scale.optimize();
    update();
    emit scaleChanged();
}

void RenderArea::resize(int w, int h)
{
//    double ratio = (double) w / width();
//    QWidget::resize(w, h);
//    setScale(QTransform(scale.m11() * ratio, 0, 0,
//                        scale.m22() * ratio, 0, 0));
//    setShift(QTransform(1, 0, 0, 1, shift.dx() * ratio,
//                                    shift.dy() * ratio));
}
