#include "dxfwriter.h"
#include "cadscene.h"
#include "caditem.h"
#include "lineitem.h"
#include "circleitem.h"
#include "arcitem.h"
#include "polylineitem.h"
#include "textitem.h"
#include "dimensionitem.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>

DxfWriter::DxfWriter(CadScene *scene)
    : m_scene(scene)
{
}

bool DxfWriter::write(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    out << writeToString();
    file.close();
    return true;
}

QString DxfWriter::writeToString()
{
    QStringList dxf;

    // HEADER
    dxf << "0" << "SECTION" << "2" << "HEADER";
    dxf << "9" << "$ACADVER" << "1" << "AC1009";
    dxf << "0" << "ENDSEC";

    // TABLES - 图层
    dxf << "0" << "SECTION" << "2" << "TABLES";
    dxf << "0" << "TABLE" << "2" << "LAYER" << "70" << "1";
    dxf << "0" << "LAYER" << "2" << "0" << "70" << "0" << "62" << "7" << "6" << "Continuous";
    dxf << "0" << "ENDTAB";
    dxf << "0" << "ENDSEC";

    // ENTITIES
    dxf << "0" << "SECTION" << "2" << "ENTITIES";

    if (m_scene) {
        for (auto item : m_scene->items()) {
            if (auto line = dynamic_cast<LineItem*>(item)) {
                dxf << "0" << "LINE" << "8" << line->layer();
                dxf << "10" << QString::number(line->startPoint().x(), 'f', 6);
                dxf << "20" << QString::number(line->startPoint().y(), 'f', 6);
                dxf << "11" << QString::number(line->endPoint().x(), 'f', 6);
                dxf << "21" << QString::number(line->endPoint().y(), 'f', 6);
            } else if (auto circle = dynamic_cast<CircleItem*>(item)) {
                dxf << "0" << "CIRCLE" << "8" << circle->layer();
                dxf << "10" << QString::number(circle->centerPoint().x(), 'f', 6);
                dxf << "20" << QString::number(circle->centerPoint().y(), 'f', 6);
                dxf << "40" << QString::number(circle->radius(), 'f', 6);
            } else if (auto arc = dynamic_cast<ArcItem*>(item)) {
                dxf << "0" << "ARC" << "8" << arc->layer();
                dxf << "10" << QString::number(arc->centerPoint().x(), 'f', 6);
                dxf << "20" << QString::number(arc->centerPoint().y(), 'f', 6);
                dxf << "40" << QString::number(arc->radius(), 'f', 6);
                dxf << "50" << QString::number(arc->startAngle(), 'f', 6);
                dxf << "51" << QString::number(arc->startAngle() + arc->spanAngle(), 'f', 6);
            } else if (auto poly = dynamic_cast<PolylineItem*>(item)) {
                dxf << "0" << "LWPOLYLINE" << "8" << poly->layer();
                dxf << "90" << QString::number(poly->points().size());
                dxf << "70" << (poly->isClosed() ? "1" : "0");
                for (auto p : poly->points()) {
                    dxf << "10" << QString::number(p.x(), 'f', 6);
                    dxf << "20" << QString::number(p.y(), 'f', 6);
                }
            } else if (auto text = dynamic_cast<TextItem*>(item)) {
                dxf << "0" << "TEXT" << "8" << text->layer();
                dxf << "10" << QString::number(text->position().x(), 'f', 6);
                dxf << "20" << QString::number(text->position().y(), 'f', 6);
                dxf << "40" << QString::number(text->textHeight(), 'f', 6);
                dxf << "1" << text->text();
            }
        }
    }

    dxf << "0" << "ENDSEC";
    dxf << "0" << "EOF";

    return dxf.join("\n") + "\n";
}
