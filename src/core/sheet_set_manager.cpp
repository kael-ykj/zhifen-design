#include "sheet_set_manager.h"
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPixmap>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QtMath>

namespace Zhifen {

SheetSetManager::SheetSetManager() {}
SheetSetManager::~SheetSetManager() {}

SheetSetManager& SheetSetManager::instance() {
    static SheetSetManager inst;
    return inst;
}

int SheetSetManager::addSheet(const SheetInfo &sheet) {
    SheetInfo s = sheet;
    s.order = m_sheets.size();
    m_sheets.append(s);
    return m_sheets.size() - 1;
}

bool SheetSetManager::removeSheet(int index) {
    if (index < 0 || index >= m_sheets.size()) return false;
    m_sheets.removeAt(index);
    // 重新排序
    for (int i = 0; i < m_sheets.size(); i++) {
        m_sheets[i].order = i;
    }
    return true;
}

bool SheetSetManager::moveSheet(int from, int to) {
    if (from < 0 || from >= m_sheets.size() || to < 0 || to >= m_sheets.size()) return false;
    m_sheets.move(from, to);
    for (int i = 0; i < m_sheets.size(); i++) {
        m_sheets[i].order = i;
    }
    return true;
}

SheetInfo SheetSetManager::sheet(int index) const {
    if (index < 0 || index >= m_sheets.size()) return SheetInfo();
    return m_sheets[index];
}

bool SheetSetManager::updateSheet(int index, const SheetInfo &sheet) {
    if (index < 0 || index >= m_sheets.size()) return false;
    m_sheets[index] = sheet;
    return true;
}

void SheetSetManager::clear() {
    m_sheets.clear();
}

QString SheetSetManager::generateSheetNo(const SheetNumberConfig &config, int index) const {
    int num = config.startNumber + index * config.step;
    QString numStr = QString("%1").arg(num, config.digits, 10, QChar('0'));
    return QString("%1%2%3%4%5")
        .arg(config.prefix).arg(config.separator).arg(config.year)
        .arg(config.separator).arg(numStr);
}

void SheetSetManager::autoNumber(const SheetNumberConfig &config) {
    m_numberConfig = config;
    for (int i = 0; i < m_sheets.size(); i++) {
        m_sheets[i].sheetNo = generateSheetNo(config, i);
    }
}

void SheetSetManager::drawTitleBlock(QPainter *painter, const QRectF &pageRect, const SheetInfo &sheet) {
    painter->save();
    QPen borderPen(QColor(0, 0, 0), 2);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);

    // 外框
    painter->drawRect(pageRect.adjusted(10, 10, -10, -10));
    // 内框
    painter->setPen(QPen(QColor(0, 0, 0), 1));
    painter->drawRect(pageRect.adjusted(25, 20, -15, -15));

    // 标题栏（右下角）
    qreal tbWidth = 180;
    qreal tbHeight = 60;
    QRectF tbRect(pageRect.right() - 15 - tbWidth, pageRect.bottom() - 15 - tbHeight, tbWidth, tbHeight);
    painter->drawRect(tbRect);
    painter->drawLine(tbRect.left(), tbRect.top() + 20, tbRect.right(), tbRect.top() + 20);
    painter->drawLine(tbRect.left(), tbRect.top() + 40, tbRect.right(), tbRect.top() + 40);
    painter->drawLine(tbRect.left() + 60, tbRect.top(), tbRect.left() + 60, tbRect.bottom());
    painter->drawLine(tbRect.left() + 120, tbRect.top() + 20, tbRect.left() + 120, tbRect.bottom());

    QFont titleFont("SimSun", 10, QFont::Bold);
    QFont normalFont("SimSun", 8);
    QFont smallFont("SimSun", 7);

    painter->setFont(titleFont);
    painter->drawText(QRectF(tbRect.left() + 2, tbRect.top() + 2, tbWidth - 4, 16),
                      Qt::AlignCenter, sheet.projectName);
    painter->setFont(normalFont);
    painter->drawText(QRectF(tbRect.left() + 2, tbRect.top() + 22, tbWidth - 4, 16),
                      Qt::AlignCenter, sheet.name);
    painter->setFont(smallFont);
    painter->drawText(QRectF(tbRect.left() + 2, tbRect.top() + 42, 56, 16), Qt::AlignCenter, "设计: " + sheet.designer);
    painter->drawText(QRectF(tbRect.left() + 62, tbRect.top() + 42, 56, 16), Qt::AlignCenter, "审核: " + sheet.reviewer);
    painter->drawText(QRectF(tbRect.left() + 122, tbRect.top() + 42, 56, 16), Qt::AlignCenter, sheet.date);

    // 图号
    painter->drawText(QRectF(tbRect.left(), tbRect.top() - 18, tbWidth, 14),
                      Qt::AlignRight, "图号: " + sheet.sheetNo);
    // 比例
    painter->drawText(QRectF(tbRect.left(), tbRect.top() - 34, tbWidth, 14),
                      Qt::AlignRight, "比例: " + sheet.scale);
    // 运营商
    painter->setFont(normalFont);
    painter->drawText(QRectF(pageRect.left() + 30, pageRect.top() + 25, 200, 20),
                      Qt::AlignLeft, sheet.operatorName);

    painter->restore();
}

bool SheetSetManager::printSheet(QGraphicsScene *scene, const SheetInfo &sheet, QPrinter *printer) {
    if (!scene || !printer) return false;

    QPainter painter(printer);
    QRectF pageRect = printer->pageRect(QPrinter::DevicePixel);

    // 绘制图签
    if (m_printConfig.printTitleBlock) {
        drawTitleBlock(&painter, pageRect, sheet);
    }

    // 绘制图纸内容
    QRectF drawingArea = pageRect.adjusted(35, 30, -210, -90);
    painter.save();
    painter.setViewport(drawingArea.toRect());
    painter.setWindow(scene->itemsBoundingRect().toRect());
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter);
    painter.restore();

    painter.end();
    return true;
}

bool SheetSetManager::printAll(QGraphicsScene *scene, QWidget *parent) {
    if (!scene || m_sheets.isEmpty()) return false;

    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize(m_printConfig.paperSize);
    printer.setOrientation(m_printConfig.orientation);

    QPrintDialog dialog(&printer, parent);
    dialog.setWindowTitle("批量打印");
    if (dialog.exec() != QDialog::Accepted) return false;

    int printed = 0;
    for (int i = 0; i < m_sheets.size(); i++) {
        if (!m_sheets[i].selected) continue;
        if (i > 0) printer.newPage();
        printSheet(scene, m_sheets[i], &printer);
        printed++;
    }
    return printed > 0;
}

bool SheetSetManager::printSelected(QGraphicsScene *scene, const QList<int> &indices, QWidget *parent) {
    if (!scene || indices.isEmpty()) return false;

    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize(m_printConfig.paperSize);
    printer.setOrientation(m_printConfig.orientation);

    QPrintDialog dialog(&printer, parent);
    dialog.setWindowTitle("批量打印");
    if (dialog.exec() != QDialog::Accepted) return false;

    int printed = 0;
    for (int idx = 0; idx < indices.size(); idx++) {
        int i = indices[idx];
        if (i < 0 || i >= m_sheets.size()) continue;
        if (idx > 0) printer.newPage();
        printSheet(scene, m_sheets[i], &printer);
        printed++;
    }
    return printed > 0;
}

bool SheetSetManager::exportSheetToPdf(QGraphicsScene *scene, const SheetInfo &sheet, const QString &filePath) {
    if (!scene) return false;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPaperSize(m_printConfig.paperSize);
    printer.setOrientation(m_printConfig.orientation);

    return printSheet(scene, sheet, &printer);
}

bool SheetSetManager::exportAllToPdf(QGraphicsScene *scene, const QString &outputPath, bool merge) {
    if (!scene || m_sheets.isEmpty()) return false;

    if (merge) {
        // 合并为单个PDF
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(outputPath);
        printer.setPaperSize(m_printConfig.paperSize);
        printer.setOrientation(m_printConfig.orientation);

        int exported = 0;
        for (int i = 0; i < m_sheets.size(); i++) {
            if (!m_sheets[i].selected) continue;
            if (i > 0) printer.newPage();
            printSheet(scene, m_sheets[i], &printer);
            exported++;
        }
        return exported > 0;
    } else {
        // 分别导出
        QFileInfo fi(outputPath);
        QString basePath = fi.absolutePath() + "/" + fi.baseName();
        int exported = 0;
        for (int i = 0; i < m_sheets.size(); i++) {
            if (!m_sheets[i].selected) continue;
            QString filePath = QString("%1_%2.pdf").arg(basePath).arg(i + 1, 3, 10, QChar('0'));
            if (exportSheetToPdf(scene, m_sheets[i], filePath)) {
                exported++;
            }
        }
        return exported > 0;
    }
}

bool SheetSetManager::exportSelectedToPdf(QGraphicsScene *scene, const QList<int> &indices,
                                            const QString &outputPath, bool merge) {
    if (!scene || indices.isEmpty()) return false;

    if (merge) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(outputPath);
        printer.setPaperSize(m_printConfig.paperSize);
        printer.setOrientation(m_printConfig.orientation);

        int exported = 0;
        for (int idx = 0; idx < indices.size(); idx++) {
            int i = indices[idx];
            if (i < 0 || i >= m_sheets.size()) continue;
            if (idx > 0) printer.newPage();
            printSheet(scene, m_sheets[i], &printer);
            exported++;
        }
        return exported > 0;
    } else {
        QFileInfo fi(outputPath);
        QString basePath = fi.absolutePath() + "/" + fi.baseName();
        int exported = 0;
        for (int idx = 0; idx < indices.size(); idx++) {
            int i = indices[idx];
            if (i < 0 || i >= m_sheets.size()) continue;
            QString filePath = QString("%1_%2.pdf").arg(basePath).arg(i + 1, 3, 10, QChar('0'));
            if (exportSheetToPdf(scene, m_sheets[i], filePath)) {
                exported++;
            }
        }
        return exported > 0;
    }
}

QPixmap SheetSetManager::generatePreview(QGraphicsScene *scene, int index, const QSize &size) {
    if (!scene || index < 0 || index >= m_sheets.size()) return QPixmap();

    QPixmap pixmap(size);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF previewRect(5, 5, size.width() - 10, size.height() - 10);
    painter.setViewport(previewRect.toRect());
    painter.setWindow(scene->itemsBoundingRect().toRect());
    scene->render(&painter);

    painter.end();
    return pixmap;
}

bool SheetSetManager::saveToFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "[SheetSet]\n";
    out << "Count=" << m_sheets.size() << "\n";
    for (int i = 0; i < m_sheets.size(); i++) {
        const auto &s = m_sheets[i];
        out << QString("\n[Sheet%1]\n").arg(i);
        out << "Name=" << s.name << "\n";
        out << "SheetNo=" << s.sheetNo << "\n";
        out << "Scale=" << s.scale << "\n";
        out << "Project=" << s.projectName << "\n";
        out << "Designer=" << s.designer << "\n";
        out << "Reviewer=" << s.reviewer << "\n";
        out << "Date=" << s.date << "\n";
        out << "Operator=" << s.operatorName << "\n";
        out << "Selected=" << (s.selected ? "1" : "0") << "\n";
    }
    file.close();
    return true;
}

bool SheetSetManager::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
    m_sheets.clear();
    SheetInfo current;
    bool inSheet = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("[Sheet")) {
            if (inSheet) m_sheets.append(current);
            current = SheetInfo();
            inSheet = true;
        } else if (line.startsWith("Name=")) {
            current.name = line.mid(5);
        } else if (line.startsWith("SheetNo=")) {
            current.sheetNo = line.mid(8);
        } else if (line.startsWith("Scale=")) {
            current.scale = line.mid(6);
        } else if (line.startsWith("Project=")) {
            current.projectName = line.mid(8);
        } else if (line.startsWith("Designer=")) {
            current.designer = line.mid(9);
        } else if (line.startsWith("Reviewer=")) {
            current.reviewer = line.mid(9);
        } else if (line.startsWith("Date=")) {
            current.date = line.mid(5);
        } else if (line.startsWith("Operator=")) {
            current.operatorName = line.mid(9);
        } else if (line.startsWith("Selected=")) {
            current.selected = line.mid(9) == "1";
        }
    }
    if (inSheet) m_sheets.append(current);

    file.close();
    return !m_sheets.isEmpty();
}

} // namespace Zhifen
