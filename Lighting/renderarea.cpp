#include "renderarea.h"

const QMatrix4x4 RenderArea::viewSide  = { 0,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
const QMatrix4x4 RenderArea::viewTop   = { 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
const QMatrix4x4 RenderArea::viewFront = { 1,0,0,0, 0,1,0,0, 0,0,0,0, 0,0,0,1 };
const QMatrix4x4 RenderArea::viewOrtho = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

RenderArea::RenderArea(QWidget *parent) : QWidget(parent)
{
    QWidget::resize(parent->size());
    recalc();
}

void RenderArea::recalc()
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
    lighter.pos_world = lighter.rotate.transposed() * QVector4D(0, 0, lighter.distance, 1);
    for (auto& p : figure->polygons)
        p->normal_world = vector_WorldTrans * p->normal_local;
    for (auto& v : figure->vertices) {
        v->point_world  = point_WorldTrans * v->point_local;
        v->normal_world = vector_WorldTrans * v->normal_local;
        v->light = {0, 0, 0};
        v->light       += lighter.ambient * figure->ambient;
        QVector4D L     = lighter.pos_world - v->point_world;
        double cosAngl  = QVector4D::dotProduct(L.normalized(), v->normal_world.normalized());
        if (cosAngl > 0)
            v->light   += lighter.intensity * figure->diffuse * cosAngl
                        * lighter.distanceFunc(L.length());
        cosAngl = QVector4D::dotProduct(
                    reflectVector(L, v->normal_world.normalized()).normalized(), {0,0,1,0});
        if (cosAngl > 0)
            v->light   += lighter.intensity * figure->specular
                        * pow(cosAngl, figure->gloss) * lighter.distanceFunc(L.length());
    }
    if (isZSortingEnabled) {
        std::sort(figure->polygons.begin(), figure->polygons.end(),
                  [&](const std::shared_ptr<Polygon>& lhs, const std::shared_ptr<Polygon>& rhs) {
            if (!qFuzzyCompare(lhs->mid().point_world.z(), rhs->mid().point_world.z()))
                return lhs->mid().point_world.z() > rhs->mid().point_world.z();
            return lhs->normal_world.z() > rhs->normal_world.z();
        });
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

QMatrix4x4 RenderArea::NormalVecTransf(const QMatrix4x4 &m) const
{
    return { m(2, 2) * m(1, 1) - m(1, 2) * m(2, 1),
             m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2),
             m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1), 0,
             m(0, 2) * m(2, 1) - m(2, 2) * m(0, 1),
             m(2, 2) * m(0, 0) - m(0, 2) * m(2, 0),
             m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1), 0,
             m(1, 2) * m(0, 1) - m(0, 2) * m(1, 1),
             m(1, 0) * m(0, 2) - m(1, 2) * m(0, 0),
             m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1), 0,
             0,           0,           0,           0  };
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
    for (const auto& p : qAsConst(figure->polygons)) {
        if (isNormalMethodEnabled && p->normal_world.z() >= 0) continue;
        QVector<QPoint> proj;
        for (const auto& v : qAsConst(p->vertices))
            proj.push_back(v->point_world.toPoint());
        if (shadingVariant == FLAT) {
            QBrush faceBrush;
            if (faceVariant == NONE)
                faceBrush = Qt::BrushStyle::NoBrush;
            else {
                QColor clr = (faceVariant == RANDOM ? p->color : 0x00FFFFFF);
                QVector3D midIntens = p->mid().light;
                faceBrush = QColor(clr.red()   * std::min(1.0f, midIntens[0]),
                                   clr.green() * std::min(1.0f, midIntens[1]),
                                   clr.blue()  * std::min(1.0f, midIntens[2]));
            }
            painter.setBrush(faceBrush);
            painter.setPen(isDrawingWireframe ? Qt::GlobalColor::white
                                              : faceBrush.color());
            painter.drawPolygon(proj);
        }
        else { // if (shadingVariant == GOURAND)
            QColor clr = (faceVariant == RANDOM ? p->color : 0x00FFFFFF);
            plotTriangle(painter, proj[0], proj[1], proj[2],
                             QColor(clr.red()   * std::min(1.0f, p->vertices[0]->light[0]),
                                    clr.green() * std::min(1.0f, p->vertices[0]->light[1]),
                                    clr.blue()  * std::min(1.0f, p->vertices[0]->light[2])),
                             QColor(clr.red()   * std::min(1.0f, p->vertices[1]->light[0]),
                                    clr.green() * std::min(1.0f, p->vertices[1]->light[1]),
                                    clr.blue()  * std::min(1.0f, p->vertices[1]->light[2])),
                             QColor(clr.red()   * std::min(1.0f, p->vertices[2]->light[0]),
                                    clr.green() * std::min(1.0f, p->vertices[2]->light[1]),
                                    clr.blue()  * std::min(1.0f, p->vertices[2]->light[2])));
            if (isDrawingWireframe) {
                painter.setPen(Qt::GlobalColor::white);
                painter.setBrush(Qt::BrushStyle::NoBrush);
                painter.drawPolygon(proj);
            }
        }
    }
    if (isPolygonNormals) {
        painter.setPen(Qt::GlobalColor::darkRed);
        painter.setBrush(Qt::GlobalColor::red);
        for (const auto& p : qAsConst(figure->polygons)) {
            if ((isNormalMethodEnabled || isZSortingEnabled) && p->normal_world.z() >= 0)
                continue;
            painter.drawEllipse(p->mid().point_world.toPoint(), 2, 2);
            painter.drawLine(p->mid().point_world.toPoint(),
                            (p->mid().point_world + p->normal_world).toPoint());
            painter.drawEllipse((p->mid().point_world + p->normal_world).toPoint(), 2, 2);
        }
    }
    if (isVertexNormals) {
        painter.setPen(Qt::GlobalColor::darkGreen);
        painter.setBrush(Qt::GlobalColor::green);
        for (const auto& v : qAsConst(figure->vertices)) {
            if (isNormalMethodEnabled || isZSortingEnabled) {
                bool isDrawing = false;
                for (const auto& p : qAsConst(v->polygons)) {
                    if (p->normal_world.z() <= 0) {
                        isDrawing = true;
                        break;
                    }
                }
                if (!isDrawing) continue;
            }
            painter.drawEllipse(v->point_world.toPoint(), 2, 2);
            painter.drawLine(v->point_world.toPoint(),
                            (v->point_world + v->normal_world).toPoint());
            painter.drawEllipse((v->point_world + v->normal_world).toPoint(), 2, 2);
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

void RenderArea::plotTriangle(QPainter& painter, QPoint p1, QPoint p2,
                              QPoint p3, QColor c1, QColor c2, QColor c3)
{
    struct PointClr {
        struct { int x, y; }; QVector3D clr;
        PointClr(QPoint p, QColor c) : x(p.x()), y(p.y())
                                     , clr(c.red(), c.green(), c.blue()) { }
    };
    std::vector<PointClr> p = { { p1, c1 }, { p2, c2 }, { p3, c3 } };
    std::sort(p.begin(), p.end(), [](PointClr& lhs, PointClr& rhs) {
        if (lhs.y != rhs.y)
            return lhs.y < rhs.y;
        return lhs.x < rhs.x;
    });
    const int y10 = p[1].y - p[0].y;
    const int y20 = p[2].y - p[0].y;
    const int y21 = p[2].y - p[1].y;
    const int x10 = p[1].x - p[0].x;
    const int x20 = p[2].x - p[0].x;
    const int x21 = p[2].x - p[1].x;
    bool toswap = x10 * y20 > y10 * x20;
    for (int y = p[0].y; y < p[1].y; y++) {
        int x1 = (y - p[0].y) * x10 / y10 + p[0].x;
        int x2 = (y - p[0].y) * x20 / y20 + p[0].x;
        QVector3D clr1 = (p[1].clr * (y - p[0].y) + p[0].clr * (p[1].y - y)) / y10;
        QVector3D clr2 = (p[2].clr * (y - p[0].y) + p[0].clr * (p[2].y - y)) / y20;
        if (toswap) {
            std::swap(x1, x2);
            std::swap(clr1, clr2);
        }
        for (int x = x1; x < x2; x++) {
            QVector3D clr = (clr1 * (x2 - x) + clr2 * (x - x1)) / (x2 - x1);
            painter.setPen(QColor(clr[0], clr[1], clr[2]));
            painter.drawPoint(x, y);
        }
    }
    for (int y = p[1].y; y < p[2].y; y++) {
        int x1 = (y - p[1].y) * x21 / y21 + p[1].x;
        int x2 = (y - p[0].y) * x20 / y20 + p[0].x;
        QVector3D clr1 = (p[2].clr * (y - p[1].y) + p[1].clr * (p[2].y - y)) / y21;
        QVector3D clr2 = (p[2].clr * (y - p[0].y) + p[0].clr * (p[2].y - y)) / y20;
        if (toswap) {
            std::swap(x1, x2);
            std::swap(clr1, clr2);
        }
        for (int x = x1; x < x2; x++) {
            QVector3D clr = (clr1 * (x2 - x) + clr2 * (x - x1)) / (x2 - x1);
            painter.setPen(QColor(clr[0], clr[1], clr[2]));
            painter.drawPoint(x, y);
        }
    }
}

void RenderArea::setShadingVariant(ShadingVariant newShadingVariant)
{
    shadingVariant = newShadingVariant;
    QWidget::update();
}

void RenderArea::setIsVertexNormals(bool newIsVertexNormals)
{
    isVertexNormals = newIsVertexNormals;
    QWidget::update();
}

void RenderArea::setFigure(Polyhedron *newFigure)
{
    figure = std::unique_ptr<Polyhedron>(newFigure);
    QWidget::update();
}

void RenderArea::setIsZSortingEnabled(bool newIsZSortingEnabled)
{
    isZSortingEnabled = newIsZSortingEnabled;
    QWidget::update();
}

void RenderArea::setIsNormalMethodEnabled(bool newIsNormalMethodEnabled)
{
    isNormalMethodEnabled = newIsNormalMethodEnabled;
    QWidget::update();
}

void RenderArea::setIsPolygonNormals(bool newIsPolygonNormals)
{
    isPolygonNormals = newIsPolygonNormals;
    QWidget::update();
}

void RenderArea::setIsDrawWireframe(bool newIsDrawWireframe)
{
    isDrawingWireframe = newIsDrawWireframe;
    QWidget::update();
}

void RenderArea::setSideView()
{
    point_viewport = viewSide;
    QMatrix4x4 E;
    E.rotate(-90, {0, 1, 0});
    setRotate(E);
    recalc();
}

void RenderArea::setFrontView()
{
    point_viewport = viewFront;
    setRotate({});
    recalc();
}

void RenderArea::setTopView()
{
    point_viewport = viewTop;
    QMatrix4x4 E;
    E.rotate(-90, {1, 0, 0});
    setRotate(E);
    recalc();
}

void RenderArea::setOrthoView()
{
    point_viewport = viewOrtho;
    recalc();
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

void RenderArea::setLighterMd(double md)
{
    lighter.md = md;
    QWidget::update();
}

void RenderArea::setLighterMk(double mk)
{
    lighter.mk = mk;
    QWidget::update();
}

void RenderArea::setFigureAmbient(QVector3D ka)
{
    figure->ambient = ka;
    QWidget::update();
}

void RenderArea::setFigureDiffuse(QVector3D kd)
{
    figure->diffuse = kd;
    QWidget::update();
}

void RenderArea::setFigureSpecular(QVector3D ks)
{
    figure->specular = ks;
    QWidget::update();
}

void RenderArea::setFigureGloss(double p)
{
    figure->gloss = p;
    QWidget::update();
}

void RenderArea::setFaceVariant(FaceVariant newFaceVariant)
{
    faceVariant = newFaceVariant;
    QWidget::update();
}

void RenderArea::rotateX(double degree, bool silent)
{
    rotate.rotate(degree, {1, 0, 0});
    if (!silent) recalc();
}

void RenderArea::rotateY(double degree, bool silent)
{
    rotate.rotate(degree, {0, 1, 0});
    if (!silent) recalc();
}

void RenderArea::rotateZ(double degree, bool silent)
{
    rotate.rotate(degree, {0, 0, 1});
    if (!silent) recalc();
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
    if (!silent) recalc();
    emit shiftChanged(shift);
}

void RenderArea::setShift(const QMatrix4x4 &newShift)
{
    if (shift == newShift)
        return;
    shift = newShift;
    shift.optimize();
    recalc();
    emit shiftChanged(shift);
}

void RenderArea::setRotate(const QMatrix4x4 &newRotate)
{
    if (rotate == newRotate)
        return;
    rotate = newRotate;
    rotate.optimize();
    recalc();
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
    recalc();
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
