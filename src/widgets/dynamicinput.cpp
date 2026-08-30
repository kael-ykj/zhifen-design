#include "dynamicinput.h"
#include "cadview.h"
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
#include <QtMath>

DynamicInput::DynamicInput(QWidget *parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint)
{
    setupUI();
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    hide();
}

void DynamicInput::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(2);

    // 提示文字
    m_promptLabel = new QLabel(this);
    m_promptLabel->setStyleSheet("color: #4ec9b0; font-size: 11px; font-family: Consolas, monospace;");
    layout->addWidget(m_promptLabel);

    // 坐标显示
    m_coordLabel = new QLabel(this);
    m_coordLabel->setStyleSheet("color: #fff; font-size: 12px; font-family: Consolas, monospace;");
    layout->addWidget(m_coordLabel);

    // 距离/角度显示
    m_distLabel = new QLabel(this);
    m_distLabel->setStyleSheet("color: #569cd6; font-size: 12px; font-family: Consolas, monospace;");
    layout->addWidget(m_distLabel);

    // 输入框
    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setStyleSheet("QLineEdit { background: #1e1e1e; color: #fff; border: 1px solid #0e639c; padding: 2px 4px; font-family: Consolas, monospace; font-size: 12px; }");
    m_inputEdit->setFixedWidth(120);
    m_inputEdit->installEventFilter(this);
    layout->addWidget(m_inputEdit);

    setStyleSheet("background: rgba(30, 30, 30, 240); border: 1px solid #3c3c3c; border-radius: 4px;");
    updateSize();
}

void DynamicInput::updateSize()
{
    adjustSize();
    setFixedSize(sizeHint());
}

void DynamicInput::updatePosition(const QPoint &screenPos)
{
    if (!m_enabled) return;
    // 显示在光标右下方
    QPoint pos = screenPos + QPoint(20, 20);

    // 确保不超出屏幕
    QScreen *screen = QApplication::screenAt(screenPos);
    if (screen) {
        QRect geo = screen->availableGeometry();
        if (pos.x() + width() > geo.right()) pos.setX(screenPos.x() - width() - 20);
        if (pos.y() + height() > geo.bottom()) pos.setY(screenPos.y() - height() - 20);
    }

    move(pos);
}

void DynamicInput::updateCoordinate(const QPointF &worldPos)
{
    m_currentWorldPos = worldPos;
    m_coordLabel->setText(QString("X: %1  Y: %2")
                              .arg(worldPos.x(), 0, 'f', 2)
                              .arg(worldPos.y(), 0, 'f', 2));
    updateSize();
}

void DynamicInput::updateDistance(qreal distance, qreal angle)
{
    m_currentDistance = distance;
    m_currentAngle = angle;
    m_distLabel->setText(QString("距离: %1  角度: %2°")
                             .arg(distance, 0, 'f', 2)
                             .arg(angle * 180 / M_PI, 0, 'f', 1));
    updateSize();
}

void DynamicInput::updatePrompt(const QString &prompt)
{
    m_promptLabel->setText(prompt);
    m_promptLabel->setVisible(!prompt.isEmpty());
    updateSize();
}

void DynamicInput::showInput()
{
    if (!m_enabled) return;
    show();
    m_inputEdit->clear();
    m_inputEdit->setFocus();
    m_hasInput = false;
}

void DynamicInput::hideInput()
{
    hide();
    m_inputEdit->clear();
    m_hasInput = false;
}

qreal DynamicInput::inputValue() const
{
    bool ok;
    qreal val = m_inputEdit->text().toDouble(&ok);
    return ok ? val : 0;
}

void DynamicInput::clearInput()
{
    m_inputEdit->clear();
    m_hasInput = false;
}

void DynamicInput::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (!m_inputEdit->text().isEmpty()) {
            m_hasInput = true;
            emit valueEntered(inputValue());
        }
        hideInput();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        hideInput();
        return;
    }
    QWidget::keyPressEvent(event);
}

bool DynamicInput::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_inputEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (!m_inputEdit->text().isEmpty()) {
                m_hasInput = true;
                emit valueEntered(inputValue());
            }
            hideInput();
            return true;
        }
        if (keyEvent->key() == Qt::Key_Escape) {
            hideInput();
            return true;
        }
        // Tab键切换到角度输入（暂未实现）
        if (keyEvent->key() == Qt::Key_Tab) {
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}
