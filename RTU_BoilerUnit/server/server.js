const mqtt = require('mqtt');
const fs = require('fs');
const express = require('express');

const app = express();
const PORT = 3000;

/* ===== AWS CONFIG ===== */

const client = mqtt.connect({
  host: 'ay2jqszs03lk1-ats.iot.ap-south-1.amazonaws.com',
  port: 8883,
  protocol: 'mqtts',

  key: fs.readFileSync('../main/certs/client.key'),
  cert: fs.readFileSync('../main/certs/client.crt'),
  ca: fs.readFileSync('../main/certs/root_cert_auth.crt'),

  clientId: 'node_' + Math.random().toString(16).substr(2, 8),
});

let latestData = {};
let lastTimestamp = 0;
let lastUpdateTime = "No Data";
let msgCount = 0;

/* ===== MQTT ===== */

client.on('connect', () => {
  console.log("✅ Connected to AWS IoT");
  client.subscribe('rtu/data');
});

client.on('message', (topic, message) => {
  try {
    const data = JSON.parse(message.toString());
    const payload = data.sensor_data ? data.sensor_data : data;
    const ts = data.timestamp || Date.now();

    if (ts <= lastTimestamp) return;

    lastTimestamp = ts;
    latestData = payload;
    lastUpdateTime = new Date().toLocaleTimeString();
    msgCount++;

  } catch (e) {
    console.log("❌ JSON error", e);
  }
});

/* ===== API ===== */

app.get('/data', (req, res) => {
  res.json({
    ...latestData,
    last_update: lastUpdateTime,
    msg_count: msgCount
  });
});

/* ===== CONTROL ===== */

app.use(express.json());

app.post('/control', (req, res) => {
  client.publish('rtu/control', JSON.stringify(req.body));
  res.send("OK");
});

/* ===== UI ===== */

app.get('/', (req, res) => {
  res.send(`
<!DOCTYPE html>
<html>
<head>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

<style>
body {
  margin:0;
  font-family:Arial;
  background:#0b1220;
  color:white;
  display:flex;
}

/* SIDEBAR */
.sidebar {
  width:160px;
  background:#111827;
  padding:10px;
}

.tab {
  padding:10px;
  margin:5px 0;
  background:#1f2937;
  border-radius:6px;
  cursor:pointer;
}

.tab.active { background:#374151; }

/* MAIN */
.main {
  flex:1;
  padding:10px;
}

/* HEADER */
.header {
  display:flex;
  justify-content:space-between;
  align-items:center;
  margin-bottom:10px;
}

/* GRID */
.grid {
  display:grid;
  grid-template-columns: repeat(3,1fr);
  gap:10px;
}

.card {
  background:#111827;
  padding:10px;
  border-radius:10px;
  text-align:center;
}

/* SWITCH */
.switch {
  position: relative;
  display: inline-block;
  width: 50px;
  height: 24px;
}

.switch input { display:none; }

.slider {
  position: absolute;
  cursor: pointer;
  background-color: #444;
  border-radius: 24px;
  top: 0; left: 0; right: 0; bottom: 0;
}

.slider:before {
  position: absolute;
  content: "";
  height: 18px;
  width: 18px;
  left: 3px;
  bottom: 3px;
  background: white;
  border-radius: 50%;
  transition: .3s;
}

input:checked + .slider {
  background-color: #2563eb;
}

input:checked + .slider:before {
  transform: translateX(26px);
}

/* LOG ONLY IN STATS */
.log {
  height:100px;
  background:black;
  overflow:auto;
  padding:5px;
  font-size:11px;
}

/* PAGE */
.page { display:none; }
.page.active { display:block; }

/* HOME */
.home {
  text-align:center;
  margin-top:50px;
}
</style>
</head>

<body>

<!-- SIDEBAR -->
<div class="sidebar">
  <div class="tab active" onclick="showTab(0)">Home</div>
  <div class="tab" onclick="showTab(1)">Dashboard</div>
  <div class="tab" onclick="showTab(2)">Debug</div>
  <div class="tab" onclick="showTab(3)">Stats</div>
</div>

<!-- MAIN -->
<div class="main">

<!-- HOME -->
<div class="page active home">

<h2>RTU Boiler Control System</h2>

<p style="opacity:0.8;">
Advanced Embedded Monitoring & Control Platform
</p>

<br>

<div style="text-align:left; max-width:500px; margin:auto; line-height:1.8;">

<b>Key Features:</b>

<ul>
  <li>⚙️ Real-time Boiler Monitoring (Temperature, Pressure, Level)</li>
  <li>📡 Secure Cloud Communication (AWS IoT MQTT)</li>
  <li>🔄 Live Process Control & Feedback System</li>
  <li>🧠 Intelligent State Handling (Start / Stop / Safety)</li>
  <li>🧪 Debug Mode for Service & Diagnostics</li>
  <li>📊 Historical Data Visualization</li>
</ul>

<b>System Capabilities:</b>

<ul>
  <li>✔ Continuous Sensor Data Acquisition</li>
  <li>✔ Remote Command Execution</li>
  <li>✔ Fault Detection & Monitoring</li>
  <li>✔ Modular Firmware Architecture</li>
</ul>

</div>

<br>

<p style="opacity:0.6;">
Use the left panel to navigate between Dashboard, Debug, and System Stats.
</p>

</div>

<!-- DASHBOARD -->
<div class="page">

<div class="header">
  <div>Time: <span id="time">--</span></div>

  <div>
    START
    <label class="switch">
      <input type="checkbox" onchange="sendCmd({start:this.checked?1:0})">
      <span class="slider"></span>
    </label>
  </div>
</div>

<div class="grid">
  <div class="card">Temp<canvas id="g1"></canvas></div>
  <div class="card">Pressure<canvas id="g2"></canvas></div>
  <div class="card">Level<canvas id="g3"></canvas></div>
</div>

<div class="grid">
  <div class="card">Pump<canvas id="g4"></canvas></div>
  <div class="card">Heater<canvas id="g5"></canvas></div>
</div>

</div>

<!-- DEBUG -->
<div class="page">
<h3>Debug</h3>

Pump
<label class="switch">
<input type="checkbox" onchange="sendCmd({pump:this.checked?1:0})">
<span class="slider"></span>
</label>

Heater
<label class="switch">
<input type="checkbox" onchange="sendCmd({heater:this.checked?1:0})">
<span class="slider"></span>
</label>

</div>

<!-- STATS -->
<div class="page">
<h3>Stats</h3>
<div id="stats"></div>
<div id="log" class="log"></div>
</div>

</div>

<script>

/* TAB */
function showTab(i){
 document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
 document.querySelectorAll('.page').forEach(p=>p.classList.remove('active'));

 document.querySelectorAll('.tab')[i].classList.add('active');
 document.querySelectorAll('.page')[i].classList.add('active');
}

/* SEND */
function sendCmd(obj){
 fetch('/control',{
  method:'POST',
  headers:{'Content-Type':'application/json'},
  body: JSON.stringify(obj)
 });
 log("CMD:"+JSON.stringify(obj));
}

/* LOG */
function log(msg){
 let d=document.getElementById('log');
 if(!d) return;
 d.innerHTML += msg+"<br>";
 d.scrollTop=d.scrollHeight;
}

/* CHART */
function create(id){
 return new Chart(document.getElementById(id),{
  type:'line',
  data:{labels:[],datasets:[{data:[]}]}
 });
}

let g1=create('g1');
let g2=create('g2');
let g3=create('g3');
let g4=create('g4');
let g5=create('g5');

function push(g,val){
 g.data.datasets[0].data.push(val);
 g.data.labels.push('');
 if(g.data.labels.length>10){
   g.data.labels.shift();
   g.data.datasets[0].data.shift();
 }
 g.update();
}

/* LOOP */
setInterval(()=>{
 fetch('/data')
 .then(r=>r.json())
 .then(d=>{

  if(!d.temperature) return;

  push(g1,d.temperature);
  push(g2,d.pressure);
  push(g3,d.levelHigh);
  push(g4,d.pump);
  push(g5,d.heater);

  document.getElementById('time').innerText=d.last_update;
  document.getElementById('stats').innerText="Messages:"+d.msg_count;

 });
},2000);

</script>

</body>
</html>
`);
});

/* START */
app.listen(PORT, () => {
  console.log("🌐 http://localhost:3000");
});