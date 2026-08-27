import { Entity } from './Entity';
import { Point, Rect, point, rectFromPoints } from '../core/types';

export interface PolylineVertex {
  x: number;
  y: number;
  bulge?: number; // 凸度，0=直线，>0=顺时针弧，<0=逆时针弧
  width?: number;  // 起始线宽
  endWidth?: number; // 结束线宽
}

export class PolylineEntity extends Entity {
  vertices: PolylineVertex[] = [];
  closed: boolean = false;
  constantWidth: number = 0;

  constructor(vertices: Point[] = [], closed: boolean = false, layer: string = '0') {
    super({ type: 'polyline', layer });
    this.vertices = vertices.map(v => ({ x: v.x, y: v.y }));
    this.closed = closed;
  }

  getBounds(): Rect {
    if (this.vertices.length === 0) return { x: 0, y: 0, width: 0, height: 0 };
    let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
    for (const v of this.vertices) {
      minX = Math.min(minX, v.x); minY = Math.min(minY, v.y);
      maxX = Math.max(maxX, v.x); maxY = Math.max(maxY, v.y);
    }
    return { x: minX, y: minY, width: maxX - minX, height: maxY - minY };
  }

  getCenter(): Point {
    const b = this.getBounds();
    return point(b.x + b.width / 2, b.y + b.height / 2);
  }

  move(dx: number, dy: number): void {
    for (const v of this.vertices) { v.x += dx; v.y += dy; }
  }

  rotate(center: Point, angleRad: number): void {
    const cos = Math.cos(angleRad); const sin = Math.sin(angleRad);
    for (const v of this.vertices) {
      const dx = v.x - center.x; const dy = v.y - center.y;
      v.x = center.x + dx * cos - dy * sin;
      v.y = center.y + dx * sin + dy * cos;
    }
  }

  scale(center: Point, factor: number): void {
    for (const v of this.vertices) {
      v.x = center.x + (v.x - center.x) * factor;
      v.y = center.y + (v.y - center.y) * factor;
    }
    this.constantWidth *= factor;
  }

  distanceToPoint(p: Point): number {
    let minDist = Infinity;
    const count = this.closed ? this.vertices.length : this.vertices.length - 1;
    for (let i = 0; i < count; i++) {
      const v1 = this.vertices[i];
      const v2 = this.vertices[(i + 1) % this.vertices.length];
      const d = this.pointToSegmentDistance(p, point(v1.x, v1.y), point(v2.x, v2.y));
      minDist = Math.min(minDist, d);
    }
    return minDist;
  }

  intersectsRect(r: Rect): boolean {
    for (const v of this.vertices) {
      if (v.x >= r.x && v.x <= r.x + r.width && v.y >= r.y && v.y <= r.y + r.height) return true;
    }
    const count = this.closed ? this.vertices.length : this.vertices.length - 1;
    for (let i = 0; i < count; i++) {
      const v1 = this.vertices[i];
      const v2 = this.vertices[(i + 1) % this.vertices.length];
      if (this.segmentIntersectsRect(point(v1.x, v1.y), point(v2.x, v2.y), r)) return true;
    }
    return false;
  }

  clone(): Entity {
    const p = new PolylineEntity([], this.closed, this.layer);
    p.vertices = this.vertices.map(v => ({ ...v }));
    p.constantWidth = this.constantWidth;
    p.color = this.color; p.lineType = this.lineType; p.lineWidth = this.lineWidth;
    return p;
  }

  toJSON(): any {
    return { id: this.id, type: this.type, layer: this.layer, color: this.color, lineType: this.lineType, lineWidth: this.lineWidth, vertices: this.vertices, closed: this.closed, constantWidth: this.constantWidth };
  }

  fromJSON(data: any): void {
    this.id = data.id; this.layer = data.layer; this.color = data.color;
    this.lineType = data.lineType; this.lineWidth = data.lineWidth;
    this.vertices = data.vertices; this.closed = data.closed;
    this.constantWidth = data.constantWidth || 0;
  }

  private pointToSegmentDistance(p: Point, a: Point, b: Point): number {
    const A = p.x - a.x; const B = p.y - a.y;
    const C = b.x - a.x; const D = b.y - a.y;
    const dot = A * C + B * D; const lenSq = C * C + D * D;
    let param = lenSq !== 0 ? dot / lenSq : -1;
    param = Math.max(0, Math.min(1, param));
    const xx = a.x + param * C; const yy = a.y + param * D;
    return Math.sqrt((p.x - xx) ** 2 + (p.y - yy) ** 2);
  }

  private segmentIntersectsRect(p1: Point, p2: Point, r: Rect): boolean {
    const corners = [point(r.x, r.y), point(r.x + r.width, r.y), point(r.x + r.width, r.y + r.height), point(r.x, r.y + r.height)];
    for (let i = 0; i < 4; i++) {
      if (this.segmentsIntersect(p1, p2, corners[i], corners[(i + 1) % 4])) return true;
    }
    return false;
  }

  private segmentsIntersect(p1: Point, p2: Point, p3: Point, p4: Point): boolean {
    const d1 = (p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x);
    const d2 = (p4.x - p3.x) * (p2.y - p3.y) - (p4.y - p3.y) * (p2.x - p3.x);
    const d3 = (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
    const d4 = (p2.x - p1.x) * (p4.y - p1.y) - (p2.y - p1.y) * (p4.x - p1.x);
    return ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) && ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0));
  }
}
