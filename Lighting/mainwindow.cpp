#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "primitives.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    shadingButtonGroup = new QButtonGroup();
    paintingButtonGroup = new QButtonGroup();
    shadingButtonGroup->addButton(ui->shadingFlat_radioButton);
    shadingButtonGroup->addButton(ui->shadingGourand_radioButton);
    paintingButtonGroup->addButton(ui->default_radioButton);
    paintingButtonGroup->addButton(ui->none_radioButton);
    paintingButtonGroup->addButton(ui->random_radioButton);
    margin.setWidth(this->width() - (ui->draw_widget->x() + ui->draw_widget->width()));
    margin.setHeight(this->height() - (ui->draw_widget->y() + ui->draw_widget->height()));
    ra = new RenderArea(ui->draw_widget);
    ra_ratio = (double) ra->width() / ra->height();
    this->setMinimumSize(ui->draw_widget->pos().x() + margin.width() + 120 * ra_ratio,
                         ui->draw_widget->pos().y() + margin.height() + 120);
    // setup renderarea
    {
        ra->setFigure(new ConeMesh(ui->coneR1_doubleSpinBox->value(),
                                   ui->coneR2_doubleSpinBox->value(),
                                   ui->coneH_doubleSpinBox->value(),
                                   ui->coneRatio_doubleSpinBox->value(),
                                   ui->horApr_spinBox->value(),
                                   ui->verApr_spinBox->value(),
                                   ui->radApr_spinBox->value()));
        ra->setFigureAmbient(QVector3D(ui->kaR_doubleSpinBox->value(),
                                       ui->kaG_doubleSpinBox->value(),
                                       ui->kaB_doubleSpinBox->value()));
        ra->setFigureDiffuse(QVector3D(ui->kdR_doubleSpinBox->value(),
                                       ui->kdG_doubleSpinBox->value(),
                                       ui->kdB_doubleSpinBox->value()));
        ra->setFigureSpecular(QVector3D(ui->ksR_doubleSpinBox->value(),
                                        ui->ksG_doubleSpinBox->value(),
                                        ui->ksB_doubleSpinBox->value()));
        ra->setFigureGloss(ui->gloss_doubleSpinBox->value());
        ra->setLighterDistance(ui->lighterR_doubleSpinBox->value());
        ra->setLighterAmbient(QVector3D(ui->iaR_doubleSpinBox->value(),
                                        ui->iaG_doubleSpinBox->value(),
                                        ui->iaB_doubleSpinBox->value()));
        ra->setLighterIntensity(QVector3D(ui->ilR_doubleSpinBox->value(),
                                          ui->ilG_doubleSpinBox->value(),
                                          ui->ilB_doubleSpinBox->value()));
        ra->setScale(scalingMtrx(ui->scaleX_doubleSpinBox->value(),
                                 ui->scaleY_doubleSpinBox->value(),
                                 ui->scaleZ_doubleSpinBox->value()));
        ra->setShift(shiftingMtrx(ui->shiftX_spinBox->value(),
                                  ui->shiftY_spinBox->value(),
                                  ui->shiftZ_spinBox->value()));
        ra->setLighterMd(ui->lighterMd_doubleSpinBox->value());
        ra->setLighterMk(ui->lighterMk_doubleSpinBox->value());
        ra->setFaceVariant( ui->none_radioButton->isChecked()   ? RenderArea::FaceVariant::NONE
                          : ui->random_radioButton->isChecked() ? RenderArea::FaceVariant::RANDOM
                                                                : RenderArea::FaceVariant::DEFAULT);
        ra->setShadingVariant(ui->shadingFlat_radioButton->isChecked() ? RenderArea::ShadingVariant::FLAT
                                                                       : RenderArea::ShadingVariant::GOURAND);
        ra->setIsDrawWireframe(ui->wireframe_checkBox->isChecked());
        ra->setIsPolygonNormals(ui->polygonNormals_checkBox->isChecked());
        ra->setIsVertexNormals(ui->vertexNormals_checkBox->isChecked());
        ra->setIsNormalMethodEnabled(ui->normalMethod_checkBox->isChecked());
        ra->setIsZSortingEnabled(ui->ZSorting_checkBox->isChecked());
    }
    // scale spinboxes
    {
        connect(ui->scaleX_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &MainWindow::scale_doubleSpinBoxChanged);
        connect(ui->scaleY_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &MainWindow::scale_doubleSpinBoxChanged);
        connect(ui->scaleZ_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &MainWindow::scale_doubleSpinBoxChanged);
        connect(this, &MainWindow::scale_doubleSpinBoxChanged, ra, [this]() {
            ra->setScale(scalingMtrx(ui->scaleX_doubleSpinBox->value(),
                                     ui->scaleY_doubleSpinBox->value(),
                                     ui->scaleZ_doubleSpinBox->value()));
        });
        connect(ra, &RenderArea::scaleChanged, this, [this](QMatrix4x4 sc) {
            ui->scaleX_doubleSpinBox->setValue(sc(0, 0));
            ui->scaleY_doubleSpinBox->setValue(sc(1, 1));
            ui->scaleZ_doubleSpinBox->setValue(sc(2, 2));
        });
    }
    // rotate buttons
    {
        connect(ui->rotateX_plus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->rotateX(ui->rotateX_doubleSpinBox->value()); });
        connect(ui->rotateX_minus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->rotateX(-ui->rotateX_doubleSpinBox->value()); });
        connect(ui->rotateY_plus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->rotateY(ui->rotateY_doubleSpinBox->value()); });
        connect(ui->rotateY_minus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->rotateY(-ui->rotateY_doubleSpinBox->value()); });
        connect(ui->rotateZ_plus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->rotateZ(ui->rotateZ_doubleSpinBox->value()); });
        connect(ui->rotateZ_minus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->rotateZ(-ui->rotateZ_doubleSpinBox->value()); });
    }
    // shift spinboxes
    {
        connect(ui->shiftX_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::shift_spinBoxChanged);
        connect(ui->shiftY_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::shift_spinBoxChanged);
        connect(ui->shiftZ_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::shift_spinBoxChanged);
        connect(this, &MainWindow::shift_spinBoxChanged, ra, [this]() {
            ra->setShift(shiftingMtrx(ui->shiftX_spinBox->value(),
                                      ui->shiftY_spinBox->value(),
                                      ui->shiftZ_spinBox->value()));
        });
        connect(ra, &RenderArea::shiftChanged, this, [this](QMatrix4x4 sh) {
            ui->shiftX_spinBox->setValue(sh(0, 3));
            ui->shiftY_spinBox->setValue(sh(1, 3));
            ui->shiftZ_spinBox->setValue(sh(2, 3));
        });
    }
    // cone params
    {
        connect(ui->coneR1_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &MainWindow::coneChanged);
        connect(ui->coneR2_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &MainWindow::coneChanged);
        connect(ui->coneH_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &MainWindow::coneChanged);
        connect(ui->coneRatio_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &MainWindow::coneChanged);
        connect(ui->horApr_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::coneChanged);
        connect(ui->verApr_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::coneChanged);
        connect(ui->radApr_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::coneChanged);
        connect(this, &MainWindow::coneChanged, ra, [this](){
            if (ui->figure_comboBox->currentText() != "Cone") return;
            ra->setFigure(new ConeMesh(ui->coneR1_doubleSpinBox->value(),
                                       ui->coneR2_doubleSpinBox->value(),
                                       ui->coneH_doubleSpinBox->value(),
                                       ui->coneRatio_doubleSpinBox->value(),
                                       ui->horApr_spinBox->value(),
                                       ui->verApr_spinBox->value(),
                                       ui->radApr_spinBox->value()));
            ra->setFigureAmbient(QVector3D(ui->kaR_doubleSpinBox->value(),
                                           ui->kaG_doubleSpinBox->value(),
                                           ui->kaB_doubleSpinBox->value()));
            ra->setFigureDiffuse(QVector3D(ui->kdR_doubleSpinBox->value(),
                                           ui->kdG_doubleSpinBox->value(),
                                           ui->kdB_doubleSpinBox->value()));
            ra->setFigureSpecular(QVector3D(ui->ksR_doubleSpinBox->value(),
                                            ui->ksG_doubleSpinBox->value(),
                                            ui->ksB_doubleSpinBox->value()));
            ra->setFigureGloss(ui->gloss_doubleSpinBox->value());
        });
    }
    // figure params
    {
        connect(ui->figure_comboBox, &QComboBox::currentTextChanged,
                ra, [this](const QString& var) {
            if (var == "Cone")
                emit coneChanged();
            else if (var == "Cube")
                ra->setFigure(new Cube());
            else if (var == "Pyramid")
                ra->setFigure(new Pyramid());
            ra->setFigureAmbient(QVector3D(ui->kaR_doubleSpinBox->value(),
                                           ui->kaG_doubleSpinBox->value(),
                                           ui->kaB_doubleSpinBox->value()));
            ra->setFigureDiffuse(QVector3D(ui->kdR_doubleSpinBox->value(),
                                           ui->kdG_doubleSpinBox->value(),
                                           ui->kdB_doubleSpinBox->value()));
            ra->setFigureSpecular(QVector3D(ui->ksR_doubleSpinBox->value(),
                                            ui->ksG_doubleSpinBox->value(),
                                            ui->ksB_doubleSpinBox->value()));
            ra->setFigureGloss(ui->gloss_doubleSpinBox->value());
        });
        connect(ui->kaR_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setFigureAmbient(QVector3D(ui->kaR_doubleSpinBox->value(),
                                                             ui->kaG_doubleSpinBox->value(),
                                                             ui->kaB_doubleSpinBox->value()));});
        connect(ui->kaG_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setFigureAmbient(QVector3D(ui->kaR_doubleSpinBox->value(),
                                                             ui->kaG_doubleSpinBox->value(),
                                                             ui->kaB_doubleSpinBox->value()));});
        connect(ui->kaB_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setFigureAmbient(QVector3D(ui->kaR_doubleSpinBox->value(),
                                                             ui->kaG_doubleSpinBox->value(),
                                                             ui->kaB_doubleSpinBox->value()));});
        connect(ui->kdR_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setFigureDiffuse(QVector3D(ui->kdR_doubleSpinBox->value(),
                                                             ui->kdG_doubleSpinBox->value(),
                                                             ui->kdB_doubleSpinBox->value()));});
        connect(ui->kdG_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setFigureDiffuse(QVector3D(ui->kdR_doubleSpinBox->value(),
                                                             ui->kdG_doubleSpinBox->value(),
                                                             ui->kdB_doubleSpinBox->value()));});
        connect(ui->kdB_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setFigureDiffuse(QVector3D(ui->kdR_doubleSpinBox->value(),
                                                             ui->kdG_doubleSpinBox->value(),
                                                             ui->kdB_doubleSpinBox->value()));});
        connect(ui->ksR_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setFigureSpecular(QVector3D(ui->ksR_doubleSpinBox->value(),
                                                              ui->ksG_doubleSpinBox->value(),
                                                              ui->ksB_doubleSpinBox->value()));});
        connect(ui->ksG_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setFigureSpecular(QVector3D(ui->ksR_doubleSpinBox->value(),
                                                              ui->ksG_doubleSpinBox->value(),
                                                              ui->ksB_doubleSpinBox->value()));});
        connect(ui->ksB_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setFigureSpecular(QVector3D(ui->ksR_doubleSpinBox->value(),
                                                              ui->ksG_doubleSpinBox->value(),
                                                              ui->ksB_doubleSpinBox->value()));});
        connect(ui->gloss_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, &RenderArea::setFigureGloss);
    }
    // painting params
    {
        connect(ui->none_radioButton, &QRadioButton::clicked,
                ra, [this](){ ra->setFaceVariant(
                            RenderArea::FaceVariant::NONE); });
        connect(ui->random_radioButton, &QRadioButton::clicked,
                ra, [this](){ ra->setFaceVariant(
                            RenderArea::FaceVariant::RANDOM); });
        connect(ui->default_radioButton, &QRadioButton::clicked,
                ra, [this](){ ra->setFaceVariant(
                            RenderArea::FaceVariant::DEFAULT); });
        connect(ui->shadingFlat_radioButton, &QRadioButton::clicked,
                ra, [this](){ ra->setShadingVariant(
                            RenderArea::ShadingVariant::FLAT); });
        connect(ui->shadingGourand_radioButton, &QRadioButton::clicked,
                ra, [this](){ ra->setShadingVariant(
                            RenderArea::ShadingVariant::GOURAND); });
        connect(ui->wireframe_checkBox, &QCheckBox::clicked,
                ra, &RenderArea::setIsDrawWireframe);
        connect(ui->polygonNormals_checkBox, &QCheckBox::clicked,
                ra, &RenderArea::setIsPolygonNormals);
        connect(ui->vertexNormals_checkBox, &QCheckBox::clicked,
                ra, &RenderArea::setIsVertexNormals);
        connect(ui->normalMethod_checkBox, &QCheckBox::clicked,
                ra, &RenderArea::setIsNormalMethodEnabled);
        connect(ui->ZSorting_checkBox, &QCheckBox::clicked,
                ra, &RenderArea::setIsZSortingEnabled);
        connect(ui->ortho_radioButton, &QRadioButton::clicked,
                ra, &RenderArea::setOrthoView);
        connect(ui->front_radioButton, &QRadioButton::clicked,
                ra, &RenderArea::setFrontView);
        connect(ui->side_radioButton, &QRadioButton::clicked,
                ra, &RenderArea::setSideView);
        connect(ui->top_radioButton, &QRadioButton::clicked,
                ra, &RenderArea::setTopView);
        connect(ui->isometry_pushButton, &QRadioButton::clicked,
                ra, &RenderArea::positIsometric);
    }
    // debug info
    {
        connect(ra, &RenderArea::debug, this, [this](QMatrix4x4 mtx){
            for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++)
                ui->tableWidget->setItem(i, j, new QTableWidgetItem(
                                             QString::number(mtx(i, j))));
        });
    }
    // lighter params
    {
        connect(ui->lighter_rotateX_plus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->lighter_rotateX(ui->lighter_rotateX_doubleSpinBox->value()); });
        connect(ui->lighter_rotateX_minus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->lighter_rotateX(-ui->lighter_rotateX_doubleSpinBox->value()); });
        connect(ui->lighter_rotateY_plus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->lighter_rotateY(ui->lighter_rotateY_doubleSpinBox->value()); });
        connect(ui->lighter_rotateY_minus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->lighter_rotateY(-ui->lighter_rotateY_doubleSpinBox->value()); });
        connect(ui->lighter_rotateZ_plus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->lighter_rotateZ(ui->lighter_rotateZ_doubleSpinBox->value()); });
        connect(ui->lighter_rotateZ_minus_pushButton, &QPushButton::clicked,
                ra, [this](){ ra->lighter_rotateZ(-ui->lighter_rotateZ_doubleSpinBox->value()); });
        connect(ui->lighterR_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, &RenderArea::setLighterDistance);
        connect(ra, &RenderArea::LighterDistanceChanged,
                ui->lighterR_doubleSpinBox, &QDoubleSpinBox::setValue);
        connect(ui->iaR_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setLighterAmbient(QVector3D(ui->iaR_doubleSpinBox->value(),
                                                              ui->iaG_doubleSpinBox->value(),
                                                              ui->iaB_doubleSpinBox->value()));});
        connect(ui->iaG_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setLighterAmbient(QVector3D(ui->iaR_doubleSpinBox->value(),
                                                              ui->iaG_doubleSpinBox->value(),
                                                              ui->iaB_doubleSpinBox->value()));});
        connect(ui->iaB_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setLighterAmbient(QVector3D(ui->iaR_doubleSpinBox->value(),
                                                              ui->iaG_doubleSpinBox->value(),
                                                              ui->iaB_doubleSpinBox->value()));});
        connect(ui->ilR_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setLighterIntensity(QVector3D(ui->ilR_doubleSpinBox->value(),
                                                                ui->ilG_doubleSpinBox->value(),
                                                                ui->ilB_doubleSpinBox->value()));});
        connect(ui->ilG_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setLighterIntensity(QVector3D(ui->ilR_doubleSpinBox->value(),
                                                                ui->ilG_doubleSpinBox->value(),
                                                                ui->ilB_doubleSpinBox->value()));});
        connect(ui->ilB_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, [this](){ ra->setLighterIntensity(QVector3D(ui->ilR_doubleSpinBox->value(),
                                                                ui->ilG_doubleSpinBox->value(),
                                                                ui->ilB_doubleSpinBox->value()));});
        connect(ui->lighterMd_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, &RenderArea::setLighterMd);
        connect(ui->lighterMk_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                ra, &RenderArea::setLighterMk);
    }
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
    delete shadingButtonGroup;
    delete paintingButtonGroup;
}
