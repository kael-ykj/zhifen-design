#include "batch_rename_plugin.h"
#include "core_api.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>
#include <QWidget>

namespace Zhifen {

BatchRenamePlugin::BatchRenamePlugin() {}
BatchRenamePlugin::~BatchRenamePlugin() {}

bool BatchRenamePlugin::initialize(CoreApi *api) {
    m_api = api;
    return true;
}

void BatchRenamePlugin::execute() {
    if (!m_api) return;

    // 获取选中的器件
    QList<DeviceItem*> devices = m_api->getSelectedDevices();
    if (devices.isEmpty()) {
        devices = m_api->getAllDevices();
    }
    if (devices.isEmpty()) {
        QMessageBox::warning(nullptr, "批量重命名", "没有找到器件");
        return;
    }

    // 获取前缀
    bool ok;
    QString prefix = QInputDialog::getText(nullptr, "批量重命名", "命名前缀:", QLineEdit::Normal, m_prefix, &ok);
    if (!ok || prefix.isEmpty()) return;

    // 获取起始编号
    int startIndex = QInputDialog::getInt(nullptr, "批量重命名", "起始编号:", m_startIndex, 1, 9999, 1, &ok);
    if (!ok) return;

    m_prefix = prefix;
    m_startIndex = startIndex;

    // 执行重命名
    int count = m_api->renameDevices(devices, prefix, startIndex);

    QMessageBox::information(nullptr, "批量重命名",
        QString("已成功重命名 %1 个器件\\n格式: %2-%3 到 %2-%4")
        .arg(count).arg(prefix).arg(startIndex, 3, 10, QChar('0')).arg(startIndex + count - 1, 3, 10, QChar('0')));

    m_api->log(QString("批量重命名: %1个器件, 前缀%2").arg(count).arg(prefix));
}

void BatchRenamePlugin::unload() {
    m_api = nullptr;
}

} // namespace Zhifen
