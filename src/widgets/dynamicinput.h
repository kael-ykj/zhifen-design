#ifndef DYNAMICINPUT_H
#define DYNAMICINPUT_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPointF>
#include <QPoint>

class CadView;

// 动态输入组件：在光标旁显示坐标和长度输入框
class DynamicInput : public QWidget
{
    Q_OBJECT
public:
    explicit DynamicInput(QWidget *parent = nullptr);

    // 启用/禁用
    void setEnabled(bool enabled) { m_enabled = enabled; hide(); }
    bool isEnabled() const { return m_enabled; }

    // 更新位置（跟随光标）
    void updatePosition(const QPoint &screenPos);

    // 更新显示内容
    void updateCoordinate(const QPointF &worldPos);
    void updateDistance(qreal distance, qreal angle);
    void updatePrompt(const QString &prompt);

    // 显示/隐藏
    void showInput();
    void hideInput();

    // 获取输入值
    qreal inputValue() const;
    bool hasInput() const { return m_hasInput; }
    void clearInput();

signals:
    void valueEntered(qreal value);
    void coordinateEntered(const QPointF &pos);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    bool m_enabled = true;
    bool m_hasInput = false;

    QLabel *m_coordLabel;      // 坐标显示
    QLabel *m_distLabel;       // 距离/角度显示
    QLabel *m_promptLabel;     // 提示文字
    QLineEdit *m_inputEdit;    // 数值输入框

    QPointF m_currentWorldPos;
    qreal m_currentDistance = 0;
    qreal m_currentAngle = 0;

    void setupUI();
    void updateSize();
};

#endif // DYNAMICINPUT_H
