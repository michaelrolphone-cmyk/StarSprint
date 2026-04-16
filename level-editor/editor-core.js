(function (globalScope) {
  'use strict';

  var DEFAULT_TILE = 0;

  function deepClone(value) {
    return JSON.parse(JSON.stringify(value));
  }

  function createGrid(width, height, fill) {
    var grid = [];
    for (var y = 0; y < height; y += 1) {
      var row = [];
      for (var x = 0; x < width; x += 1) {
        row.push(fill);
      }
      grid.push(row);
    }
    return grid;
  }

  function normalizeLevel(rawLevel) {
    var level = rawLevel || {};
    var id = typeof level.id === 'string' && level.id.trim() ? level.id.trim() : 'level';
    var width = Number.isInteger(level.width) && level.width > 0 ? level.width : 16;
    var height = Number.isInteger(level.height) && level.height > 0 ? level.height : 12;
    var spawn = level.spawn || {};

    var tiles = createGrid(width, height, DEFAULT_TILE);
    if (Array.isArray(level.tiles)) {
      for (var y = 0; y < height; y += 1) {
        if (!Array.isArray(level.tiles[y])) {
          continue;
        }
        for (var x = 0; x < width; x += 1) {
          var candidate = level.tiles[y][x];
          tiles[y][x] = Number.isInteger(candidate) && candidate >= 0 ? candidate : DEFAULT_TILE;
        }
      }
    }

    return {
      id: id,
      name: typeof level.name === 'string' && level.name.trim() ? level.name.trim() : id,
      width: width,
      height: height,
      spawn: {
        x: Number.isInteger(spawn.x) ? Math.max(0, Math.min(width - 1, spawn.x)) : 0,
        y: Number.isInteger(spawn.y) ? Math.max(0, Math.min(height - 1, spawn.y)) : 0
      },
      tiles: tiles,
      metadata: typeof level.metadata === 'object' && level.metadata !== null ? deepClone(level.metadata) : {}
    };
  }

  function validateLevel(level) {
    if (!level || typeof level !== 'object') {
      throw new Error('Level must be an object.');
    }
    if (!Number.isInteger(level.width) || level.width <= 0) {
      throw new Error('Level width must be a positive integer.');
    }
    if (!Number.isInteger(level.height) || level.height <= 0) {
      throw new Error('Level height must be a positive integer.');
    }
    if (!Array.isArray(level.tiles) || level.tiles.length !== level.height) {
      throw new Error('Tile rows must match level height.');
    }

    for (var y = 0; y < level.height; y += 1) {
      if (!Array.isArray(level.tiles[y]) || level.tiles[y].length !== level.width) {
        throw new Error('Tile columns must match level width.');
      }
      for (var x = 0; x < level.width; x += 1) {
        if (!Number.isInteger(level.tiles[y][x]) || level.tiles[y][x] < 0) {
          throw new Error('Tile values must be non-negative integers.');
        }
      }
    }

    if (!level.spawn || !Number.isInteger(level.spawn.x) || !Number.isInteger(level.spawn.y)) {
      throw new Error('Spawn coordinates must be integers.');
    }
    if (level.spawn.x < 0 || level.spawn.x >= level.width || level.spawn.y < 0 || level.spawn.y >= level.height) {
      throw new Error('Spawn point must be inside the level bounds.');
    }

    return true;
  }

  function serializeLevel(level) {
    validateLevel(level);
    return JSON.stringify(level, null, 2) + '\n';
  }

  function parseLevel(text) {
    var parsed = JSON.parse(text);
    var normalized = normalizeLevel(parsed);
    validateLevel(normalized);
    return normalized;
  }

  var api = {
    DEFAULT_TILE: DEFAULT_TILE,
    createGrid: createGrid,
    normalizeLevel: normalizeLevel,
    validateLevel: validateLevel,
    serializeLevel: serializeLevel,
    parseLevel: parseLevel
  };

  if (typeof module !== 'undefined' && module.exports) {
    module.exports = api;
  }

  globalScope.LevelEditorCore = api;
})(typeof window !== 'undefined' ? window : globalThis);
