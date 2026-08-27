import { Document } from '../core/Document';
import { Entity } from '../entities/Entity';
import { Point, point, distance } from '../core/types';
import { LineEntity } from '../entities/LineEntity';
import { CircleEntity } from '../entities/CircleEntity';
import { ArcEntity } from '../entities/ArcEntity';
import { PolylineEntity } from '../entities/PolylineEntity';
import { RectangleEntity } from '../entities/RectangleEntity';

export type SnapType = 'endpoint' | 'midpoint' | 'center' | 'intersection' | 'perpendicular' | 'nearest' | 'quadrant' | 'tangent';

export interface SnapResult {
  point: Point;
  type: SnapType;
  entity: Entity;
}

export class SnapManager {
  document: Document;
  enabled: boolean = true;
  snapTypes: Set<SnapType> = new Set(['endpoint', 'midpoint', 'center', 'intersection', 'nearest']);
  tolerance: number = 10; // 屏幕像素
  currentSnap: SnapResult | null = null;

  constructor(document: Document) {
    this.document = document;
  }

  setEnabled(enabled: boolean): void {
    this.enabled = enabled;
  }

  toggleType(type: SnapType): void {
    if (this.snapTypes.has(type)) this.snapTypes.delete(type);
    else this.snapTypes.add(type);
  }

  // 计算捕捉点
  computeSnap(worldPoint: Point, screenPoint: Point, viewScale: number): SnapResult | null {
    if (!this.enabled) return null;
    const worldTolerance = this.tolerance / viewScale;
    let best: SnapResult | null = null;
    let bestDist = worldTolerance;

    for (const entity of this.document.getAllEntities()) {
      if (!entity.isSelectable()) continue;

      if (this.snapTypes.has('endpoint')) {
        const endpoints = this.getEndpoints(entity);
        for (const ep of endpoints) {
          const d = distance(worldPoint, ep);
          if (d < bestDist) { bestDist = d; best = { point: ep, type: 'endpoint', entity }; }
        }
      }

      if (this.snapTypes.has('midpoint')) {
        const midpoints = this.getMidpoints(entity);
        for (const mp of midpoints) {
          const d = distance(worldPoint, mp);
          if (d < bestDist) { bestDist = d; best = { point: mp, type: 'midpoint', entity }; }
        }
      }

      if (this.snapTypes.has('center')) {
        if (entity instanceof CircleEntity || entity instanceof ArcEntity) {
          const c = entity.getCenter();
          const d = distance(worldPoint, c);
          if (d < bestDist) { bestDist = d; best = { point: c, type: 'center', entity }; }
        }
      }

      if (this.snapTypes.has('quadrant')) {
        if (entity instanceof CircleEntity) {
          for (const a of [0, Math.PI / 2, Math.PI, Math.PI * 1.5]) {
            const qp = point(entity.center.x + entity.radius * Math.cos(a), entity.center.y + entity.radius * Math.sin(a));
            const d = distance(worldPoint, qp);
            if (d < bestDist) { bestDist = d; best = { point: qp, type: 'quadrant', entity }; }
          }
        }
      }

      if (this.snapTypes.has('nearest')) {
        const nearest = this.getNearestPoint(entity, worldPoint);
        if (nearest) {
          const d = distance(worldPoint, nearest);
          if (d < bestDist) { bestDist = d; best = { point: nearest, type: 'nearest', entity }; }
        }
      }
    }

    this.currentSnap = best;
    return best;
  }

  private getEndpoints(entity: Entity): Point[] {
    if (entity instanceof LineEntity) return [entity.start, entity.end];
    if (entity instanceof ArcEntity) return [entity.getStartPoint(), entity.getEndPoint()];
    if (entity instanceof PolylineEntity) {
      return entity.vertices.map(v => point(v.x, v.y));
    }
    if (entity instanceof RectangleEntity) return entity.getVertices();
    return [];
  }

  private getMidpoints(entity: Entity): Point[] {
    if (entity instanceof LineEntity) return [entity.getCenter()];
    if (entity instanceof PolylineEntity && entity.vertices.length >= 2) {
      const mids: Point[] = [];
      const count = entity.closed ? entity.vertices.length : entity.vertices.length - 1;
      for (let i = 0; i < count; i++) {
        const v1 = entity.vertices[i];
        const v2 = entity.vertices[(i + 1) % entity.vertices.length];
        mids.push(point((v1.x + v2.x) / 2, (v1.y + v2.y) / 2));
      }
      return mids;
    }
    return [];
  }

  private getNearestPoint(entity: Entity, p: Point): Point | null {
    if (entity instanceof LineEntity) {
      return this.nearestOnSegment(p, entity.start, entity.end);
    }
    if (entity instanceof CircleEntity) {
      const a = Math.atan2(p.y - entity.center.y, p.x - entity.center.x);
      return point(entity.center.x + entity.radius * Math.cos(a), entity.center.y + entity.radius * Math.sin(a));
    }
    if (entity instanceof PolylineEntity && entity.vertices.length >= 2) {
      let best: Point | null = null;
      let bestDist = Infinity;
      const count = entity.closed ? entity.vertices.length : entity.vertices.length - 1;
      for (let i = 0; i < count; i++) {
        const v1 = point(entity.vertices[i].x, entity.vertices[i].y);
        const v2 = point(entity.vertices[(i + 1) % entity.vertices.length].x, entity.vertices[(i + 1) % entity.vertices.length].y);
        const np = this.nearestOnSegment(p, v1, v2);
        const d = distance(p, np);
        if (d < bestDist) { bestDist = d; best = np; }
      }
      return best;
    }
    return null;
  }

  private nearestOnSegment(p: Point, a: Point, b: Point): Point {
    const A = p.x - a.x; const B = p.y - a.y;
    const C = b.x - a.x; const D = b.y - a.y;
    const dot = A * C + B * D; const lenSq = C * C + D * D;
    let param = lenSq !== 0 ? dot / lenSq : 0;
    param = Math.max(0, Math.min(1, param));
    return point(a.x + param * C, a.y + param * D);
  }

  // 绘制捕捉标记
  drawSnapMarker(ctx: CanvasRenderingContext2D, screenPoint: Point, type: SnapType): void {
    const size = 8;
    ctx.save();
    ctx.strokeStyle = '#FF00FF';
    ctx.fillStyle = '#FF00FF';
    ctx.lineWidth = 1.5;
    ctx.translate(screenPoint.x, screenPoint.y);

    switch (type) {
      case 'endpoint':
        ctx.beginPath();
        ctx.moveTo(-size, -size); ctx.lineTo(size, -size);
        ctx.lineTo(size, size); ctx.lineTo(-size, size);
        ctx.closePath(); ctx.stroke();
        break;
      case 'midpoint':
        ctx.beginPath();
        ctx.moveTo(0, -size); ctx.lineTo(size, 0);
        ctx.lineTo(0, size); ctx.lineTo(-size, 0);
        ctx.closePath(); ctx.stroke();
        break;
      case 'center':
        ctx.beginPath();
        ctx.arc(0, 0, size, 0, Math.PI * 2);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(-size, 0); ctx.lineTo(size, 0);
        ctx.moveTo(0, -size); ctx.lineTo(0, size);
        ctx.stroke();
        break;
      case 'intersection':
        ctx.beginPath();
        ctx.moveTo(-size, -size); ctx.lineTo(size, size);
        ctx.moveTo(size, -size); ctx.lineTo(-size, size);
        ctx.stroke();
        break;
      case 'nearest':
        ctx.beginPath();
        ctx.moveTo(0, -size); ctx.lineTo(size * 0.87, size * 0.5);
        ctx.lineTo(-size * 0.87, size * 0.5);
        ctx.closePath(); ctx.stroke();
        break;
      case 'quadrant':
        ctx.beginPath();
        ctx.arc(0, 0, size, 0, Math.PI * 2);
        ctx.stroke();
        break;
      default:
        ctx.beginPath();
        ctx.arc(0, 0, size / 2, 0, Math.PI * 2);
        ctx.fill();
    }
    ctx.restore();
  }
}
