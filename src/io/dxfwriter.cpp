#include "dxfwriter.h"
#include "cadscene.h"
#include "cad/document.h"
#include "entities/caditem.h"
#include "entities/lineitem.h"
#include "entities/circleitem.h"
#include "entities/arcitem.h"
#include "entities/polylineitem.h"
#include "entities/rectangleitem.h"
#include "entities/textitem.h"
#include "entities/dimensionitem.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>

DxfWriter::DxfWriter(CadScene *scene) : m_scene(scene) {}

bool DxfWriter::write(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file); out << writeToString(); file.close(); return true;
}

QString DxfWriter::writeToString() {
    QStringList dxf;
    dxf << "0" << "SECTION" << "2" << "HEADER";
    dxf << "9" << "$ACADVER" << "1" << "AC1024";
    dxf << "0" << "ENDSEC";
    dxf << "0" << "SECTION" << "2" << "TABLES";
    dxf << "0" << "TABLE" << "2" << "LAYER" << "70" << "10";
    dxf << "0" << "LAYER" << "2" << "0" << "70" << "0" << "62" << "7" << "6" << "Continuous";
    dxf << "0" << "LAYER" << "2" << "DEFPOINTS" << "70" << "0" << "62" << "7" << "6" << "Continuous";
    dxf << "0" << "LAYER" << "2" << "墙体" << "70" << "0" << "62" << "8" << "6" << "Continuous";
    dxf << "0" << "LAYER" << "2" << "门窗" << "70" << "0" << "62" << "2" << "6" << "Continuous";
    dxf << "0" << "LAYER" << "2" << "天线" << "70" << "0" << "62" << "1" << "6" << "Continuous";
    dxf << "0" << "LAYER" << "2" << "器件" << "70" << "0" << "62" << "5" << "6" << "Continuous";
    dxf << "0" << "LAYER" << "2" << "馈线" << "70" << "0" << "62" << "7" << "6" << "Continuous";
    dxf << "0" << "LAYER" << "2" << "光纤" << "70" << "0" << "62" << "3" << "6" << "Continuous";
    dxf << "0" << "LAYER" << "2" << "标注" << "70" << "0" << "62" << "4" << "6" << "Continuous";
    dxf << "0" << "LAYER" << "2" << "文字" << "70" << "0" << "62" << "3" << "6" << "Continuous";
    dxf << "0" << "ENDTAB" << "0" << "ENDSEC";
    dxf << "0" << "SECTION" << "2" << "ENTITIES";
    if (m_scene) {
        for (auto item : m_scene->items()) {
            if (auto line = dynamic_cast<LineItem*>(item)) {
                dxf << "0" << "LINE" << "8" << line->layer();
                dxf << "10" << QString::number(line->startPoint().x(),'f',6) << "20" << QString::number(line->startPoint().y(),'f',6);
                dxf << "11" << QString::number(line->endPoint().x(),'f',6) << "21" << QString::number(line->endPoint().y(),'f',6);
            } else if (auto circle = dynamic_cast<CircleItem*>(item)) {
                dxf << "0" << "CIRCLE" << "8" << circle->layer();
                dxf << "10" << QString::number(circle->centerPoint().x(),'f',6) << "20" << QString::number(circle->centerPoint().y(),'f',6);
                dxf << "40" << QString::number(circle->radius(),'f',6);
            } else if (auto arc = dynamic_cast<ArcItem*>(item)) {
                dxf << "0" << "ARC" << "8" << arc->layer();
                dxf << "10" << QString::number(arc->centerPoint().x(),'f',6) << "20" << QString::number(arc->centerPoint().y(),'f',6);
                dxf << "40" << QString::number(arc->radius(),'f',6);
                dxf << "50" << QString::number(arc->startAngle(),'f',6) << "51" << QString::number(arc->startAngle()+arc->spanAngle(),'f',6);
            } else if (auto poly = dynamic_cast<PolylineItem*>(item)) {
                dxf << "0" << "LWPOLYLINE" << "8" << poly->layer() << "90" << QString::number(poly->points().size()) << "70" << (poly->isClosed()?"1":"0");
                for (auto p : poly->points()) dxf << "10" << QString::number(p.x(),'f',6) << "20" << QString::number(p.y(),'f',6);
            } else if (auto rect = dynamic_cast<RectangleItem*>(item)) {
                QRectF r = rect->rectangle();
                dxf << "0" << "LWPOLYLINE" << "8" << rect->layer() << "90" << "4" << "70" << "1";
                dxf << "10" << QString::number(r.topLeft().x(),'f',6) << "20" << QString::number(r.topLeft().y(),'f',6);
                dxf << "10" << QString::number(r.topRight().x(),'f',6) << "20" << QString::number(r.topRight().y(),'f',6);
                dxf << "10" << QString::number(r.bottomRight().x(),'f',6) << "20" << QString::number(r.bottomRight().y(),'f',6);
                dxf << "10" << QString::number(r.bottomLeft().x(),'f',6) << "20" << QString::number(r.bottomLeft().y(),'f',6);
            } else if (auto text = dynamic_cast<TextItem*>(item)) {
                dxf << "0" << "TEXT" << "8" << text->layer();
                dxf << "10" << QString::number(text->position().x(),'f',6) << "20" << QString::number(text->position().y(),'f',6);
                dxf << "40" << QString::number(text->textHeight(),'f',6) << "1" << text->text();
            } else if (auto dim = dynamic_cast<DimensionItem*>(item)) {
                dxf << "0" << "LINE" << "8" << dim->layer();
                dxf << "10" << QString::number(dim->startPoint().x(),'f',6) << "20" << QString::number(dim->startPoint().y(),'f',6);
                dxf << "11" << QString::number(dim->endPoint().x(),'f',6) << "21" << QString::number(dim->endPoint().y(),'f',6);
            }
        }
    }
    dxf << "0" << "ENDSEC" << "0" << "EOF";
    return dxf.join("\n") + "\n";
}
