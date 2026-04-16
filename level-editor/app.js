(function () {
  'use strict';

  var core = window.LevelEditorCore;
  var dom = {
    levelSelect: document.getElementById('levelSelect'),
    newLevelBtn: document.getElementById('newLevelBtn'),
    widthInput: document.getElementById('widthInput'),
    heightInput: document.getElementById('heightInput'),
    resizeBtn: document.getElementById('resizeBtn'),
    tilePicker: document.getElementById('tilePicker'),
    spawnMode: document.getElementById('spawnMode'),
    gridHost: document.getElementById('gridHost'),
    jsonEditor: document.getElementById('jsonEditor'),
    importBtn: document.getElementById('importBtn'),
    exportBtn: document.getElementById('exportBtn'),
    status: document.getElementById('status')
  };

  var appState = {
    manifest: [],
    activeLevelFile: '',
    level: null
  };

  function setStatus(message, isError) {
    dom.status.textContent = message;
    dom.status.className = isError ? 'error' : 'ok';
  }

  function loadManifest() {
    return fetch('levels/index.json')
      .then(function (res) {
        if (!res.ok) {
          throw new Error('Unable to load levels/index.json (' + res.status + ')');
        }
        return res.json();
      })
      .then(function (manifest) {
        if (!Array.isArray(manifest)) {
          throw new Error('levels/index.json must be an array of filenames.');
        }
        appState.manifest = manifest;
        populateLevelSelect();
        if (manifest.length) {
          return loadLevelFile(manifest[0]);
        }
        createNewLevel();
        return null;
      });
  }

  function populateLevelSelect() {
    dom.levelSelect.innerHTML = '';
    appState.manifest.forEach(function (filename) {
      var option = document.createElement('option');
      option.value = filename;
      option.textContent = filename;
      dom.levelSelect.appendChild(option);
    });
  }

  function loadLevelFile(filename) {
    return fetch('levels/' + filename)
      .then(function (res) {
        if (!res.ok) {
          throw new Error('Unable to load levels/' + filename + ' (' + res.status + ')');
        }
        return res.text();
      })
      .then(function (text) {
        appState.level = core.parseLevel(text);
        appState.activeLevelFile = filename;
        render();
        setStatus('Loaded ' + filename + '.');
      });
  }

  function createNewLevel() {
    appState.level = core.normalizeLevel({
      id: 'new-level',
      name: 'New Level',
      width: 16,
      height: 12,
      spawn: { x: 0, y: 0 }
    });
    appState.activeLevelFile = 'new-level.json';
    render();
    setStatus('Created an unsaved level. Export it into level-editor/levels/.');
  }

  function resizeLevel(newWidth, newHeight) {
    var next = core.normalizeLevel({
      id: appState.level.id,
      name: appState.level.name,
      width: newWidth,
      height: newHeight,
      spawn: appState.level.spawn,
      tiles: appState.level.tiles,
      metadata: appState.level.metadata
    });
    appState.level = next;
    render();
    setStatus('Resized to ' + newWidth + 'x' + newHeight + '.');
  }

  function updateFromJsonEditor() {
    try {
      appState.level = core.parseLevel(dom.jsonEditor.value);
      renderGrid();
      syncInputs();
      setStatus('Imported JSON into the editor state.');
    } catch (err) {
      setStatus(err.message, true);
    }
  }

  function exportJson() {
    try {
      var json = core.serializeLevel(appState.level);
      var blob = new Blob([json], { type: 'application/json' });
      var link = document.createElement('a');
      var levelName = appState.activeLevelFile || (appState.level.id + '.json');
      link.href = URL.createObjectURL(blob);
      link.download = levelName;
      document.body.appendChild(link);
      link.click();
      link.remove();
      URL.revokeObjectURL(link.href);
      setStatus('Exported ' + levelName + '. Move it into level-editor/levels/.');
    } catch (err) {
      setStatus(err.message, true);
    }
  }

  function paintCell(x, y) {
    if (!appState.level) {
      return;
    }
    if (dom.spawnMode.checked) {
      appState.level.spawn.x = x;
      appState.level.spawn.y = y;
      renderGrid();
      renderJson();
      setStatus('Set spawn to (' + x + ', ' + y + ').');
      return;
    }

    var tileValue = parseInt(dom.tilePicker.value, 10);
    if (!Number.isInteger(tileValue) || tileValue < 0) {
      setStatus('Tile value must be a non-negative integer.', true);
      return;
    }

    appState.level.tiles[y][x] = tileValue;
    renderGrid();
    renderJson();
  }

  function renderGrid() {
    dom.gridHost.innerHTML = '';
    dom.gridHost.style.gridTemplateColumns = 'repeat(' + appState.level.width + ', 28px)';

    for (var y = 0; y < appState.level.height; y += 1) {
      for (var x = 0; x < appState.level.width; x += 1) {
        var btn = document.createElement('button');
        var tile = appState.level.tiles[y][x];
        btn.type = 'button';
        btn.className = 'cell';
        btn.textContent = String(tile);
        btn.title = 'x=' + x + ', y=' + y;
        if (appState.level.spawn.x === x && appState.level.spawn.y === y) {
          btn.classList.add('spawn');
        }
        btn.addEventListener('click', paintCell.bind(null, x, y));
        dom.gridHost.appendChild(btn);
      }
    }
  }

  function renderJson() {
    dom.jsonEditor.value = core.serializeLevel(appState.level);
  }

  function syncInputs() {
    dom.widthInput.value = String(appState.level.width);
    dom.heightInput.value = String(appState.level.height);
  }

  function render() {
    renderGrid();
    renderJson();
    syncInputs();
  }

  dom.levelSelect.addEventListener('change', function () {
    loadLevelFile(dom.levelSelect.value).catch(function (err) {
      setStatus(err.message, true);
    });
  });

  dom.newLevelBtn.addEventListener('click', createNewLevel);

  dom.resizeBtn.addEventListener('click', function () {
    var width = parseInt(dom.widthInput.value, 10);
    var height = parseInt(dom.heightInput.value, 10);
    if (!Number.isInteger(width) || !Number.isInteger(height) || width <= 0 || height <= 0) {
      setStatus('Width and height must be positive integers.', true);
      return;
    }
    resizeLevel(width, height);
  });

  dom.importBtn.addEventListener('click', updateFromJsonEditor);
  dom.exportBtn.addEventListener('click', exportJson);

  loadManifest().catch(function (err) {
    createNewLevel();
    setStatus(err.message + ' Create level files in level-editor/levels/.', true);
  });
})();
