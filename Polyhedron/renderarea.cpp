#include "renderarea.h"

const QMatrix4x4 RenderArea::viewSide  = { 0,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
const QMatrix4x4 RenderArea::viewTop   = { 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
const QMatrix4x4 RenderArea::viewFront = { 1,0,0,0, 0,1,0,0, 0,0,0,0, 0,0,0,1 };
const QMatrix4x4 RenderArea::viewOrtho = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
    , faceVariant(NONE)
    , isDrawingWireframe(true)
    , isDrawingNormals(false)
    , isNormalMethodEnabled(true)
    , isZSortingEnabled(false)
{
    QWidget::resize(parent->size());
    update();
}

void RenderArea::update()
{
    point_WorldTrans = shift * rotate.transposed() * scale * point_viewport;
    vector_WorldTrans = NormalVecTransf(point_WorldTrans);
    QWidget::update();
    emit debug(point_WorldTrans);
}

const QPoint RenderArea::getCenter() const
{
    return { width() / 2, height() / 2 };
}

void RenderArea::paintEvent(QPaintEvent*)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::GlobalColor::gray);
    painter.drawRect(0, 0, width()-1, height()-1);

    // plot axes
    painter.translate(60, 60);
    painter.setBrush(QBrush(Qt::GlobalColor::gray,
                            Qt::BrushStyle::Dense7Pattern));
    painter.drawEllipse({0, 0}, 55, 55);
    QVector3D Ax(50, 0, 0);
    QVector3D Ay(0, 50, 0);
    QVector3D Az(0, 0, 50);
    painter.setPen(Qt::GlobalColor::red);
    painter.drawLine(QPoint(0, 0), (rotate * Ax).toPoint());
    painter.setPen(Qt::GlobalColor::green);
    painter.drawLine(QPoint(0, 0), (rotate * Ay).toPoint());
    painter.setPen(Qt::GlobalColor::blue);
    painter.drawLine(QPoint(0, 0), (rotate * Az).toPoint());
    painter.translate(-60, -60);

    // to screen space
    painter.translate(getCenter());

    // transform cube
    for (auto& v : figure.vertices)
        v.point_world = point_WorldTrans * v.point_local;
    for (auto& p : figure.polygons)
        p.normal_world = vector_WorldTrans * p.normal_local;

    QVector<Polygon> tmpPolygons;
    if (isZSortingEnabled) {
        tmpPolygons = figure.polygons;
        std::sort(figure.polygons.begin(), figure.polygons.end(),
                  [&](const Polygon& lhs, const Polygon& rhs) {
            if (!qFuzzyCompare(lhs.mid().z(), rhs.mid().z()))
                return lhs.mid().z() > rhs.mid().z();
            return lhs.normal_world.z() > rhs.normal_world.z();
        });
    }

    // plot cube
    for (const auto& p : qAsConst(figure.polygons)) {
        QVector<QPointF> proj;
        for (const auto v : p.vertices)
            proj.push_back(v->point_world.toPointF());
        if (isNormalMethodEnabled && p.normal_world.z() >= 0) continue;
        painter.setBrush(faceVariant == RANDOM  ? QBrush(p.color) :
                         faceVariant == DEFAULT ? QBrush(Qt::GlobalColor::cyan)
                                                : QBrush(Qt::BrushStyle::NoBrush));
        painter.setPen(isDrawingWireframe ? Qt::GlobalColor::black
                                          : Qt::GlobalColor::transparent);
        painter.drawPolygon(proj);
        if (isDrawingNormals) {
            painter.setPen(Qt::GlobalColor::red);
            painter.setBrush(Qt::GlobalColor::red);
            painter.drawEllipse(p.mid().toPoint(), 2, 2);
            painter.drawLine(p.mid().toPoint(), (p.mid()+ p.normal_world).toPoint());
            painter.drawEllipse((p.mid() + p.normal_world).toPoint(), 4, 4);
        }
    }
    if (isZSortingEnabled) figure.polygons = tmpPolygons;
    painter.end();
}

void RenderArea::mousePressEvent(QMouseEvent *event)
{
    prevPos = event->pos();
    setCursor(Qt::CursorShape::DragMoveCursor);
}

void RenderArea::mouseMoveEvent(QMouseEvent *event)
{
    switch (event->modifiers()) {
    case Qt::ControlModifier: {
        QPoint s = prevPos - shift * getCenter();
        QPoint p = event->pos() - shift * getCenter();
        double delta = atan2(QPoint::dotProduct(s, p),
                             s.x()*p.y()-s.y()*p.x()) - M_PI/2;
        rotateZ(delta * 180.0 / M_PI);
        break;
    }
    case Qt::ShiftModifier: {
        QPoint delta = event->pos() - prevPos;
        doShift(delta.x(), delta.y(), 0);
        break;
    }
    default: {
        QPoint delta = event->pos() - prevPos;
        rotateX(-delta.y(), true);
        rotateY(delta.x());
        break;
    }
    }
    prevPos = event->pos();
}

void RenderArea::mouseReleaseEvent(QMouseEvent*)
{
    unsetCursor();
}

void RenderArea::wheelEvent(QWheelEvent *event)
{
    QMatrix4x4 E;
    if (!event->pixelDelta().isNull())
        E.scale(event->pixelDelta().y());
    else if (!event->angleDelta().isNull())
        E.scale(event->angleDelta().y());
    else
        E.scale(0);
    E(3, 3) = 0;
    setScale(scale + E * 0.003);
}

QMatrix4x4 RenderArea::NormalVecTransf(const QMatrix4x4 &m)
{
    return QMatrix4x4(
                m(2, 2) * m(1, 1) - m(1, 2) * m(2, 1),
                m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2),
                m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1), 0,
                m(0, 2) * m(2, 1) - m(2, 2) * m(0, 1),
                m(2, 2) * m(0, 0) - m(0, 2) * m(2, 0),
                m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1), 0,
                m(1, 2) * m(0, 1) - m(0, 2) * m(1, 1),
                m(1, 0) * m(0, 2) - m(1, 2) * m(0, 0),
                m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1), 0,
                0,           0,           0,           0);
}

void RenderArea::setFigure(const Polyhedron &newFigure)
{
    figure = newFigure;
    update();
}

void RenderArea::setPoint_viewport(const QMatrix4x4 &newPoint_viewport)
{
    point_viewport = newPoint_viewport;
    update();
}

void RenderArea::setIsZSortingEnabled(bool newIsZBufferingEnabled)
{
    isZSortingEnabled = newIsZBufferingEnabled;
    update();
}

void RenderArea::setIsNormalMethodEnabled(bool newIsNormalMethodEnabled)
{
    isNormalMethodEnabled = newIsNormalMethodEnabled;
    update();
}

void RenderArea::setIsDrawingNormals(bool newIsDrawingNormals)
{
    isDrawingNormals = newIsDrawingNormals;
    update();
}

void RenderArea::setIsDrawWireframe(bool newIsDrawWireframe)
{
    isDrawingWireframe = newIsDrawWireframe;
    update();
}

void RenderArea::setSideView()
{
    point_viewport = viewSide;
    QMatrix4x4 E;
    E.rotate(-90, {0, 1, 0});
    setRotate(E);
    update();
}

void RenderArea::setFrontView()
{
    point_viewport = viewFront;
    setRotate({});
    update();
}

void RenderArea::setTopView()
{
    point_viewport = viewTop;
    QMatrix4x4 E;
    E.rotate(-90, {1, 0, 0});
    setRotate(E);
    update();
}

void RenderArea::setOrthoView()
{
    point_viewport = viewOrtho;
    update();
}

void RenderArea::positIsometric()
{
    QMatrix4x4 ret;
    ret.rotate(-45, {0, 1, 0});
    ret.rotate(-35, {1, 0, 0});
    setRotate(ret);
}

void RenderArea::setFaceVariant(FaceVariant newFaceVariant)
{
    if (faceVariant == newFaceVariant)
        return;
    faceVariant = newFaceVariant;
    update();
}

void RenderArea::rotateX(double degree, bool silent)
{
    rotate.rotate(degree, {1, 0, 0});
    if (!silent) update();
}

void RenderArea::rotateY(double degree, bool silent)
{
    rotate.rotate(degree, {0, 1, 0});
    if (!silent) update();
}

void RenderArea::rotateZ(double degree, bool silent)
{
    rotate.rotate(degree, {0, 0, 1});
    if (!silent) update();
}

void RenderArea::doShift(int x, int y, int z, bool silent)
{
    shift.translate(x, y, z);
    if (!silent) update();
    emit shiftChanged(shift);
}

void RenderArea::setShift(const QMatrix4x4 &newShift)
{
    if (shift == newShift)
        return;
    shift = newShift;
    shift.optimize();
    update();
    emit shiftChanged(shift);
}

void RenderArea::setRotate(const QMatrix4x4 &newRotate)
{
    if (rotate == newRotate)
        return;
    rotate = newRotate;
    rotate.optimize();
    update();
    emit rotateChanged();
}

void RenderArea::setScale(const QMatrix4x4 &newScale)
{
    if (newScale(0, 0) < 0
     || newScale(1, 1) < 0
     || newScale(2, 2) < 0
     || scale == newScale)
        return;
    scale = newScale;
    scale.optimize();
    update();
    emit scaleChanged(scale);
}

void RenderArea::resize(int w, int h)
{
    double ratio = (double) w / width();
    QWidget::resize(w, h);
    QMatrix4x4 newScale = scale;
    newScale.scale(ratio);
    setScale(newScale);
    QMatrix4x4 newShift = shift * ratio;
    for (int i = 0; i < 4; i++)
        newShift(i, i) = shift(i, i);
    setShift(newShift);
}
