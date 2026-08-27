import { Entity } from './Entity';
import { Point, Rect, point, distance } from '../core/types';

export type DimensionType = 'linear' | 'aligned' | 'radius' | 'diameter' | 'angular' | 'leader';

export class DimensionEntity extends Entity {
  dimType: DimensionType;
  point1: Point;
  point2: Point;
  dimensionLinePoint: Point; // 尺寸线位置
  text: string = '';
  textHeight: number = 2.5;
  arrowSize: number = 2.5;
  extensionLineOffset: number = 0.625;
  extensionLineExtension: number = 1.25;

  constructor(dimType: DimensionType, p1: Point, p2: Point, dimLinePoint: Point, layer: string = '0') {
    super({ type: 'dimension', layer });
    this.dimType = dimType;
    this.point1 = p1;
    this.point2 = p2;
    this.dimensionLinePoint = dimLinePoint;
  }

  getMeasurement(): number {
    return distance(this.point1, this.point2);
  }

  getDisplayText(): string {
    if (this.text) return this.text;
    return this.getMeasurement().toFixed(2);
  }

  getBounds(): Rect {
    const allPoints = [this.point1, this.point2, this.dimensionLinePoint];
    let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
    for (const p of allPoints) {
      minX = Math.min(minX, p.x); minY = Math.min(minY, p.y);
      maxX = Math.max(maxX, p.x); maxY = Math.max(maxY, p.y);
    }
    const padding = this.textHeight * 3;
    return { x: minX - padding, y: minY - padding, width: (maxX - minX) + padding * 2, height: (maxY - minY) + padding * 2 };
  }

  getCenter(): Point {
    const b = this.getBounds();
    return point(b.x + b.width / 2, b.y + b.height / 2);
  }

  move(dx: number, dy: number): void {
    this.point1.x += dx; this.point1.y += dy;
    this.point2.x += dx; this.point2.y += dy;
    this.dimensionLinePoint.x += dx; this.dimensionLinePoint.y += dy;
  }

  rotate(center: Point, angleRad: number): void {
    const cos = Math.cos(angleRad); const sin = Math.sin(angleRad);
    for (const p of [this.point1, this.point2, this.dimensionLinePoint]) {
      const dx = p.x - center.x; const dy = p.y - center.y;
      p.x = center.x + dx * cos - dy * sin;
      p.y = center.y + dx * sin + dy * cos;
    }
  }

  scale(center: Point, factor: number): void {
    for (const p of [this.point1, this.point2, this.dimensionLinePoint]) {
      p.x = center.x + (p.x - center.x) * factor;
      p.y = center.y + (p.y - center.y) * factor;
    }
    this.textHeight *= factor;
    this.arrowSize *= factor;
  }

  distanceToPoint(p: Point): number {
    const b = this.getBounds();
    if (p.x >= b.x && p.x <= b.x + b.width && p.y >= b.y && p.y <= b.y + b.height) return 0;
    const closestX = Math.max(b.x, Math.min(p.x, b.x + b.width));
    const closestY = Math.max(b.y, Math.min(p.y, b.y + b.height));
    return Math.sqrt((p.x - closestX) ** 2 + (p.y - closestY) ** 2);
  }

  intersectsRect(r: Rect): boolean {
    const b = this.getBounds();
    return b.x < r.x + r.width && b.x + b.width > r.x && b.y < r.y + r.height && b.y + b.height > r.y;
  }

  clone(): Entity {
    const d = new DimensionEntity(this.dimType, { ...this.point1 }, { ...this.point2 }, { ...this.dimensionLinePoint }, this.layer);
    d.text = this.text; d.textHeight = this.textHeight; d.arrowSize = this.arrowSize;
    d.extensionLineOffset = this.extensionLineOffset; d.extensionLineExtension = this.extensionLineExtension;
    d.color = this.color; d.lineType = this.lineType; d.lineWidth = this.lineWidth;
    return d;
  }

  toJSON(): any {
    return { id: this.id, type: this.type, layer: this.layer, color: this.color, lineType: this.lineType, lineWidth: this.lineWidth, dimType: this.dimType, point1: this.point1, point2: this.point2, dimensionLinePoint: this.dimensionLinePoint, text: this.text, textHeight: this.textHeight, arrowSize: this.arrowSize, extensionLineOffset: this.extensionLineOffset, extensionLineExtension: this.extensionLineExtension };
  }

  fromJSON(data: any): void {
    this.id = data.id; this.layer = data.layer; this.color = data.color;
    this.lineType = data.lineType; this.lineWidth = data.lineWidth;
    this.dimType = data.dimType; this.point1 = data.point1; this.point2 = data.point2;
    this.dimensionLinePoint = data.dimensionLinePoint; this.text = data.text;
    this.textHeight = data.textHeight || 2.5; this.arrowSize = data.arrowSize || 2.5;
    this.extensionLineOffset = data.extensionLineOffset || 0.625;
    this.extensionLineExtension = data.extensionLineExtension || 1.25;
  }
}
