#include "renderarea.h"

RenderArea::RenderArea(QWidget *parent) : QOpenGLWidget(parent)
{
    QOpenGLWidget::resize(parent->size());
    recalc();
}

void RenderArea::recalc()
{
    point_WorldTrans = shift * rotate.transposed() * scale * point_viewport;
    vector_WorldTrans = NormalVecTransf(point_WorldTrans);
//    QWidget::update();
    QOpenGLWidget::update();
    emit debug(point_WorldTrans);
}

const QPoint RenderArea::getCenter() const
{
    return { width() / 2, height() / 2 };
}
/*
void RenderArea::paintEvent(QPaintEvent*)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::GlobalColor::gray);
    painter.drawRect(0, 0, width()-1, height()-1);

    // transform
    for (auto& p : figure->polygons)
        p->normal_world = vector_WorldTrans * p->normal_local;
    for (auto& v : figure->vertices) {
        v->point_world  = point_WorldTrans * v->point_local;
        v->normal_world = vector_WorldTrans * v->normal_local;
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
    plotFigure(painter);
    painter.translate(-getCenter());

    painter.end();
}*/

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
        rotateZ(-delta * 180.0 / M_PI);
        break;
    }
    case Qt::ShiftModifier: {
        doShift(0.02 * delta.x(), -0.02 * delta.y(), 0);
        break;
    }
    default: {
        rotateX(-delta.y(), true);
        rotateY(-delta.x());
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
    QMatrix4x4 E;
    E.scale(delta.y());
    E(3, 3) = 0;
    setScale(scale + E * 0.003);
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

/*
void RenderArea::plotFigure(QPainter &painter)
{
    for (const auto& p : qAsConst(figure->polygons)) {
        if (isNormalMethodEnabled && p->normal_world.z() >= 0) continue;
        QVector<QPoint> proj;
        for (const auto& v : qAsConst(p->vertices))
            proj.push_back(v->point_world.toPoint());
        QBrush faceBrush;
        if (faceVariant == NONE)
            faceBrush = Qt::BrushStyle::NoBrush;
        else {
            QColor clr = (faceVariant == RANDOM ? p->color : 0x00FF0000);
            faceBrush = QColor(clr.red(), clr.green(), clr.blue());
        }
        painter.setBrush(faceBrush);
        painter.setPen(isDrawingWireframe ? Qt::GlobalColor::black
                                          : faceBrush.color());
        painter.drawPolygon(proj);
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
}*/

void RenderArea::setIsVertexNormals(bool newIsVertexNormals)
{
    isVertexNormals = newIsVertexNormals;
//    QWidget::update();
    QOpenGLWidget::update();
}

void RenderArea::setIsZSortingEnabled(bool newIsZSortingEnabled)
{
    isZSortingEnabled = newIsZSortingEnabled;
//    QWidget::update();
    QOpenGLWidget::update();
}

void RenderArea::setIsNormalMethodEnabled(bool newIsNormalMethodEnabled)
{
    isNormalMethodEnabled = newIsNormalMethodEnabled;
//    QWidget::update();
    QOpenGLWidget::update();
}

void RenderArea::setIsPolygonNormals(bool newIsPolygonNormals)
{
    isPolygonNormals = newIsPolygonNormals;
//    QWidget::update();
    QOpenGLWidget::update();
}

void RenderArea::setIsDrawWireframe(bool newIsDrawWireframe)
{
    isDrawingWireframe = newIsDrawWireframe;
//    QWidget::update();
    QOpenGLWidget::update();
}

void RenderArea::setSideView()
{
    point_viewport = { 0,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    QMatrix4x4 E;
    E.rotate(-90, {0, 1, 0});
    setRotate(E);
    recalc();
}

void RenderArea::setFrontView()
{
    point_viewport = { 1,0,0,0, 0,1,0,0, 0,0,0,0, 0,0,0,1 };
    setRotate({});
    recalc();
}

void RenderArea::setTopView()
{
    point_viewport = { 1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1 };
    QMatrix4x4 E;
    E.rotate(-90, {1, 0, 0});
    setRotate(E);
    recalc();
}

void RenderArea::setOrthoView()
{
    point_viewport = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    recalc();
}

void RenderArea::positIsometric()
{
    QMatrix4x4 ret;
    ret.rotate(-45, {0, 1, 0});
    ret.rotate(-35, {1, 0, 0});
    setRotate(ret);
}

void RenderArea::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);
    if (!shaders.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vert_shader.vsh"))
        close();
    if (!shaders.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/frag_shader.fsh"))
        close();
    if (!shaders.link())
        close();
    if (!shaders.bind())
        close();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
//    setFigure<Pyramid>();
    setFigure<Cube>();
//    figure = std::unique_ptr<Polyhedron>(new Cube(shaders));
//    figure = std::make_unique<Cube>(shaders);
}

void RenderArea::resizeGL(int w, int h)
{
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    const qreal zNear = 3.0, zFar = 7.0, fov = 45.0;
    perspect_proj.setToIdentity();
    perspect_proj.perspective(fov, aspect, zNear, zFar);
}

void RenderArea::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaders.setUniformValue("qt_ModelViewProjectionMatrix", perspect_proj * point_WorldTrans);
    figure->drawPolyhedron();
//    if (figure)
//        glDrawElements(GL_TRIANGLE_STRIP, figure->indices.size(), GL_UNSIGNED_INT, figure->indices.data());
}

void RenderArea::setFaceVariant(FaceVariant newFaceVariant)
{
    faceVariant = newFaceVariant;
//    QWidget::update();
    QOpenGLWidget::update();
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

void RenderArea::doShift(double x, double y, double z, bool silent)
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
//    QWidget::resize(w, h);
    QOpenGLWidget::resize(w, h);
    QMatrix4x4 newScale = scale;
    newScale.scale(ratio);
    setScale(newScale);
    QMatrix4x4 newShift = shift * ratio;
    for (int i = 0; i < 4; i++)
        newShift(i, i) = shift(i, i);
    setShift(newShift);
}
