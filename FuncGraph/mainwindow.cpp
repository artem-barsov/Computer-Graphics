#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    margin.setWidth(this->width() - (ui->draw_widget->x() + ui->draw_widget->width()));
    margin.setHeight(this->height() - (ui->draw_widget->y() + ui->draw_widget->height()));
    graph = new Graph(ui->n_spinBox->value(),
                      ui->a_doubleSpinBox->value(),
                      ui->b_doubleSpinBox->value());
    ra = new RenderArea(ui->draw_widget,
                        graph,
                        { ui->scaleX_doubleSpinBox->value(),
                          ui->scaleY_doubleSpinBox->value() },
                        { ui->shiftX_spinBox->value(),
                          ui->shiftY_spinBox->value() },
                        ui->rotate_doubleSpinBox->value()
                        );
    ra_ratio = (double)ra->width() / ra->height();

    connect(ui->n_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            graph,         &Graph::setN);

    connect(ui->a_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            graph,               &Graph::setA);

    connect(ui->b_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            graph,               &Graph::setB);

    connect(ui->scaleX_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ra,
            [=](){ra->setScale(QTransform(ui->scaleX_doubleSpinBox->value(), 0, 0,
                                          ui->scaleY_doubleSpinBox->value(), 0, 0));});
    connect(ui->scaleY_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ra,
            [=](){ra->setScale(QTransform(ui->scaleX_doubleSpinBox->value(), 0, 0,
                                          ui->scaleY_doubleSpinBox->value(), 0, 0));});

    connect(ui->shiftX_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), ra,
            [=](){ra->setShift(QTransform(1, 0, 0, 1, ui->shiftX_spinBox->value(),
                                                      ui->shiftY_spinBox->value()));});
    connect(ui->shiftY_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), ra,
            [=](){ra->setShift(QTransform(1, 0, 0, 1, ui->shiftX_spinBox->value(),
                                                      ui->shiftY_spinBox->value()));});

    connect(ui->rotate_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            ra, [=](double grad){
        double rad = grad * M_PI / 180.0;
        double cosr = cos(rad), sinr = sin(rad);
        ra->setRotate(QTransform(cosr, -sinr, sinr, cosr, 0, 0));
    });

    connect(ra, &RenderArea::scaleChanged, ui->scaleX_doubleSpinBox,
            [=](QTransform scl){
        ui->scaleX_doubleSpinBox->blockSignals(true);
        ui->scaleX_doubleSpinBox->setValue(scl.m11());
        ui->scaleX_doubleSpinBox->blockSignals(false);
    });
    connect(ra, &RenderArea::scaleChanged, ui->scaleY_doubleSpinBox,
            [=](QTransform scl){
        ui->scaleY_doubleSpinBox->blockSignals(true);
        ui->scaleY_doubleSpinBox->setValue(scl.m22());
        ui->scaleY_doubleSpinBox->blockSignals(false);
    });

    connect(ra, &RenderArea::shiftChanged, ui->shiftX_spinBox,
            [=](QTransform shft){
        ui->shiftX_spinBox->blockSignals(true);
        ui->shiftX_spinBox->setValue(round(shft.dx()));
        ui->shiftX_spinBox->blockSignals(false);
    });
    connect(ra, &RenderArea::shiftChanged, ui->shiftY_spinBox,
            [=](QTransform shft){
        ui->shiftY_spinBox->blockSignals(true);
        ui->shiftY_spinBox->setValue(round(shft.dy()));
        ui->shiftY_spinBox->blockSignals(false);
    });

    connect(ra, &RenderArea::rotateChanged, ui->rotate_doubleSpinBox,
            [=](QTransform rot){ui->rotate_doubleSpinBox->setValue(
                    -atan2(rot.m12(), rot.m11()) * 180.0 / M_PI);});

    connect(ra,                 &RenderArea::debugRA,
            ui->debug_ra_label, &QLabel::setText);
    connect(ra,                 &RenderArea::debugSC,
            ui->debug_sc_label, &QLabel::setText);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    static int cnt = 0;
    cnt++;
    ui->debug_mw_label->setText("MA resize: " + QString::number(cnt));

    QSize newSize = size() - margin - QSize(ui->draw_widget->pos().x(),
                                            ui->draw_widget->pos().y());
    double newRatio = (double) newSize.width() / newSize.height();
    int newW, newH;
    if (newRatio > ra_ratio) {
        newW = newSize.height() * ra_ratio;
        newH = newSize.height();
    } else {
        newW = newSize.width();
        newH = newSize.width() / ra_ratio;
    }
    double ratio = (double) newW / ui->draw_widget->width();
    ui->draw_widget->resize(newW, newH);
    ra->resize(newW, newH);
    ra->setCenter({ newW / 2, newH / 2 });
//    auto qwe = ra->getScale() * ratio;
//    auto asd = ra->getShift() * ratio;
//    ra->setScale(qwe);
//    ra->setShift(asd);
//    ra->setScale(ra->getScale() * ratio);
    ra->setShift(ra->getShift() * ratio);

//    ui->scaleX_doubleSpinBox->setValue(
//                ui->scaleX_doubleSpinBox->value() * ratio);
//    ui->scaleY_doubleSpinBox->setValue(
//                ui->scaleY_doubleSpinBox->value() * ratio);
//    ui->shiftX_spinBox->setValue(
//                round(ui->shiftX_spinBox->value() * ratio));
//    ui->shiftY_spinBox->setValue(
//                round(ui->shiftY_spinBox->value() * ratio));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete graph;
    delete ra;
}

