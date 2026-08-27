import { Document } from '../core/Document';
import { Point, point } from '../core/types';
import { LineEntity } from '../entities/LineEntity';
import { CircleEntity } from '../entities/CircleEntity';
import { ArcEntity } from '../entities/ArcEntity';
import { PolylineEntity } from '../entities/PolylineEntity';
import { TextEntity } from '../entities/TextEntity';
import { Layer } from '../core/Layer';

interface DxfPair {
  code: number;
  value: string;
}

export class DxfReader {
  document: Document;

  constructor(document: Document) {
    this.document = document;
  }

  read(content: string): Document {
    const pairs = this.parsePairs(content);
    this.readLayers(pairs);
    this.readEntities(pairs);
    return this.document;
  }

  private parsePairs(content: string): DxfPair[] {
    const lines = content.split(/\r?\n/);
    const pairs: DxfPair[] = [];
    for (let i = 0; i < lines.length - 1; i += 2) {
      const code = parseInt(lines[i].trim());
      const value = lines[i + 1] ? lines[i + 1].trim() : '';
      if (!isNaN(code)) {
        pairs.push({ code, value });
      }
    }
    return pairs;
  }

  private readLayers(pairs: DxfPair[]): void {
    let inLayerTable = false;
    let currentLayer: Layer | null = null;
    for (let i = 0; i < pairs.length; i++) {
      const p = pairs[i];
      if (p.code === 2 && p.value === 'LAYER') {
        inLayerTable = true;
        continue;
      }
      if (inLayerTable && p.code === 0 && p.value === 'LAYER') {
        currentLayer = new Layer('0', '#FFFFFF');
        continue;
      }
      if (currentLayer) {
        if (p.code === 2) currentLayer.name = p.value;
        if (p.code === 62) currentLayer.color = this.aciToColor(parseInt(p.value));
        if (p.code === 6) currentLayer.lineType = p.value;
        if (p.code === 70) {
          const flags = parseInt(p.value);
          currentLayer.frozen = (flags & 1) !== 0;
          currentLayer.visible = (flags & 2) === 0;
        }
        if (p.code === 0 && p.value === 'ENDTAB') {
          if (currentLayer.name !== '0') this.document.addLayer(currentLayer);
          currentLayer = null;
          inLayerTable = false;
        }
      }
    }
  }

  private readEntities(pairs: DxfPair[]): void {
    let inEntities = false;
    let currentEntity: any = null;
    let vertices: any[] = [];
    for (let i = 0; i < pairs.length; i++) {
      const p = pairs[i];
      if (p.code === 2 && p.value === 'ENTITIES') {
        inEntities = true;
        continue;
      }
      if (!inEntities) continue;
      if (p.code === 0 && p.value === 'ENDSEC') break;

      if (p.code === 0) {
        // 保存上一个实体
        if (currentEntity) {
          this.addEntity(currentEntity, vertices);
        }
        vertices = [];
        currentEntity = { type: p.value, layer: '0' };
        continue;
      }

      if (!currentEntity) continue;

      switch (currentEntity.type) {
        case 'LINE':
          if (p.code === 8) currentEntity.layer = p.value;
          if (p.code === 10) currentEntity.x1 = parseFloat(p.value);
          if (p.code === 20) currentEntity.y1 = parseFloat(p.value);
          if (p.code === 11) currentEntity.x2 = parseFloat(p.value);
          if (p.code === 21) currentEntity.y2 = parseFloat(p.value);
          break;
        case 'CIRCLE':
          if (p.code === 8) currentEntity.layer = p.value;
          if (p.code === 10) currentEntity.cx = parseFloat(p.value);
          if (p.code === 20) currentEntity.cy = parseFloat(p.value);
          if (p.code === 40) currentEntity.r = parseFloat(p.value);
          break;
        case 'ARC':
          if (p.code === 8) currentEntity.layer = p.value;
          if (p.code === 10) currentEntity.cx = parseFloat(p.value);
          if (p.code === 20) currentEntity.cy = parseFloat(p.value);
          if (p.code === 40) currentEntity.r = parseFloat(p.value);
          if (p.code === 50) currentEntity.startAngle = parseFloat(p.value) * Math.PI / 180;
          if (p.code === 51) currentEntity.endAngle = parseFloat(p.value) * Math.PI / 180;
          break;
        case 'POLYLINE':
          if (p.code === 8) currentEntity.layer = p.value;
          if (p.code === 70) currentEntity.closed = (parseInt(p.value) & 1) !== 0;
          break;
        case 'VERTEX':
          if (p.code === 10) vertices.push({ x: parseFloat(p.value), y: 0 });
          if (p.code === 20 && vertices.length > 0) vertices[vertices.length - 1].y = parseFloat(p.value);
          break;
        case 'TEXT':
          if (p.code === 8) currentEntity.layer = p.value;
          if (p.code === 10) currentEntity.x = parseFloat(p.value);
          if (p.code === 20) currentEntity.y = parseFloat(p.value);
          if (p.code === 40) currentEntity.height = parseFloat(p.value);
          if (p.code === 1) currentEntity.text = p.value;
          if (p.code === 50) currentEntity.rotation = parseFloat(p.value) * Math.PI / 180;
          break;
      }
    }
    if (currentEntity) {
      this.addEntity(currentEntity, vertices);
    }
  }

  private addEntity(data: any, vertices: any[]): void {
    try {
      switch (data.type) {
        case 'LINE':
          if (data.x1 !== undefined) {
            const line = new LineEntity(point(data.x1, data.y1 || 0), point(data.x2, data.y2 || 0), data.layer);
            this.document.addEntity(line);
          }
          break;
        case 'CIRCLE':
          if (data.cx !== undefined) {
            const circle = new CircleEntity(point(data.cx, data.cy || 0), data.r || 1, data.layer);
            this.document.addEntity(circle);
          }
          break;
        case 'ARC':
          if (data.cx !== undefined) {
            const arc = new ArcEntity(point(data.cx, data.cy || 0), data.r || 1, data.startAngle || 0, data.endAngle || Math.PI, data.layer);
            this.document.addEntity(arc);
          }
          break;
        case 'POLYLINE':
          if (vertices.length >= 2) {
            const pts = vertices.map(v => point(v.x, v.y));
            const poly = new PolylineEntity(pts, data.closed || false, data.layer);
            this.document.addEntity(poly);
          }
          break;
        case 'TEXT':
          if (data.x !== undefined) {
            const text = new TextEntity(point(data.x, data.y || 0), data.text || '', data.height || 2.5, data.layer);
            if (data.rotation) text.rotation = data.rotation;
            this.document.addEntity(text);
          }
          break;
      }
    } catch (e) {
      // 忽略解析错误
    }
  }

  private aciToColor(aci: number): string {
    const colors: Record<number, string> = {
      1: '#FF0000', 2: '#FFFF00', 3: '#00FF00', 4: '#00FFFF',
      5: '#0000FF', 6: '#FF00FF', 7: '#FFFFFF', 8: '#808080', 9: '#C0C0C0',
    };
    return colors[aci] || '#FFFFFF';
  }
}
