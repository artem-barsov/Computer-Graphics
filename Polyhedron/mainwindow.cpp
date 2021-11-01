#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // test
    margin.setWidth(this->width() - (ui->draw_widget->x() + ui->draw_widget->width()));
    margin.setHeight(this->height() - (ui->draw_widget->y() + ui->draw_widget->height()));
    ra = new RenderArea(ui->draw_widget);
    ra_ratio = (double) ra->width() / ra->height();
    this->setMinimumSize(ui->draw_widget->pos().x() + margin.width() + 120 * ra_ratio,
                         ui->draw_widget->pos().y() + margin.height() + 120);

    connect(ui->scaleX_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::scale_doubleSpinBoxChanged);
    connect(ui->scaleY_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::scale_doubleSpinBoxChanged);
    connect(ui->scaleZ_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::scale_doubleSpinBoxChanged);

    connect(this, &MainWindow::scale_doubleSpinBoxChanged, ra, [this]() {
        QMatrix4x4 E;
        E.scale(ui->scaleX_doubleSpinBox->value(),
                ui->scaleY_doubleSpinBox->value(),
                ui->scaleZ_doubleSpinBox->value());
        ra->setScale(E);
    });

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

    connect(ui->shiftX_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::shift_spinBoxChanged);
    connect(ui->shiftY_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::shift_spinBoxChanged);
    connect(ui->shiftZ_spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::shift_spinBoxChanged);

    connect(this, &MainWindow::shift_spinBoxChanged, ra, [this]() {
        QMatrix4x4 E;
        E.translate(ui->shiftX_spinBox->value(),
                    ui->shiftY_spinBox->value(),
                    ui->shiftZ_spinBox->value());
        ra->setShift(E);
    });

    connect(ui->figure_comboBox, &QComboBox::currentTextChanged,
            ra, [this](const QString& var) {
        if (var == "Cube")
            ra->setFigure(Polyhedron::GenerateCube());
        else if (var == "Pyramid")
            ra->setFigure(Polyhedron::GeneratePyramid());
    });

    connect(ui->none_radioButton, &QRadioButton::clicked,
            ra, [this](){ ra->setFaceVariant(
                        RenderArea::FaceVariant::NONE); });
    connect(ui->random_radioButton, &QRadioButton::clicked,
            ra, [this](){ ra->setFaceVariant(
                        RenderArea::FaceVariant::RANDOM); });
    connect(ui->default_radioButton, &QRadioButton::clicked,
            ra, [this](){ ra->setFaceVariant(
                        RenderArea::FaceVariant::DEFAULT); });

    connect(ui->wireframe_checkBox, &QCheckBox::clicked,
            ra, &RenderArea::setIsDrawWireframe);
    connect(ui->normals_checkBox, &QCheckBox::clicked,
            ra, &RenderArea::setIsDrawingNormals);
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

    connect(ra, &RenderArea::scaleChanged, this, [this](QMatrix4x4 sc) {
        ui->scaleX_doubleSpinBox->blockSignals(true);
        ui->scaleY_doubleSpinBox->blockSignals(true);
        ui->scaleZ_doubleSpinBox->blockSignals(true);
        ui->scaleX_doubleSpinBox->setValue(sc(0, 0));
        ui->scaleY_doubleSpinBox->setValue(sc(1, 1));
        ui->scaleZ_doubleSpinBox->setValue(sc(2, 2));
        ui->scaleY_doubleSpinBox->blockSignals(false);
        ui->scaleX_doubleSpinBox->blockSignals(false);
        ui->scaleZ_doubleSpinBox->blockSignals(false);
    });
    connect(ra, &RenderArea::shiftChanged, this, [this](QMatrix4x4 sh) {
        ui->shiftX_spinBox->blockSignals(true);
        ui->shiftY_spinBox->blockSignals(true);
        ui->shiftZ_spinBox->blockSignals(true);
        ui->shiftX_spinBox->setValue(sh(0, 3));
        ui->shiftY_spinBox->setValue(sh(1, 3));
        ui->shiftZ_spinBox->setValue(sh(2, 3));
        ui->shiftX_spinBox->blockSignals(false);
        ui->shiftY_spinBox->blockSignals(false);
        ui->shiftZ_spinBox->blockSignals(false);
    });

    connect(ra, &RenderArea::debug, this, [this](QMatrix4x4 mtx){
        for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++)
            ui->tableWidget->setItem(i, j, new QTableWidgetItem(
                                         QString::number(mtx(i, j))));
    });
    emit ui->figure_comboBox->currentTextChanged(
                ui->figure_comboBox->currentText());
    emit this->scale_doubleSpinBoxChanged();
    emit this->shift_spinBoxChanged();
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
