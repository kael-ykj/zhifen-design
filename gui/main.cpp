#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("智分Design");
    app.setApplicationVersion("3.1.0");
    app.setOrganizationName("ZhiFen");

    MainWindow w;
    w.show();
    return app.exec();
}
