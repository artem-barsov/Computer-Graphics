#include "renderarea.h"

const QMatrix4x4 RenderArea::viewSide  = { 0,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
const QMatrix4x4 RenderArea::viewTop   = { 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
const QMatrix4x4 RenderArea::viewFront = { 1,0,0,0, 0,1,0,0, 0,0,0,0, 0,0,0,1 };
const QMatrix4x4 RenderArea::viewOrtho = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

RenderArea::RenderArea(QWidget *parent)
    : QWidget               (parent)
    , faceVariant           (DEFAULT)
    , isDrawingWireframe    (false)
    , isDrawingNormals      (false)
    , isNormalMethodEnabled (true)
    , isZSortingEnabled     (false)
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

    // transform
    if (isZSortingEnabled) {
        std::sort(figure.polygons.begin(), figure.polygons.end(),
                  [&](const Polygon& lhs, const Polygon& rhs) {
            if (!qFuzzyCompare(lhs.mid().point_world.z(), rhs.mid().point_world.z()))
                return lhs.mid().point_world.z() > rhs.mid().point_world.z();
            return lhs.normal_world.z() > rhs.normal_world.z();
        });
    }
    lighter.pos_world = lighter.rotate.transposed() *
            QVector4D(0, 0, lighter.distance, 1);
    for (auto& p : figure.polygons)
        p.normal_world = vector_WorldTrans * p.normal_local;
    for (auto& v : figure.vertices) {
        v.point_world = point_WorldTrans * v.point_local;
        v.normal_world = vector_WorldTrans * v.normal_local;
        v.light = v.ambient * lighter.ambient;
        QVector4D L = (lighter.pos_world - v.point_world).normalized();
        double cosAngl = QVector4D::dotProduct(L, v.normal_world.normalized());
        if (cosAngl > 0) v.light += v.diffuse * lighter.intensity * cosAngl;
    }

    // plot
    painter.translate(60, 60);
    plotAxes(painter);
    painter.translate(-60, -60);

    painter.translate(getCenter());
    if ((point_WorldTrans * QVector4D()).z() > lighter.pos_world.z()) {
        plotFigure(painter);
        plotLighter(painter);
    }
    else {
        plotLighter(painter);
        plotFigure(painter);
    }
    painter.translate(-getCenter());

    painter.end();
}

void RenderArea::mousePressEvent(QMouseEvent *event)
{
    prevPos = event->pos();
    setCursor(Qt::CursorShape::DragMoveCursor);
}

void RenderArea::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->pos() - prevPos;
    switch (event->modifiers()) {
    case Qt::ControlModifier: {
        QPoint s = prevPos - shift * getCenter();
        QPoint p = event->pos() - shift * getCenter();
        double delta = atan2(QPoint::dotProduct(s, p),
                             s.x()*p.y()-s.y()*p.x()) - M_PI/2;
        rotateZ(delta * 180.0 / M_PI);
        break;
    }
    case Qt::ControlModifier | Qt::AltModifier: {
        QPoint s = prevPos - shift * getCenter();
        QPoint p = event->pos() - shift * getCenter();
        double delta = atan2(QPoint::dotProduct(s, p),
                             s.x()*p.y()-s.y()*p.x()) - M_PI/2;
        lighter_rotateZ(delta * 180.0 / M_PI);
        break;
    }
    case Qt::ShiftModifier: {
        doShift(delta.x(), delta.y(), 0);
        break;
    }
    case Qt::AltModifier: {
        lighter_rotateX(-delta.y(), true);
        lighter_rotateY(delta.x());
        break;
    }
    default: {
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
    QPoint delta;
    if (!event->pixelDelta().isNull())
        delta = event->pixelDelta();
    else if (!event->angleDelta().isNull())
        delta = event->angleDelta();
    else
        delta = {0,0};
    if (event->modifiers() == Qt::NoModifier) {
        QMatrix4x4 E;
        E.scale(delta.y());
        E(3, 3) = 0;
        setScale(scale + E * 0.003);
    }
    else if (event->modifiers() == Qt::AltModifier) {
        setLighterDistance(lighter.distance + delta.x());
    }
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

void RenderArea::plotAxes(QPainter &painter)
{
    painter.setBrush(QBrush(Qt::GlobalColor::gray,
                            Qt::BrushStyle::Dense7Pattern));
    painter.drawEllipse({0, 0}, 55, 55);
    QPoint Ax = (rotate * QVector3D(50, 0, 0)).toPoint();
    QPoint Ay = (rotate * QVector3D(0, 50, 0)).toPoint();
    QPoint Az = (rotate * QVector3D(0, 0, 50)).toPoint();
    painter.setPen(Qt::GlobalColor::red);
    painter.drawLine(QPoint(0, 0), Ax);
    painter.drawText(Ax, "X");
    painter.setPen(Qt::GlobalColor::green);
    painter.drawLine(QPoint(0, 0), Ay);
    painter.drawText(Ay, "Y");
    painter.setPen(Qt::GlobalColor::blue);
    painter.drawLine(QPoint(0, 0), Az);
    painter.drawText(Az, "Z");
}

void RenderArea::plotFigure(QPainter &painter)
{
    for (const auto& p : qAsConst(figure.polygons)) {
        QVector<QPointF> proj;
        for (const auto& v : p.vertices)
            proj.push_back(v->point_world.toPointF());
        if (isNormalMethodEnabled && p.normal_world.z() >= 0) continue;
        QBrush faceBrush;
        if (faceVariant == RANDOM)
            faceBrush = p.color;
        else if (faceVariant == NONE)
            faceBrush = Qt::BrushStyle::NoBrush;
        else {
            QVector3D midIntens = p.mid().light;
            faceBrush = QColor(ceil(0xFF * std::min(1.0f, midIntens[0])),
                               ceil(0xFF * std::min(1.0f, midIntens[1])),
                               ceil(0xFF * std::min(1.0f, midIntens[2])));
        }
        painter.setBrush(faceBrush);
        painter.setPen(isDrawingWireframe ? Qt::GlobalColor::white
                                          : faceBrush.color());
        painter.drawPolygon(proj);
        if (isDrawingNormals) {
            painter.setPen(Qt::GlobalColor::red);
            painter.setBrush(Qt::GlobalColor::red);
            painter.drawEllipse(p.mid().point_world.toPoint(), 2, 2);
            painter.drawLine(p.mid().point_world.toPoint(),
                            (p.mid().point_world + p.normal_world).toPoint());
            painter.drawEllipse((p.mid().point_world + p.normal_world).toPoint(), 4, 4);
        }
    }
}

void RenderArea::plotLighter(QPainter &painter)
{
    QColor clrAmbient(255 * lighter.ambient[0],
                      255 * lighter.ambient[1],
                      255 * lighter.ambient[2]);
    QColor clrIntensity(255 * lighter.intensity[0],
                        255 * lighter.intensity[1],
                        255 * lighter.intensity[2]);
    painter.setPen(clrAmbient);
    painter.setBrush(clrAmbient);
    painter.drawEllipse(lighter.pos_world.toPointF(), 20, 20);
    painter.setPen(clrIntensity);
    painter.setBrush(clrIntensity);
    painter.drawEllipse(lighter.pos_world.toPointF(), 10, 10);
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

void RenderArea::setIsZSortingEnabled(bool newIsZSortingEnabled)
{
    isZSortingEnabled = newIsZSortingEnabled;
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

void RenderArea::setLighterDistance(double newD)
{
    if (newD <= 0) return;
    lighter.distance = newD;
    QWidget::update();
    emit LighterDistanceChanged(newD);
}

void RenderArea::setLighterAmbient(QVector3D ia)
{
    lighter.ambient = ia;
    QWidget::update();
}

void RenderArea::setLighterIntensity(QVector3D il)
{
    lighter.intensity = il;
    QWidget::update();
}

void RenderArea::setFigureAmbient(QVector3D ka)
{
    for (auto& v : figure.vertices) v.ambient = ka;
    QWidget::update();
}

void RenderArea::setFigureDiffuse(QVector3D kd)
{
    for (auto& v : figure.vertices) v.diffuse = kd;
    QWidget::update();
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

void RenderArea::lighter_rotateX(double degree, bool silent)
{
    lighter.rotate.rotate(degree, {1, 0, 0});
    if (!silent) QWidget::update();
}

void RenderArea::lighter_rotateY(double degree, bool silent)
{
    lighter.rotate.rotate(degree, {0, 1, 0});
    if (!silent) QWidget::update();
}

void RenderArea::lighter_rotateZ(double degree, bool silent)
{
    lighter.rotate.rotate(degree, {0, 0, 1});
    if (!silent) QWidget::update();
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
