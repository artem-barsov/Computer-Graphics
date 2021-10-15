#include "graph.h"

Graph::Graph(int n, double a, double b)
    : _n(n), _a(a), _b(b), _points(QVector<QPointF>(n))
{
    _recalc();
}

void Graph::_recalc()
{
    const double step = 2 * acos(-1) / n();
    double t = 0;
    for (int i = 0; i < n(); i++, t += step) {
        _points[i].setX(a() * cos(t));
        _points[i].setY(b() * sin(t));
    }
}

const QVector<QPointF> &Graph::points() const
{
    return _points;
}

int Graph::n() const
{
    return _n;
}

void Graph::setN(int newN)
{
    if (_n == newN)
        return;
    _n = newN;
    _points.resize(_n);
    _recalc();
    emit nChanged();
}

double Graph::a() const
{
    return _a;
}

void Graph::setA(double newA)
{
    if (qFuzzyCompare(_a, newA))
        return;
    _a = newA;
    _recalc();
    emit aChanged();
}

double Graph::b() const
{
    return _b;
}

void Graph::setB(double newB)
{
    if (qFuzzyCompare(_b, newB))
        return;
    _b = newB;
    _recalc();
    emit bChanged();
}
