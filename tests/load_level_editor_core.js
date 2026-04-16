'use strict';

const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

function extractScriptById(html, id) {
  const escaped = id.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  const re = new RegExp(`<script[^>]*id=["']${escaped}["'][^>]*>([\\s\\S]*?)</script>`, 'i');
  const match = html.match(re);
  if (!match) {
    throw new Error(`Could not find inline script with id="${id}"`);
  }
  return match[1];
}

function loadCoreFromHtml() {
  const htmlPath = path.join(__dirname, '..', 'level-editor', 'index.html');
  const html = fs.readFileSync(htmlPath, 'utf8');
  const coreScript = extractScriptById(html, 'level-editor-core');
  const context = vm.createContext({ window: {}, globalThis: {} });
  vm.runInContext(coreScript, context, { filename: 'level-editor-core.inline.js' });
  const core = context.window.LevelEditorCore || context.globalThis.LevelEditorCore;
  if (!core) {
    throw new Error('LevelEditorCore was not initialized by the inline script.');
  }
  return { html, core };
}

module.exports = { loadCoreFromHtml };
