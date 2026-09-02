#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>
#include <QDir>

// 全局日志文件
QFile *g_logFile = nullptr;

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    if (!g_logFile) return;
    
    QTextStream out(g_logFile);
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString level;
    switch (type) {
    case QtDebugMsg: level = "DEBUG"; break;
    case QtInfoMsg: level = "INFO"; break;
    case QtWarningMsg: level = "WARNING"; break;
    case QtCriticalMsg: level = "CRITICAL"; break;
    case QtFatalMsg: level = "FATAL"; break;
    }
    out << "[" << time << "] [" << level << "] " << msg << "\n";
    out.flush();
}

int main(int argc, char *argv[])
{
    // 创建日志目录
    QString logDir = QDir::homePath() + "/.zhifen-design";
    QDir().mkpath(logDir);
    
    // 打开日志文件
    g_logFile = new QFile(logDir + "/startup.log");
    if (g_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qInstallMessageHandler(customMessageHandler);
        qInfo() << "=== 智分Design V3.1 启动 ===";
        qInfo() << "Qt版本:" << qVersion();
        qInfo() << "应用路径:" << QCoreApplication::applicationDirPath();
    }

    try {
        QApplication app(argc, argv);
        app.setApplicationName("智分Design");
        app.setApplicationVersion("3.1.0");
        app.setOrganizationName("智分Design");

        // 设置深色主题
        app.setStyle(QStyleFactory::create("Fusion"));
        qInfo() << "Fusion主题设置成功";

        // 检查平台插件
        QString pluginPath = QCoreApplication::applicationDirPath() + "/platforms";
        qInfo() << "平台插件路径:" << pluginPath << "存在:" << QDir(pluginPath).exists();

        MainWindow w;
        qInfo() << "主窗口创建成功";
        w.show();
        qInfo() << "主窗口显示成功";

        int ret = app.exec();
        qInfo() << "程序退出，返回码:" << ret;
        return ret;
    } catch (const std::exception &e) {
        qCritical() << "程序异常:" << e.what();
        QMessageBox::critical(nullptr, "启动失败", 
            QString("程序启动时发生异常:\n%1\n\n日志文件位置:\n%2")
            .arg(e.what()).arg(logDir + "/startup.log"));
        return 1;
    } catch (...) {
        qCritical() << "程序发生未知异常";
        QMessageBox::critical(nullptr, "启动失败", 
            QString("程序启动时发生未知异常\n\n日志文件位置:\n%1")
            .arg(logDir + "/startup.log"));
        return 1;
    }
}
