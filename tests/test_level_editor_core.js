'use strict';

const assert = require('node:assert/strict');
const test = require('node:test');
const core = require('../level-editor/editor-core.js');

test('normalizeLevel applies defaults and clamps spawn', () => {
  const level = core.normalizeLevel({
    id: ' demo ',
    width: 3,
    height: 2,
    spawn: { x: 9, y: -1 },
    tiles: [[7], [8, 9, 10]]
  });

  assert.equal(level.id, 'demo');
  assert.equal(level.name, 'demo');
  assert.deepEqual(level.spawn, { x: 2, y: 0 });
  assert.deepEqual(level.tiles, [
    [7, 0, 0],
    [8, 9, 10]
  ]);
});

test('validateLevel rejects out-of-bounds spawn', () => {
  const level = core.normalizeLevel({
    width: 4,
    height: 4,
    spawn: { x: 0, y: 0 }
  });
  level.spawn.x = 4;

  assert.throws(() => core.validateLevel(level), /inside the level bounds/);
});

test('serialize/parse round trip preserves shape', () => {
  const original = core.normalizeLevel({
    id: 'roundtrip',
    name: 'Roundtrip',
    width: 5,
    height: 3,
    spawn: { x: 4, y: 2 },
    tiles: [
      [0, 1, 2, 3, 4],
      [4, 3, 2, 1, 0],
      [1, 1, 1, 1, 1]
    ],
    metadata: { world: 1 }
  });

  const json = core.serializeLevel(original);
  const parsed = core.parseLevel(json);

  assert.deepEqual(parsed, original);
});
