#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    margin.setWidth(this->width() - (ui->draw_widget->x() + ui->draw_widget->width()));
    margin.setHeight(this->height() - (ui->draw_widget->y() + ui->draw_widget->height()));
    ra = new RenderArea(ui->draw_widget);
    ra_ratio = (double) ra->width() / ra->height();

    connect(ui->scaleX_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::scale_doubleSpinBoxChanged);
    connect(ui->scaleY_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::scale_doubleSpinBoxChanged);
    connect(ui->scaleZ_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::scale_doubleSpinBoxChanged);

    connect(this, &MainWindow::scale_doubleSpinBoxChanged, ra, [=]() {
        QMatrix4x4 E;
        E.scale(ui->scaleX_doubleSpinBox->value(),
                ui->scaleY_doubleSpinBox->value(),
                ui->scaleZ_doubleSpinBox->value());
        ra->setScale(E);
    });

    connect(ui->rotateX_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            ra, [=](double grad) { QMatrix4x4 E; E.rotate(grad, 1, 0, 0); ra->setRotateX(E); });
    connect(ui->rotateY_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            ra, [=](double grad) { QMatrix4x4 E; E.rotate(grad, 0, 1, 0); ra->setRotateY(E); });
    connect(ui->rotateZ_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            ra, [=](double grad) { QMatrix4x4 E; E.rotate(grad, 0, 0, 1); ra->setRotateZ(E); });

    connect(ui->shiftX_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::shift_spinBoxChanged);
    connect(ui->shiftY_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::shift_spinBoxChanged);
    connect(ui->shiftZ_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::shift_spinBoxChanged);

    connect(this, &MainWindow::shift_spinBoxChanged, ra, [=]() {
        QMatrix4x4 E;
        E.translate(ui->shiftX_spinBox->value(),
                    ui->shiftY_spinBox->value(),
                    ui->shiftZ_spinBox->value());
        ra->setShift(E);
    });

//    connect(ra, &RenderArea::rotateXChanged, ui->rotateX_doubleSpinBox,
//            [=](QMatrix4x4 rotX){
//        ui->rotateX_doubleSpinBox->blockSignals(true);
//        ui->rotateX_doubleSpinBox->setValue(
//                    -atan2(rotX(2, 3), rotX(2, 2)) * 180.0 / M_PI);
//        ui->rotateX_doubleSpinBox->blockSignals(false);
//    });
//    connect(ra, &RenderArea::rotateYChanged, ui->rotateY_doubleSpinBox,
//            [=](QMatrix4x4 rotY){
//        ui->rotateY_doubleSpinBox->blockSignals(true);
//        ui->rotateY_doubleSpinBox->setValue(
//                    atan2(rotY(1, 3), rotY(1, 1)) * 180.0 / M_PI);
//        ui->rotateY_doubleSpinBox->blockSignals(false);
//    });


    emit this->scale_doubleSpinBoxChanged();
    emit this->shift_spinBoxChanged();
    emit ui->rotateX_doubleSpinBox->valueChanged(ui->rotateX_doubleSpinBox->value());
    emit ui->rotateY_doubleSpinBox->valueChanged(ui->rotateY_doubleSpinBox->value());
    emit ui->rotateZ_doubleSpinBox->valueChanged(ui->rotateZ_doubleSpinBox->value());
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    QSize newSize = size() - margin - QSize(ui->draw_widget->pos().x(),
                                            ui->draw_widget->pos().y());
    double newRatio = (double)newSize.width() / newSize.height();
    int newW, newH;
    if (newRatio > ra_ratio) {
        newW = newSize.height() * ra_ratio;
        newH = newSize.height();
    } else {
        newW = newSize.width();
        newH = newSize.width() / ra_ratio;
    }
    ui->draw_widget->resize(newW, newH);
    ra->resize(newW, newH);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete ra;
}

