import { Entity } from './Entity';
import { Point, Rect, point } from '../core/types';

export class RectangleEntity extends Entity {
  corner1: Point;
  corner2: Point;
  rotation: number = 0; // 弧度

  constructor(corner1: Point, corner2: Point, layer: string = '0') {
    super({ type: 'rectangle', layer });
    this.corner1 = corner1;
    this.corner2 = corner2;
  }

  getVertices(): Point[] {
    const cx = (this.corner1.x + this.corner2.x) / 2;
    const cy = (this.corner1.y + this.corner2.y) / 2;
    const w = Math.abs(this.corner2.x - this.corner1.x);
    const h = Math.abs(this.corner2.y - this.corner1.y);
    const cos = Math.cos(this.rotation);
    const sin = Math.sin(this.rotation);
    const corners = [
      point(-w / 2, -h / 2), point(w / 2, -h / 2),
      point(w / 2, h / 2), point(-w / 2, h / 2),
    ];
    return corners.map(c => point(cx + c.x * cos - c.y * sin, cy + c.x * sin + c.y * cos));
  }

  getBounds(): Rect {
    const verts = this.getVertices();
    let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
    for (const v of verts) {
      minX = Math.min(minX, v.x); minY = Math.min(minY, v.y);
      maxX = Math.max(maxX, v.x); maxY = Math.max(maxY, v.y);
    }
    return { x: minX, y: minY, width: maxX - minX, height: maxY - minY };
  }

  getCenter(): Point {
    return point((this.corner1.x + this.corner2.x) / 2, (this.corner1.y + this.corner2.y) / 2);
  }

  move(dx: number, dy: number): void {
    this.corner1.x += dx; this.corner1.y += dy;
    this.corner2.x += dx; this.corner2.y += dy;
  }

  rotate(center: Point, angleRad: number): void {
    const cos = Math.cos(angleRad); const sin = Math.sin(angleRad);
    for (const p of [this.corner1, this.corner2]) {
      const dx = p.x - center.x; const dy = p.y - center.y;
      p.x = center.x + dx * cos - dy * sin;
      p.y = center.y + dx * sin + dy * cos;
    }
    this.rotation += angleRad;
  }

  scale(center: Point, factor: number): void {
    for (const p of [this.corner1, this.corner2]) {
      p.x = center.x + (p.x - center.x) * factor;
      p.y = center.y + (p.y - center.y) * factor;
    }
  }

  distanceToPoint(p: Point): number {
    const verts = this.getVertices();
    let minDist = Infinity;
    for (let i = 0; i < 4; i++) {
      const d = this.pointToSegmentDistance(p, verts[i], verts[(i + 1) % 4]);
      minDist = Math.min(minDist, d);
    }
    return minDist;
  }

  intersectsRect(r: Rect): boolean {
    const verts = this.getVertices();
    for (const v of verts) {
      if (v.x >= r.x && v.x <= r.x + r.width && v.y >= r.y && v.y <= r.y + r.height) return true;
    }
    for (let i = 0; i < 4; i++) {
      if (this.segmentIntersectsRect(verts[i], verts[(i + 1) % 4], r)) return true;
    }
    return false;
  }

  clone(): Entity {
    const r = new RectangleEntity({ ...this.corner1 }, { ...this.corner2 }, this.layer);
    r.rotation = this.rotation;
    r.color = this.color; r.lineType = this.lineType; r.lineWidth = this.lineWidth;
    return r;
  }

  toJSON(): any {
    return { id: this.id, type: this.type, layer: this.layer, color: this.color, lineType: this.lineType, lineWidth: this.lineWidth, corner1: this.corner1, corner2: this.corner2, rotation: this.rotation };
  }

  fromJSON(data: any): void {
    this.id = data.id; this.layer = data.layer; this.color = data.color;
    this.lineType = data.lineType; this.lineWidth = data.lineWidth;
    this.corner1 = data.corner1; this.corner2 = data.corner2;
    this.rotation = data.rotation || 0;
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
