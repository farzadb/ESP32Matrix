const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, viewport-fit=cover, user-scalable=no">
  <title>ESP32 Matrix Controller</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; }

    body {
      margin: 0;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      background: #0f0f1a;
      font-family: 'Segoe UI', system-ui, sans-serif;
      touch-action: manipulation;
      user-select: none;
      -webkit-user-select: none;
    }

    /* Ambient glow behind the pad */
    body::before {
      content: '';
      position: fixed;
      inset: 0;
      background:
        radial-gradient(ellipse 60% 40% at 50% 60%, rgba(99,60,180,0.25) 0%, transparent 70%);
      pointer-events: none;
    }

    h1 {
      color: #e0d7ff;
      font-size: 1.1rem;
      font-weight: 600;
      letter-spacing: 0.18em;
      text-transform: uppercase;
      margin: 0 0 32px;
      opacity: 0.7;
    }

    /* D-pad */
    .dpad {
      display: grid;
      grid-template-areas:
        ".    up    ."
        "left mid  right"
        ".    down  .";
      grid-template-columns: 80px 80px 80px;
      grid-template-rows: 80px 80px 80px;
      gap: 6px;
    }

    .dpad-mid {
      grid-area: mid;
      background: #1e1b2e;
      border-radius: 12px;
    }

    .btn {
      border: none;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 26px;
      color: #d0c8ff;
      background: #1e1b2e;
      border-radius: 12px;
      box-shadow:
        0 4px 0 #0a0914,
        0 0 0 1px rgba(255,255,255,0.05);
      transition: box-shadow 0.08s, transform 0.08s, background 0.08s;
      -webkit-tap-highlight-color: transparent;
    }

    .btn-up    { grid-area: up; }
    .btn-left  { grid-area: left; }
    .btn-right { grid-area: right; }
    .btn-down  { grid-area: down; }

    .btn:hover {
      background: #26213d;
    }

    .btn.btn-active, .btn:active {
      background: #3d2f6e;
      box-shadow:
        0 0 0 #0a0914,
        0 0 0 1px rgba(255,255,255,0.08),
        0 0 16px rgba(140,100,255,0.4);
      transform: translateY(3px);
      color: #fff;
    }

    /* Action buttons row */
    .actions {
      display: flex;
      gap: 16px;
      margin-top: 36px;
    }

    .action-btn {
      border: none;
      cursor: pointer;
      padding: 0;
      width: 110px;
      height: 48px;
      border-radius: 24px;
      font-size: 0.85rem;
      font-weight: 700;
      letter-spacing: 0.1em;
      text-transform: uppercase;
      color: #fff;
      box-shadow: 0 4px 0 rgba(0,0,0,0.4);
      transition: transform 0.08s, box-shadow 0.08s;
      -webkit-tap-highlight-color: transparent;
    }

    .action-btn:active {
      transform: translateY(3px);
      box-shadow: 0 1px 0 rgba(0,0,0,0.4);
    }

    #btn-menu  { background: linear-gradient(160deg, #7c4dff, #512da8); }
    #btn-reset { background: linear-gradient(160deg, #f06292, #ad1457); }

    /* Connection pill */
    .status {
      margin-top: 28px;
      display: flex;
      align-items: center;
      gap: 7px;
      font-size: 0.72rem;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      color: rgba(255,255,255,0.3);
    }

    .status-dot {
      width: 7px;
      height: 7px;
      border-radius: 50%;
      background: #555;
      transition: background 0.3s;
    }

    .status-dot.connected { background: #69f0ae; box-shadow: 0 0 6px #69f0ae; }
    .status-dot.error     { background: #ff5252; box-shadow: 0 0 6px #ff5252; }

    .config-link {
      margin-top: 18px;
      font-size: 0.72rem;
      color: rgba(255,255,255,0.2);
      text-decoration: none;
      letter-spacing: 0.1em;
      text-transform: uppercase;
      transition: color 0.15s;
    }
    .config-link:hover { color: rgba(255,255,255,0.45); }

    @media (max-width: 380px) {
      .dpad {
        grid-template-columns: 72px 72px 72px;
        grid-template-rows: 72px 72px 72px;
      }
      .btn { font-size: 22px; }
    }
  </style>
</head>
<body>
  <h1>Matrix Controller</h1>

  <div class="dpad">
    <button id="btn-up"    class="btn btn-up">▲</button>
    <button id="btn-left"  class="btn btn-left">◀</button>
    <div class="dpad-mid"></div>
    <button id="btn-right" class="btn btn-right">▶</button>
    <button id="btn-down"  class="btn btn-down">▼</button>
  </div>

  <div class="actions">
    <button id="btn-menu"  class="action-btn" onclick="sendCommand('m', 'btn-menu')">Menu</button>
    <button id="btn-reset" class="action-btn" onclick="sendCommand('r', 'btn-reset')">Reset</button>
  </div>

  <div class="status">
    <div id="status-dot" class="status-dot"></div>
    <span id="status-text">Connecting…</span>
  </div>

  <a href="/config" class="config-link">⚙ Config</a>

  <script>
    let ws = new WebSocket(`ws://${location.hostname}:81/`);

    const dot  = document.getElementById('status-dot');
    const text = document.getElementById('status-text');

    ws.onopen    = () => { dot.className = 'status-dot connected'; text.textContent = 'Connected'; };
    ws.onclose   = () => { dot.className = 'status-dot error';     text.textContent = 'Disconnected'; };
    ws.onerror   = () => { dot.className = 'status-dot error';     text.textContent = 'Error'; };

    const INITIAL_DELAY_MS  = 300;  // pause before repeat starts
    const REPEAT_INTERVAL_MS = 80;   // repeat rate once held
    let repeatTimeouts  = {};
    let repeatIntervals = {};

    function sendCommand(command, buttonId) {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(command);
      }
      if (buttonId) {
        document.getElementById(buttonId)?.classList.add('btn-active');
      }
    }

    function startRepeat(command, buttonId) {
      if (repeatTimeouts[command] || repeatIntervals[command]) return;
      sendCommand(command, buttonId);
      // Wait before starting rapid repeat so a normal tap sends exactly once
      repeatTimeouts[command] = setTimeout(() => {
        delete repeatTimeouts[command];
        repeatIntervals[command] = setInterval(() => sendCommand(command, buttonId), REPEAT_INTERVAL_MS);
      }, INITIAL_DELAY_MS);
    }

    function stopRepeat(command, buttonId) {
      clearTimeout(repeatTimeouts[command]);
      clearInterval(repeatIntervals[command]);
      delete repeatTimeouts[command];
      delete repeatIntervals[command];
      document.getElementById(buttonId)?.classList.remove('btn-active');
    }

    // Direction buttons: hold-to-repeat
    const dirButtons = [
      { id: 'btn-up',    cmd: 'w' },
      { id: 'btn-left',  cmd: 'a' },
      { id: 'btn-right', cmd: 'd' },
      { id: 'btn-down',  cmd: 's' },
    ];
    dirButtons.forEach(({ id, cmd }) => {
      let btn = document.getElementById(id);
      btn.addEventListener('pointerdown', (e) => { e.preventDefault(); startRepeat(cmd, id); });
      btn.addEventListener('pointerup',   () => stopRepeat(cmd, id));
      btn.addEventListener('pointerleave',() => stopRepeat(cmd, id));
    });

    // Keyboard: hold-to-repeat for directions, single-fire for others
    const keyMap = {
      'ArrowUp': 'w', 'w': 'w',
      'ArrowLeft': 'a', 'a': 'a',
      'ArrowDown': 's', 's': 's',
      'ArrowRight': 'd', 'd': 'd',
    };
    const buttonMap = { 'w': 'btn-up', 'a': 'btn-left', 's': 'btn-down', 'd': 'btn-right' };

    document.addEventListener('keydown', function(event) {
      if (event.repeat) return;
      let command = keyMap[event.key];
      if (command) {
        startRepeat(command, buttonMap[command]);
      } else if (event.key === 'm') {
        sendCommand('m', 'btn-menu');
      } else if (event.key === 'r') {
        sendCommand('r', 'btn-reset');
      }
    });

    document.addEventListener('keyup', function(event) {
      let command = keyMap[event.key];
      if (command) stopRepeat(command, buttonMap[command]);
    });
  </script>
</body>
</html>
)rawliteral";
