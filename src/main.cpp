#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("智分Design");
    app.setApplicationVersion("3.1.0");
    app.setOrganizationName("智分Design");

    // 设置深色主题
    app.setStyle(QStyleFactory::create("Fusion"));

    MainWindow w;
    w.show();

    return app.exec();
}
