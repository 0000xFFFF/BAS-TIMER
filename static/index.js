document.addEventListener("DOMContentLoaded", function() {
    fetch('/get_timer_seconds')
        .then(response => response.json())
        .then(data => {
            const seconds = data.AUTO_TIMER_SECONDS;
            document.getElementById('txt_input').value = seconds;
            highlightButton(seconds);
        })
        .catch(error => console.error('Error fetching timer seconds:', error));
});

function setTime(seconds) {
    document.getElementById('txt_input').value = seconds;
    highlightButton(seconds);
    updateServer(seconds);
}

function handleTextboxChange() {
    document.querySelectorAll('.btn_time').forEach(btn => btn.classList.remove('on'));
    let seconds = parseInt(document.getElementById('txt_input').value);
    if (!isNaN(seconds)) {
        updateServer(seconds);
    }
}

function highlightButton(seconds) {
    document.querySelectorAll('.btn_time').forEach(btn => {
        btn.classList.remove('on');
        if (parseInt(btn.textContent) * 60 === seconds) {
            btn.classList.add('on');
        }
    });
}

function updateServer(seconds) {
    fetch('/set_timer_seconds', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ seconds: seconds })
    })
        .then(response => response.json())
        .then(data => console.log('Server updated:', data))
        .catch(error => console.error('Error:', error));
}


function toggleAutoTimer() {
    fetch('/toggle_autotimer', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            const button = document.getElementById('toggleTimerButton');
            button.classList.toggle('on', data.auto_timer);
        });
}

function toggleAutoGas() {
    fetch('/toggle_autogas', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            const button = document.getElementById('toggleGasButton');
            button.classList.toggle('on', data.auto_gas);
        });
}


function time() {
    let d = new Date();
    let year = d.getFullYear();
    let month = (d.getMonth() + 1).toString().padStart(2, '0'); // Months are zero-based
    let day = d.getDate().toString().padStart(2, '0');
    let h = d.getHours().toString().padStart(2, '0');
    let m = d.getMinutes().toString().padStart(2, '0');
    let s = d.getSeconds().toString().padStart(2, '0');
    return `${year}-${month}-${day} ${h}:${m}:${s}`;
}

let TEMP_MIN = 45;
let TEMP_MAX = 60;

// Helper function to interpolate colors
function getColor(temp) {

    TEMP_MIN = Math.min(TEMP_MIN, temp);
    TEMP_MAX = Math.max(TEMP_MAX, temp);

    // Prevent division by zero if all temps are the same
    if (TEMP_MIN === TEMP_MAX) {
        TEMP_MIN -= 1;
        TEMP_MAX += 1;
    }

    const normalizedTemp = (temp - TEMP_MIN) / (TEMP_MAX - TEMP_MIN);
    const r = Math.min(255, Math.max(0, normalizedTemp * 255));
    const b = Math.min(255, Math.max(0, 255 - normalizedTemp * 255));
    return `rgb(${r}, 0, ${b})`;
}

function drawTemperatureGradient(temp1, temp2, temp3) {

    const canvas = document.getElementById("canv");
    const ctx = canvas.getContext("2d");

    const gradient = ctx.createLinearGradient(0, 0, 0, canvas.height);
    gradient.addColorStop(0, getColor(temp3)); // Top (max temp)
    gradient.addColorStop(0.5, getColor(temp2)); // Middle (mid temp)
    gradient.addColorStop(1, getColor(temp1)); // Bottom (min temp)

    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, canvas.width, canvas.height);
}

function createElement(tag, props = {}) {
    const el = document.createElement(tag);
    Object.assign(el, props);
    return el;
}


const cur1 = document.getElementById("cur1");
const cur2 = document.getElementById("cur2");

function colorIt(par, label, text, clr, isbool = false) {
    const COLOR_OFF = "rgb(255, 255, 0)";
    const COLOR_ON = "rgb(0, 255, 0)";

    let style = "color: white";
    if (isbool) { style = `color: ${(!!clr ? COLOR_ON : COLOR_OFF)}`; }
    else { style = `color: ${clr}`; }

    const tr = createElement("tr");
    const td1 = createElement("td", { className: "currents_label", innerHTML: label });
    const td2 = createElement("td", { className: "currents_text", innerHTML: text, style: style });
    tr.append(td1, td2);
    par.append(tr);
}

function process(json) {


    const Tspv = json.Tspv;
    const Tsolar = json.Tsolar;
    const Tsobna = json.Tsobna;
    const Tzadata = json.Tzadata;
    const Tmax = json.Tmax;
    const Tmid = json.Tmid;
    const Tmin = json.Tmin;
    const Tfs = json.Tfs;
    const Thottest = json.Thottest;
    const Tcoldest = json.Tcoldest;

    const mod_rada = json.mod_rada;
    const mod_rezim = json.mod_rezim;
    const StatusPumpe6 = json.StatusPumpe6;
    const StatusPumpe4 = json.StatusPumpe4;
    const StatusPumpe3 = json.StatusPumpe3;
    const StatusPumpe5 = json.StatusPumpe5;
    const StatusPumpe7 = json.StatusPumpe7;
    const TminLT = json.TminLT;
    const TmidGE = json.TmidGE;

    cur1.innerHTML = "";
    colorIt(cur1, "Outside 󱇜", Tspv, getColor(Tspv));
    colorIt(cur1, "Solar 󱩳", Tsolar, getColor(Tsolar));
    colorIt(cur1, "Room ", Tsobna, getColor(Tsobna));
    colorIt(cur1, "Set ", Tzadata, getColor(Tzadata));
    colorIt(cur1, "Max ", Tmax, getColor(Tmax));
    colorIt(cur1, "Mid ", Tmid, getColor(Tmin));
    colorIt(cur1, "Min ", Tmin, getColor(Tmin));
    colorIt(cur1, "Circ. ", Tfs, getColor(Tfs));
    colorIt(cur1, "Hottest 󰈸", Thottest, getColor(Thottest));
    colorIt(cur1, "Coldest ", Tcoldest, getColor(Tcoldest));
    drawTemperatureGradient(Tmin, Tmid, Tmax);

    cur2.innerHTML = "";
    colorIt(cur2, "Mode 󱪯", mod_rada, !!mod_rada, true);
    colorIt(cur2, "Regime 󱖫", mod_rezim, !!mod_rezim, true);
    colorIt(cur2, "Heat 󱩃", StatusPumpe6, !!StatusPumpe6, true);
    colorIt(cur2, "Gas 󰙇", StatusPumpe4, !!StatusPumpe4, true);
    colorIt(cur2, "Circ. ", StatusPumpe3, !!StatusPumpe3, true);
    colorIt(cur2, "Pump5 ", StatusPumpe5, !!StatusPumpe5, true);
    colorIt(cur2, "Pump7 ", StatusPumpe7, !!StatusPumpe7, true);
    colorIt(cur2, "Min < 45", TminLT, !!TminLT, true);
    colorIt(cur2, "Mid >= 60", TmidGE, !!TmidGE, true);
}

var socket = io.connect("http://" + document.domain + ":" + location.port);

// Handle fetched data updates
socket.on("vars", function(json) {
    // document.getElementById("currents").innerText = JSON.stringify(json, null, 2);
    process(json);
});

