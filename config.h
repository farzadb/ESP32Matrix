const char* configPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, viewport-fit=cover, user-scalable=no">
  <title>Matrix Config</title>
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
      color: #e0d7ff;
      padding: 32px 16px;
    }

    body::before {
      content: '';
      position: fixed;
      inset: 0;
      background: radial-gradient(ellipse 60% 40% at 50% 40%, rgba(99,60,180,0.25) 0%, transparent 70%);
      pointer-events: none;
    }

    h1 {
      font-size: 1.1rem;
      font-weight: 600;
      letter-spacing: 0.18em;
      text-transform: uppercase;
      margin: 0 0 32px;
      opacity: 0.7;
    }

    .card {
      background: #1a1730;
      border: 1px solid rgba(255,255,255,0.06);
      border-radius: 16px;
      padding: 28px 24px;
      width: 100%;
      max-width: 400px;
      display: flex;
      flex-direction: column;
      gap: 28px;
      position: relative;
    }

    .section-label {
      font-size: 0.72rem;
      font-weight: 700;
      letter-spacing: 0.14em;
      text-transform: uppercase;
      color: rgba(224,215,255,0.45);
      margin-bottom: 14px;
    }

    .field {
      display: flex;
      flex-direction: column;
      gap: 10px;
    }

    .field-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .field-header .field-label {
      font-size: 0.82rem;
      font-weight: 600;
      letter-spacing: 0.06em;
      color: rgba(224,215,255,0.75);
    }

    .field-header .field-value {
      font-size: 0.85rem;
      font-weight: 700;
      color: #c8b8ff;
      min-width: 60px;
      text-align: right;
    }

    input[type=range] {
      -webkit-appearance: none;
      width: 100%;
      height: 6px;
      border-radius: 3px;
      background: #2e2a45;
      outline: none;
    }

    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 22px;
      height: 22px;
      border-radius: 50%;
      background: #7c4dff;
      cursor: pointer;
      box-shadow: 0 0 10px rgba(124,77,255,0.55);
      transition: box-shadow 0.1s;
    }

    input[type=range]::-webkit-slider-thumb:hover {
      box-shadow: 0 0 16px rgba(124,77,255,0.8);
    }

    .volts-group {
      display: flex;
      gap: 8px;
    }

    .volts-group input[type=radio] { display: none; }

    .volts-group label {
      flex: 1;
      text-align: center;
      padding: 10px 0;
      border-radius: 10px;
      background: #2e2a45;
      cursor: pointer;
      font-size: 0.88rem;
      font-weight: 600;
      color: rgba(224,215,255,0.45);
      border: 1px solid transparent;
      transition: background 0.15s, color 0.15s, box-shadow 0.15s;
      user-select: none;
    }

    .volts-group input[type=radio]:checked + label {
      background: #3d2f6e;
      border-color: rgba(124,77,255,0.5);
      color: #fff;
      box-shadow: 0 0 12px rgba(124,77,255,0.35);
    }

    .divider {
      height: 1px;
      background: rgba(255,255,255,0.05);
      margin: 0 -4px;
    }

    .save-btn {
      border: none;
      cursor: pointer;
      width: 100%;
      height: 50px;
      border-radius: 12px;
      background: linear-gradient(160deg, #7c4dff, #512da8);
      color: #fff;
      font-size: 0.88rem;
      font-weight: 700;
      letter-spacing: 0.12em;
      text-transform: uppercase;
      box-shadow: 0 4px 0 rgba(0,0,0,0.4), 0 0 0 1px rgba(255,255,255,0.05);
      transition: transform 0.08s, box-shadow 0.08s;
      -webkit-tap-highlight-color: transparent;
    }

    .save-btn:active {
      transform: translateY(3px);
      box-shadow: 0 1px 0 rgba(0,0,0,0.4);
    }

    .save-btn:disabled {
      opacity: 0.5;
      cursor: not-allowed;
    }

    .toast {
      margin-top: 18px;
      font-size: 0.75rem;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      height: 18px;
      color: #69f0ae;
      opacity: 0;
      transition: opacity 0.3s;
    }

    .toast.visible { opacity: 1; }
    .toast.err { color: #ff5252; }

    .back {
      margin-top: 28px;
      font-size: 0.72rem;
      color: rgba(224,215,255,0.3);
      text-decoration: none;
      letter-spacing: 0.1em;
      text-transform: uppercase;
      transition: color 0.15s;
    }

    .back:hover { color: rgba(224,215,255,0.6); }
  </style>
</head>
<body>
  <h1>Matrix Config</h1>

  <div class="card">

    <!-- Brightness -->
    <div class="field">
      <div class="field-header">
        <span class="field-label">Brightness</span>
        <span class="field-value" id="val-brightness">–</span>
      </div>
      <input type="range" id="brightness" min="0" max="255" step="1">
    </div>

    <div class="divider"></div>

    <!-- Max current -->
    <div class="field">
      <div class="field-header">
        <span class="field-label">Max Current</span>
        <span class="field-value" id="val-milliamps">–</span>
      </div>
      <input type="range" id="milliamps" min="500" max="5000" step="100">
    </div>

    <div class="divider"></div>

    <!-- Supply voltage -->
    <div class="field">
      <div class="field-header">
        <span class="field-label">Supply Voltage</span>
      </div>
      <div class="volts-group">
        <input type="radio" name="volts" id="v3" value="3">
        <label for="v3">3.3 V</label>
        <input type="radio" name="volts" id="v4" value="4">
        <label for="v4">4 V</label>
        <input type="radio" name="volts" id="v5" value="5">
        <label for="v5">5 V</label>
      </div>
    </div>

    <button class="save-btn" id="save-btn" onclick="save()">Save Settings</button>
  </div>

  <div id="toast" class="toast"></div>
  <a href="/" class="back">← Controller</a>

  <script>
    const bSlider = document.getElementById('brightness');
    const mSlider = document.getElementById('milliamps');
    const bVal    = document.getElementById('val-brightness');
    const mVal    = document.getElementById('val-milliamps');
    const saveBtn = document.getElementById('save-btn');

    bSlider.addEventListener('input', () => bVal.textContent = bSlider.value);
    mSlider.addEventListener('input', () => mVal.textContent = mSlider.value + ' mA');

    async function load() {
      try {
        const r = await fetch('/api/config');
        const d = await r.json();
        bSlider.value = d.brightness;
        bVal.textContent = d.brightness;
        mSlider.value = d.milliamps;
        mVal.textContent = d.milliamps + ' mA';
        const radio = document.querySelector(`input[name="volts"][value="${d.volts}"]`);
        if (radio) radio.checked = true;
      } catch (e) {
        showToast('Failed to load settings', true);
      }
    }

    async function save() {
      const voltsEl = document.querySelector('input[name="volts"]:checked');
      const params = new URLSearchParams({
        brightness: bSlider.value,
        milliamps:  mSlider.value,
        volts:      voltsEl ? voltsEl.value : '5'
      });
      saveBtn.disabled = true;
      try {
        const r = await fetch('/api/config', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: params.toString()
        });
        showToast(r.ok ? 'Settings saved!' : 'Save failed', !r.ok);
      } catch (e) {
        showToast('Save failed', true);
      }
      saveBtn.disabled = false;
    }

    function showToast(msg, isErr = false) {
      const t = document.getElementById('toast');
      t.textContent = msg;
      t.className = 'toast visible' + (isErr ? ' err' : '');
      setTimeout(() => { t.className = 'toast'; }, 2500);
    }

    load();
  </script>
</body>
</html>
)rawliteral";
