import { Document } from '../core/Document';
import { Entity } from '../entities/Entity';
import { Point, point, ACI_COLORS } from '../core/types';
import { LineEntity } from '../entities/LineEntity';
import { CircleEntity } from '../entities/CircleEntity';
import { ArcEntity } from '../entities/ArcEntity';
import { PolylineEntity } from '../entities/PolylineEntity';
import { TextEntity } from '../entities/TextEntity';
import { DimensionEntity } from '../entities/DimensionEntity';
import { BlockReference } from '../entities/BlockEntity';

export class DxfWriter {
  document: Document;

  constructor(document: Document) {
    this.document = document;
  }

  write(): string {
    let dxf = '';
    dxf += this.writeHeader();
    dxf += this.writeTables();
    dxf += this.writeBlocks();
    dxf += this.writeEntities();
    dxf += '0\nEOF\n';
    return dxf;
  }

  private writeHeader(): string {
    let h = '0\nSECTION\n2\nHEADER\n';
    h += '9\n$ACADVER\n1\nAC1009\n';
    h += '9\n$INSBASE\n10\n0.0\n20\n0.0\n30\n0.0\n';
    h += '9\n$EXTMIN\n10\n0.0\n20\n0.0\n30\n0.0\n';
    h += '9\n$EXTMAX\n10\n1000.0\n20\n1000.0\n30\n0.0\n';
    h += '9\n$LTSCALE\n40\n1.0\n';
    h += '0\nENDSEC\n';
    return h;
  }

  private writeTables(): string {
    let t = '0\nSECTION\n2\nTABLES\n';
    // 图层表
    t += '0\nTABLE\n2\nLAYER\n70\n' + this.document.getAllLayers().length + '\n';
    for (const layer of this.document.getAllLayers()) {
      t += '0\nLAYER\n';
      t += '2\n' + layer.name + '\n';
      t += '70\n0\n';
      t += '62\n' + this.colorToAci(layer.color) + '\n';
      t += '6\nContinuous\n';
    }
    t += '0\nENDTAB\n';
    t += '0\nENDSEC\n';
    return t;
  }

  private writeBlocks(): string {
    return '0\nSECTION\n2\nBLOCKS\n0\nENDSEC\n';
  }

  private writeEntities(): string {
    let e = '0\nSECTION\n2\nENTITIES\n';
    for (const entity of this.document.getAllEntities()) {
      e += this.writeEntity(entity);
    }
    e += '0\nENDSEC\n';
    return e;
  }

  private writeEntity(entity: Entity): string {
    let e = '';
    const handle = entity.id.substring(0, 8).toUpperCase();
    if (entity instanceof LineEntity) {
      e += '0\nLINE\n5\n' + handle + '\n8\n' + entity.layer + '\n';
      e += '10\n' + entity.start.x + '\n20\n' + entity.start.y + '\n30\n0.0\n';
      e += '11\n' + entity.end.x + '\n21\n' + entity.end.y + '\n31\n0.0\n';
    } else if (entity instanceof CircleEntity) {
      e += '0\nCIRCLE\n5\n' + handle + '\n8\n' + entity.layer + '\n';
      e += '10\n' + entity.center.x + '\n20\n' + entity.center.y + '\n30\n0.0\n';
      e += '40\n' + entity.radius + '\n';
    } else if (entity instanceof ArcEntity) {
      e += '0\nARC\n5\n' + handle + '\n8\n' + entity.layer + '\n';
      e += '10\n' + entity.center.x + '\n20\n' + entity.center.y + '\n30\n0.0\n';
      e += '40\n' + entity.radius + '\n';
      e += '50\n' + (entity.startAngle * 180 / Math.PI) + '\n';
      e += '51\n' + (entity.endAngle * 180 / Math.PI) + '\n';
    } else if (entity instanceof PolylineEntity) {
      e += '0\nPOLYLINE\n5\n' + handle + '\n8\n' + entity.layer + '\n';
      e += '66\n1\n70\n' + (entity.closed ? 1 : 0) + '\n';
      for (let i = 0; i < entity.vertices.length; i++) {
        const v = entity.vertices[i];
        e += '0\nVERTEX\n5\n' + handle + '_' + i + '\n8\n' + entity.layer + '\n';
        e += '10\n' + v.x + '\n20\n' + v.y + '\n30\n0.0\n';
        if (v.bulge) e += '42\n' + v.bulge + '\n';
      }
      e += '0\nSEQEND\n5\n' + handle + '_end\n8\n' + entity.layer + '\n';
    } else if (entity instanceof TextEntity) {
      e += '0\nTEXT\n5\n' + handle + '\n8\n' + entity.layer + '\n';
      e += '10\n' + entity.position.x + '\n20\n' + entity.position.y + '\n30\n0.0\n';
      e += '40\n' + entity.height + '\n';
      e += '1\n' + entity.text + '\n';
      e += '50\n' + (entity.rotation * 180 / Math.PI) + '\n';
    } else if (entity instanceof DimensionEntity) {
      e += '0\nDIMENSION\n5\n' + handle + '\n8\n' + entity.layer + '\n';
      e += '10\n' + entity.dimensionLinePoint.x + '\n20\n' + entity.dimensionLinePoint.y + '\n30\n0.0\n';
      e += '11\n' + entity.point1.x + '\n21\n' + entity.point1.y + '\n31\n0.0\n';
      e += '12\n' + entity.point2.x + '\n22\n' + entity.point2.y + '\n32\n0.0\n';
      e += '1\n' + entity.getDisplayText() + '\n';
    }
    return e;
  }

  private colorToAci(color: string): number {
    const map: Record<string, number> = {
      '#FF0000': 1, '#FFFF00': 2, '#00FF00': 3, '#00FFFF': 4,
      '#0000FF': 5, '#FF00FF': 6, '#FFFFFF': 7, '#000000': 7,
      '#808080': 8, '#C0C0C0': 9,
    };
    return map[color.toUpperCase()] || 7;
  }
}
