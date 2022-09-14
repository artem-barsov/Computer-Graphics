#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    MainWindow w;
//    w.showMaximized();
#ifndef QT_NO_OPENGL
    w.show();
#else
    QLabel note("OpenGL Support required");
    note.show();
#endif
    return a.exec();
}
