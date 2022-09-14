#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QButtonGroup>
#include <QMainWindow>
#include "renderarea.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void scale_doubleSpinBoxChanged();
    void shift_spinBoxChanged();
    void coneChanged();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    RenderArea *ra;
    QSize margin;
    double ra_ratio;
    QButtonGroup *paintingButtonGroup;
};
#endif // MAINWINDOW_H
