'use strict';

const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const test = require('node:test');
const core = require('../level-editor/editor-core.js');

const levelsDir = path.join(__dirname, '..', 'level-editor', 'levels');

test('level manifest exists and each listed file is valid', () => {
  const manifestPath = path.join(levelsDir, 'index.json');
  const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));

  assert.ok(Array.isArray(manifest), 'manifest should be an array');
  assert.ok(manifest.length > 0, 'manifest should include at least one level');

  for (const file of manifest) {
    const levelPath = path.join(levelsDir, file);
    assert.ok(fs.existsSync(levelPath), `missing level file ${file}`);
    const parsed = core.parseLevel(fs.readFileSync(levelPath, 'utf8'));
    assert.ok(core.validateLevel(parsed));
  }
});
