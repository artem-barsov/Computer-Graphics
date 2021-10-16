#include "renderarea.h"

RenderArea::RenderArea(QWidget *parent, Graph *graph,
                       QPointF scl, QPoint sh, double angle)
    : QWidget(parent)
    , graph(graph)
    , scale(QTransform(scl.x(), 0, 0, scl.y(), 0, 0))
    , shift(QTransform(1, 0, 0, 1, sh.x(), sh.y()))
    , rotate(QTransform(cos(angle), -sin(angle),
                        sin(angle), cos(angle), 0, 0))
    , world_trans(scale * rotate * shift)
{
    QWidget::resize(parent->size());
    connect(graph, &Graph::nChanged,
            this,  &RenderArea::update);
    connect(graph, &Graph::aChanged,
            this,  &RenderArea::update);
    connect(graph, &Graph::bChanged,
            this,  &RenderArea::update);
}

RenderArea::~RenderArea()
{
    delete graph;
}

void RenderArea::update()
{
    world_trans = scale * rotate * shift;
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

    // plot axes from world space
    int mx = this->height() + this->width();
    QLine axis(-mx, 0, mx, 0);
    QPolygonF arrow({ {0.6, 0.2}, {1, 0}, {0.6, -0.2} });
    painter.setPen(Qt::GlobalColor::blue);
    painter.drawLine(axis * world_trans);
    painter.drawPolyline(arrow * world_trans);
    painter.setPen(Qt::GlobalColor::darkGreen);
    painter.drawLine(axis * QTransform(0, -1, 1, 0, 0, 0) * world_trans);
    painter.drawPolyline(arrow * QTransform(0, -1, 1, 0, 0, 0) * world_trans);

    // plot graph from world space
    painter.setPen(Qt::GlobalColor::black);
    painter.drawPolygon(QPolygonF(graph->points()) * world_trans);
    painter.end();
}

void RenderArea::mousePressEvent(QMouseEvent *event)
{
    startPos = event->pos();
    startShift = shift;
    startRotate = rotate;
    setCursor(Qt::CursorShape::DragMoveCursor);
}

void RenderArea::mouseMoveEvent(QMouseEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier) {
        QPoint s = startPos - getCenter() * shift;
        QPoint p = event->pos() - getCenter() * shift;
        double delta = atan2(QPoint::dotProduct(s, p),
                             s.x()*p.y()-s.y()*p.x()) - M_PI/2;
        double cosd = cos(delta), sind = sin(delta);
        setRotate(startRotate * QTransform(cosd, -sind, sind, cosd, 0, 0));
    }
    else {
        QPoint delta = event->pos() - startPos;
        setShift(startShift * QTransform(1, 0, 0, 1, delta.x(), delta.y()));
    }
}

void RenderArea::mouseReleaseEvent(QMouseEvent*)
{
    unsetCursor();
}

void RenderArea::wheelEvent(QWheelEvent *event)
{
    if (!event->pixelDelta().isNull())
        setScale(QTransform(scale.m11() + event->pixelDelta().x()*0.1, 0, 0,
                            scale.m22() + event->pixelDelta().y()*0.1, 0, 0));
    else if (!event->angleDelta().isNull())
        setScale(QTransform(scale.m11() + event->angleDelta().x()*0.1, 0, 0,
                            scale.m22() + event->angleDelta().y()*0.1, 0, 0));
}

void RenderArea::setRotate(const QTransform &newRotate)
{
    if (rotate == newRotate) return;
    rotate = newRotate;
    update();
    emit rotateChanged(rotate);
}

void RenderArea::setShift(const QTransform &newShift)
{
    if (shift == newShift) return;
    shift = newShift;
    update();
    emit shiftChanged(shift);
}

void RenderArea::setScale(const QTransform &newScale)
{
    if (scale == newScale) return;
    if (newScale.m11() < 0) return;
    if (newScale.m22() < 0) return;
    scale = newScale;
    update();
    emit scaleChanged(scale);
}

void RenderArea::resize(int w, int h)
{
    double ratio = (double) w / width();
    QWidget::resize(w, h);
    setScale(QTransform(scale.m11() * ratio, 0, 0,
                        scale.m22() * ratio, 0, 0));
    setShift(QTransform(1, 0, 0, 1, shift.dx() * ratio,
                                    shift.dy() * ratio));
}
