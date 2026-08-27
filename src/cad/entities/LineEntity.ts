import { Entity } from './Entity';
import { Point, Rect, point, distance, angle, rectFromPoints } from '../core/types';

export class LineEntity extends Entity {
  start: Point;
  end: Point;

  constructor(start: Point, end: Point, layer: string = '0') {
    super({ type: 'line', layer });
    this.start = start;
    this.end = end;
  }

  getBounds(): Rect {
    return rectFromPoints(this.start, this.end);
  }

  getCenter(): Point {
    return point((this.start.x + this.end.x) / 2, (this.start.y + this.end.y) / 2);
  }

  move(dx: number, dy: number): void {
    this.start.x += dx;
    this.start.y += dy;
    this.end.x += dx;
    this.end.y += dy;
  }

  rotate(center: Point, angleRad: number): void {
    this.start = this.rotatePoint(this.start, center, angleRad);
    this.end = this.rotatePoint(this.end, center, angleRad);
  }

  scale(center: Point, factor: number): void {
    this.start = this.scalePoint(this.start, center, factor);
    this.end = this.scalePoint(this.end, center, factor);
  }

  distanceToPoint(p: Point): number {
    return this.pointToLineDistance(p, this.start, this.end);
  }

  intersectsRect(r: Rect): boolean {
    // 检查端点是否在矩形内
    if (this.pointInRect(this.start, r) || this.pointInRect(this.end, r)) return true;
    // 检查线段是否与矩形四边相交
    const corners = [
      point(r.x, r.y), point(r.x + r.width, r.y),
      point(r.x + r.width, r.y + r.height), point(r.x, r.y + r.height),
    ];
    for (let i = 0; i < 4; i++) {
      if (this.segmentsIntersect(this.start, this.end, corners[i], corners[(i + 1) % 4])) {
        return true;
      }
    }
    return false;
  }

  clone(): Entity {
    const line = new LineEntity({ ...this.start }, { ...this.end }, this.layer);
    line.color = this.color;
    line.lineType = this.lineType;
    line.lineWidth = this.lineWidth;
    return line;
  }

  toJSON(): any {
    return {
      id: this.id, type: this.type, layer: this.layer,
      color: this.color, lineType: this.lineType, lineWidth: this.lineWidth,
      start: this.start, end: this.end,
    };
  }

  fromJSON(data: any): void {
    this.id = data.id;
    this.layer = data.layer;
    this.color = data.color;
    this.lineType = data.lineType;
    this.lineWidth = data.lineWidth;
    this.start = data.start;
    this.end = data.end;
  }

  private rotatePoint(p: Point, center: Point, angleRad: number): Point {
    const cos = Math.cos(angleRad);
    const sin = Math.sin(angleRad);
    const dx = p.x - center.x;
    const dy = p.y - center.y;
    return point(center.x + dx * cos - dy * sin, center.y + dx * sin + dy * cos);
  }

  private scalePoint(p: Point, center: Point, factor: number): Point {
    return point(center.x + (p.x - center.x) * factor, center.y + (p.y - center.y) * factor);
  }

  private pointInRect(p: Point, r: Rect): boolean {
    return p.x >= r.x && p.x <= r.x + r.width && p.y >= r.y && p.y <= r.y + r.height;
  }

  private pointToLineDistance(p: Point, a: Point, b: Point): number {
    const A = p.x - a.x;
    const B = p.y - a.y;
    const C = b.x - a.x;
    const D = b.y - a.y;
    const dot = A * C + B * D;
    const lenSq = C * C + D * D;
    let param = lenSq !== 0 ? dot / lenSq : -1;
    param = Math.max(0, Math.min(1, param));
    const xx = a.x + param * C;
    const yy = a.y + param * D;
    return Math.sqrt((p.x - xx) ** 2 + (p.y - yy) ** 2);
  }

  private segmentsIntersect(p1: Point, p2: Point, p3: Point, p4: Point): boolean {
    const d1 = this.crossProduct(p4.x - p3.x, p4.y - p3.y, p1.x - p3.x, p1.y - p3.y);
    const d2 = this.crossProduct(p4.x - p3.x, p4.y - p3.y, p2.x - p3.x, p2.y - p3.y);
    const d3 = this.crossProduct(p2.x - p1.x, p2.y - p1.y, p3.x - p1.x, p3.y - p1.y);
    const d4 = this.crossProduct(p2.x - p1.x, p2.y - p1.y, p4.x - p1.x, p4.y - p1.y);
    if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
        ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) return true;
    return false;
  }

  private crossProduct(x1: number, y1: number, x2: number, y2: number): number {
    return x1 * y2 - y1 * x2;
  }
}
