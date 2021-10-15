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
            [=](QTransform scl){ui->scaleX_doubleSpinBox->setValue(scl.m11());});
    connect(ra, &RenderArea::scaleChanged, ui->scaleY_doubleSpinBox,
            [=](QTransform scl){ui->scaleY_doubleSpinBox->setValue(scl.m22());});

    connect(ra, &RenderArea::shiftChanged, ui->shiftX_spinBox,
            [=](QTransform shft){ui->shiftX_spinBox->setValue(shft.dx());});
    connect(ra, &RenderArea::shiftChanged, ui->shiftY_spinBox,
            [=](QTransform shft){ui->shiftY_spinBox->setValue(shft.dy());});

    connect(ra, &RenderArea::rotateChanged, ui->rotate_doubleSpinBox,
            [=](QTransform rot){ui->rotate_doubleSpinBox->setValue(
                    -atan2(rot.m12(), rot.m11()) * 180.0 / M_PI);});
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    QSize newSize = size() - margin - QSize(ui->draw_widget->pos().x(),
                                            ui->draw_widget->pos().y());
    if (newSize.width() <= 0 || newSize.height() <= 0)
        this->resize(1, 1);
    double newRatio = (double)newSize.width() / newSize.height();
    if (newRatio > ra_ratio) {
        ra->resize(newSize.height() * ra_ratio,
                   newSize.height());
    } else {
        ra->resize(newSize.width(),
                   newSize.width() / ra_ratio);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete graph;
    delete ra;
}

