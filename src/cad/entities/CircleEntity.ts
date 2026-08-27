import { Entity } from './Entity';
import { Point, Rect, point } from '../core/types';

export class CircleEntity extends Entity {
  center: Point;
  radius: number;

  constructor(center: Point, radius: number, layer: string = '0') {
    super({ type: 'circle', layer });
    this.center = center;
    this.radius = radius;
  }

  getBounds(): Rect {
    return { x: this.center.x - this.radius, y: this.center.y - this.radius, width: this.radius * 2, height: this.radius * 2 };
  }

  getCenter(): Point {
    return { ...this.center };
  }

  move(dx: number, dy: number): void {
    this.center.x += dx;
    this.center.y += dy;
  }

  rotate(center: Point, angleRad: number): void {
    const cos = Math.cos(angleRad);
    const sin = Math.sin(angleRad);
    const dx = this.center.x - center.x;
    const dy = this.center.y - center.y;
    this.center = point(center.x + dx * cos - dy * sin, center.y + dx * sin + dy * cos);
  }

  scale(center: Point, factor: number): void {
    this.center = point(center.x + (this.center.x - center.x) * factor, center.y + (this.center.y - center.y) * factor);
    this.radius *= factor;
  }

  distanceToPoint(p: Point): number {
    const d = Math.sqrt((p.x - this.center.x) ** 2 + (p.y - this.center.y) ** 2);
    return Math.abs(d - this.radius);
  }

  intersectsRect(r: Rect): boolean {
    const closestX = Math.max(r.x, Math.min(this.center.x, r.x + r.width));
    const closestY = Math.max(r.y, Math.min(this.center.y, r.y + r.height));
    const dx = this.center.x - closestX;
    const dy = this.center.y - closestY;
    return (dx * dx + dy * dy) < (this.radius * this.radius);
  }

  clone(): Entity {
    const c = new CircleEntity({ ...this.center }, this.radius, this.layer);
    c.color = this.color;
    c.lineType = this.lineType;
    c.lineWidth = this.lineWidth;
    return c;
  }

  toJSON(): any {
    return { id: this.id, type: this.type, layer: this.layer, color: this.color, lineType: this.lineType, lineWidth: this.lineWidth, center: this.center, radius: this.radius };
  }

  fromJSON(data: any): void {
    this.id = data.id; this.layer = data.layer; this.color = data.color;
    this.lineType = data.lineType; this.lineWidth = data.lineWidth;
    this.center = data.center; this.radius = data.radius;
  }
}
