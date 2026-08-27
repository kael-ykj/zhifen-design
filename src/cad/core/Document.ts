import { Layer } from './Layer';
import { Entity } from '../entities/Entity';
import { Point, Rect, COLORS } from './types';

export interface DocumentState {
  name: string;
  units: string;
  limits: Rect;
  currentLayer: string;
  layers: Record<string, Layer>;
  entityCount: number;
}

export class Document {
  name: string = '未命名';
  units: string = 'mm';
  limits: Rect = { x: 0, y: 0, width: 420, height: 297 };
  currentLayer: string = '0';
  layers: Map<string, Layer> = new Map();
  modified: boolean = false;
  filePath: string = '';

  constructor() {
    // 默认图层
    this.addLayer(new Layer('0', COLORS.white));
    this.addLayer(new Layer('DEFPOINTS', COLORS.white));
    this.addLayer(new Layer('墙体', COLORS.gray));
    this.addLayer(new Layer('门窗', COLORS.yellow));
    this.addLayer(new Layer('天线', COLORS.red));
    this.addLayer(new Layer('器件', COLORS.blue));
    this.addLayer(new Layer('馈线', COLORS.white));
    this.addLayer(new Layer('光纤', COLORS.orange));
    this.addLayer(new Layer('标注', COLORS.cyan));
    this.addLayer(new Layer('文字', COLORS.green));
  }

  addLayer(layer: Layer): void {
    this.layers.set(layer.name, layer);
  }

  getLayer(name: string): Layer | undefined {
    return this.layers.get(name);
  }

  getCurrentLayer(): Layer {
    return this.layers.get(this.currentLayer) || this.layers.get('0')!;
  }

  setCurrentLayer(name: string): boolean {
    if (this.layers.has(name)) {
      this.currentLayer = name;
      return true;
    }
    return false;
  }

  getAllLayers(): Layer[] {
    return Array.from(this.layers.values());
  }

  addEntity(entity: Entity): void {
    const layer = this.getLayer(entity.layer);
    if (layer) {
      layer.addEntity(entity);
    } else {
      entity.layer = '0';
      this.getLayer('0')!.addEntity(entity);
    }
    this.modified = true;
  }

  removeEntity(id: string): boolean {
    for (const layer of this.layers.values()) {
      if (layer.removeEntity(id)) {
        this.modified = true;
        return true;
      }
    }
    return false;
  }

  getEntity(id: string): Entity | undefined {
    for (const layer of this.layers.values()) {
      const entity = layer.getEntity(id);
      if (entity) return entity;
    }
    return undefined;
  }

  getAllEntities(): Entity[] {
    const entities: Entity[] = [];
    for (const layer of this.layers.values()) {
      if (layer.visible && !layer.frozen) {
        entities.push(...layer.entities);
      }
    }
    return entities;
  }

  getEntitiesInRect(r: Rect, fullyInside: boolean = false): Entity[] {
    const result: Entity[] = [];
    for (const entity of this.getAllEntities()) {
      if (!entity.isSelectable()) continue;
      if (fullyInside) {
        const bounds = entity.getBounds();
        if (bounds.x >= r.x && bounds.y >= r.y &&
            bounds.x + bounds.width <= r.x + r.width &&
            bounds.y + bounds.height <= r.y + r.height) {
          result.push(entity);
        }
      } else {
        if (entity.intersectsRect(r)) {
          result.push(entity);
        }
      }
    }
    return result;
  }

  getEntityAtPoint(p: Point, tolerance: number = 5): Entity | undefined {
    let closest: Entity | undefined;
    let minDist = tolerance;
    for (const entity of this.getAllEntities()) {
      if (!entity.isSelectable()) continue;
      const d = entity.distanceToPoint(p);
      if (d < minDist) {
        minDist = d;
        closest = entity;
      }
    }
    return closest;
  }

  clearSelection(): void {
    for (const entity of this.getAllEntities()) {
      entity.selected = false;
    }
  }

  getSelectedEntities(): Entity[] {
    return this.getAllEntities().filter(e => e.selected);
  }

  getExtents(): Rect {
    const entities = this.getAllEntities();
    if (entities.length === 0) {
      return { x: -100, y: -100, width: 200, height: 200 };
    }
    let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
    for (const e of entities) {
      const b = e.getBounds();
      minX = Math.min(minX, b.x);
      minY = Math.min(minY, b.y);
      maxX = Math.max(maxX, b.x + b.width);
      maxY = Math.max(maxY, b.y + b.height);
    }
    const padding = Math.max((maxX - minX), (maxY - minY)) * 0.1;
    return {
      x: minX - padding,
      y: minY - padding,
      width: (maxX - minX) + padding * 2,
      height: (maxY - minY) + padding * 2,
    };
  }

  toJSON(): DocumentState {
    const layers: Record<string, Layer> = {};
    for (const [name, layer] of this.layers) {
      layers[name] = layer;
    }
    return {
      name: this.name,
      units: this.units,
      limits: this.limits,
      currentLayer: this.currentLayer,
      layers,
      entityCount: this.getAllEntities().length,
    };
  }
}
