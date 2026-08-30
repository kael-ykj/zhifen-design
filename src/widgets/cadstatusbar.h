#ifndef CADSTATUSBAR_H
#define CADSTATUSBAR_H

#include <QStatusBar>
#include <QLabel>
#include <QPushButton>
#include <QString>

namespace Zhifen {

class CadStatusBar : public QStatusBar
{
    Q_OBJECT
public:
    explicit CadStatusBar(QWidget *parent = nullptr);

    void setCoordinate(const QPointF &pos);
    void setCommand(const QString &cmd);
    void setSelectionCount(int count);

    bool isSnapEnabled() const { return m_snapBtn->isChecked(); }
    bool isOrthoEnabled() const { return m_orthoBtn->isChecked(); }
    bool isGridEnabled() const { return m_gridBtn->isChecked(); }
    bool isPolarEnabled() const { return m_polarBtn->isChecked(); }

signals:
    void snapToggled(bool enabled);
    void orthoToggled(bool enabled);
    void gridToggled(bool enabled);
    void polarToggled(bool enabled);

private:
    QLabel *m_coordLabel;
    QLabel *m_commandLabel;
    QLabel *m_selectionLabel;
    QPushButton *m_snapBtn;
    QPushButton *m_orthoBtn;
    QPushButton *m_gridBtn;
    QPushButton *m_polarBtn;
    QPushButton *m_lwtBtn;
    QPushButton *m_modelBtn;

    QPushButton *createToggleButton(const QString &text);
};

} // namespace Zhifen

#endif // CADSTATUSBAR_H
