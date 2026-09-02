#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>
#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QThread>
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

        // 启动画面
        QPixmap splashPix(600, 350);
        splashPix.fill(QColor(30, 30, 30));
        QPainter painter(&splashPix);
        painter.setRenderHint(QPainter::Antialiasing);
        // 标题
        QFont titleFont("Microsoft YaHei", 28, QFont::Bold);
        painter.setFont(titleFont);
        painter.setPen(QColor(0, 150, 255));
        painter.drawText(splashPix.rect().adjusted(0, 60, 0, 0), Qt::AlignHCenter, "智分Design");
        // 副标题
        QFont subFont("Microsoft YaHei", 12);
        painter.setFont(subFont);
        painter.setPen(QColor(180, 180, 180));
        painter.drawText(splashPix.rect().adjusted(0, 110, 0, 0), Qt::AlignHCenter, "专业室分设计CAD软件  V3.1.0");
        // 分隔线
        painter.setPen(QPen(QColor(0, 120, 200), 2));
        painter.drawLine(150, 150, 450, 150);
        // 功能特性
        painter.setFont(QFont("Microsoft YaHei", 9));
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(splashPix.rect().adjusted(0, 170, 0, 0), Qt::AlignHCenter, "传统室分  |  数字化室分  |  漏缆  |  电梯覆盖  |  楼间对打");
        painter.drawText(splashPix.rect().adjusted(0, 195, 0, 0), Qt::AlignHCenter, "AI自动布放  |  智能校验  |  链路预算  |  材料统计  |  批量出图");
        // 版本信息
        painter.setFont(QFont("Arial", 8));
        painter.setPen(QColor(100, 100, 100));
        painter.drawText(splashPix.rect().adjusted(0, 280, 0, 0), Qt::AlignHCenter, "基于Qt5 Framework  |  C++17  |  64-bit");
        painter.drawText(splashPix.rect().adjusted(0, 300, 0, 0), Qt::AlignHCenter, "Copyright 2026 智分Design Team");
        painter.end();

        QSplashScreen splash(splashPix);
        splash.show();
        app.processEvents();
        qInfo() << "启动画面显示";

        MainWindow w;
        qInfo() << "主窗口创建成功";
        
        // 模拟加载进度
        for (int i = 0; i <= 100; i += 25) {
            splash.showMessage(QString("正在加载... %1%").arg(i), Qt::AlignBottom | Qt::AlignHCenter, QColor(200, 200, 200));
            app.processEvents();
            QThread::msleep(50);
        }
        
        w.show();
        splash.finish(&w);
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
