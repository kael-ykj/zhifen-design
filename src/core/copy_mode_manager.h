#ifndef COPY_MODE_MANAGER_H
#define COPY_MODE_MANAGER_H

#include <QString>
#include <QObject>

namespace Zhifen {

// 复制模式
enum CopyMode {
    LIGHT_COPY = 0,  // 轻量复制：不携带小区、信源、备注等脏参数
    FULL_COPY = 1    // 完整复制：继承全部工程参数
};

// 复制模式管理器（单例）
class CopyModeManager : public QObject
{
    Q_OBJECT
public:
    static CopyModeManager& instance() {
        static CopyModeManager mgr;
        return mgr;
    }

    CopyMode mode() const { return m_mode; }
    void setMode(CopyMode mode) {
        if (m_mode != mode) {
            m_mode = mode;
            emit modeChanged(m_mode);
        }
    }

    QString modeName() const {
        return m_mode == LIGHT_COPY ? "轻量复制" : "完整复制";
    }

    QString modeDescription() const {
        return m_mode == LIGHT_COPY
            ? "轻量复制：不携带小区、信源、备注等工程参数"
            : "完整复制：继承全部工程参数（小区、信源、备注等）";
    }

    bool isLightCopy() const { return m_mode == LIGHT_COPY; }
    bool isFullCopy() const { return m_mode == FULL_COPY; }

signals:
    void modeChanged(CopyMode mode);

private:
    CopyModeManager() : QObject(), m_mode(LIGHT_COPY) {}
    ~CopyModeManager() = default;
    CopyModeManager(const CopyModeManager&) = delete;
    CopyModeManager& operator=(const CopyModeManager&) = delete;

    CopyMode m_mode;
};

} // namespace Zhifen

#endif // COPY_MODE_MANAGER_H
