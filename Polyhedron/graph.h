#ifndef GRAPH_H
#define GRAPH_H

#include <QObject>
#include <QPainter>
#include <QPaintEvent>
#include <cmath>

class Graph : public QObject
{
    Q_OBJECT
public:
    Graph(int n, double a, double b);
    int n() const;
    double a() const;
    double b() const;
    void setN(int newN);
    void setA(double newA);
    void setB(double newB);

    const QVector<QPointF> &points() const;

signals:
    void nChanged();
    void aChanged();
    void bChanged();

private:
    void _recalc();

    int _n;
    double _a, _b;
    Q_PROPERTY(int n READ n WRITE setN NOTIFY nChanged)
    Q_PROPERTY(double a READ a WRITE setA NOTIFY aChanged)
    Q_PROPERTY(double b READ b WRITE setB NOTIFY bChanged)
    QVector<QPointF> _points;
};

#endif // GRAPH_H
