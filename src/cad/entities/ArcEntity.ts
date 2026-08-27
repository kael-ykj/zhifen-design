import { Entity } from './Entity';
import { Point, Rect, point } from '../core/types';

export class ArcEntity extends Entity {
  center: Point;
  radius: number;
  startAngle: number; // 弧度
  endAngle: number;   // 弧度
  counterClockwise: boolean = true;

  constructor(center: Point, radius: number, startAngle: number, endAngle: number, layer: string = '0') {
    super({ type: 'arc', layer });
    this.center = center;
    this.radius = radius;
    this.startAngle = startAngle;
    this.endAngle = endAngle;
  }

  getStartPoint(): Point {
    return point(this.center.x + this.radius * Math.cos(this.startAngle), this.center.y + this.radius * Math.sin(this.startAngle));
  }

  getEndPoint(): Point {
    return point(this.center.x + this.radius * Math.cos(this.endAngle), this.center.y + this.radius * Math.sin(this.endAngle));
  }

  getBounds(): Rect {
    const pts = [this.getStartPoint(), this.getEndPoint()];
    // 检查极值点
    const extrema = [0, Math.PI / 2, Math.PI, Math.PI * 1.5];
    for (const a of extrema) {
      if (this.isAngleInArc(a)) {
        pts.push(point(this.center.x + this.radius * Math.cos(a), this.center.y + this.radius * Math.sin(a)));
      }
    }
    let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
    for (const p of pts) {
      minX = Math.min(minX, p.x); minY = Math.min(minY, p.y);
      maxX = Math.max(maxX, p.x); maxY = Math.max(maxY, p.y);
    }
    return { x: minX, y: minY, width: maxX - minX, height: maxY - minY };
  }

  getCenter(): Point {
    return { ...this.center };
  }

  move(dx: number, dy: number): void {
    this.center.x += dx; this.center.y += dy;
  }

  rotate(center: Point, angleRad: number): void {
    const cos = Math.cos(angleRad); const sin = Math.sin(angleRad);
    const dx = this.center.x - center.x; const dy = this.center.y - center.y;
    this.center = point(center.x + dx * cos - dy * sin, center.y + dx * sin + dy * cos);
    this.startAngle += angleRad; this.endAngle += angleRad;
  }

  scale(center: Point, factor: number): void {
    this.center = point(center.x + (this.center.x - center.x) * factor, center.y + (this.center.y - center.y) * factor);
    this.radius *= factor;
  }

  distanceToPoint(p: Point): number {
    const d = Math.sqrt((p.x - this.center.x) ** 2 + (p.y - this.center.y) ** 2);
    const angle = Math.atan2(p.y - this.center.y, p.x - this.center.x);
    if (this.isAngleInArc(angle)) {
      return Math.abs(d - this.radius);
    }
    // 到端点的距离
    const d1 = Math.sqrt((p.x - this.getStartPoint().x) ** 2 + (p.y - this.getStartPoint().y) ** 2);
    const d2 = Math.sqrt((p.x - this.getEndPoint().x) ** 2 + (p.y - this.getEndPoint().y) ** 2);
    return Math.min(d1, d2);
  }

  intersectsRect(r: Rect): boolean {
    const pts = [this.getStartPoint(), this.getEndPoint()];
    for (const p of pts) {
      if (p.x >= r.x && p.x <= r.x + r.width && p.y >= r.y && p.y <= r.y + r.height) return true;
    }
    // 简化：检查圆心到矩形的距离
    const closestX = Math.max(r.x, Math.min(this.center.x, r.x + r.width));
    const closestY = Math.max(r.y, Math.min(this.center.y, r.y + r.height));
    const dx = this.center.x - closestX; const dy = this.center.y - closestY;
    return (dx * dx + dy * dy) < (this.radius * this.radius);
  }

  clone(): Entity {
    const a = new ArcEntity({ ...this.center }, this.radius, this.startAngle, this.endAngle, this.layer);
    a.counterClockwise = this.counterClockwise;
    a.color = this.color; a.lineType = this.lineType; a.lineWidth = this.lineWidth;
    return a;
  }

  toJSON(): any {
    return { id: this.id, type: this.type, layer: this.layer, color: this.color, lineType: this.lineType, lineWidth: this.lineWidth, center: this.center, radius: this.radius, startAngle: this.startAngle, endAngle: this.endAngle, counterClockwise: this.counterClockwise };
  }

  fromJSON(data: any): void {
    this.id = data.id; this.layer = data.layer; this.color = data.color;
    this.lineType = data.lineType; this.lineWidth = data.lineWidth;
    this.center = data.center; this.radius = data.radius;
    this.startAngle = data.startAngle; this.endAngle = data.endAngle;
    this.counterClockwise = data.counterClockwise;
  }

  private isAngleInArc(angle: number): boolean {
    let a = angle;
    while (a < 0) a += Math.PI * 2;
    while (a >= Math.PI * 2) a -= Math.PI * 2;
    let s = this.startAngle % (Math.PI * 2);
    let e = this.endAngle % (Math.PI * 2);
    if (s < 0) s += Math.PI * 2;
    if (e < 0) e += Math.PI * 2;
    if (this.counterClockwise) {
      if (s <= e) return a >= s && a <= e;
      return a >= s || a <= e;
    } else {
      if (s >= e) return a <= s && a >= e;
      return a <= s || a >= e;
    }
  }
}
