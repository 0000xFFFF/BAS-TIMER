function toggleAutoTimer() {
    fetch('/toggle_autotimer', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            document.getElementById('auto_timer').innerText = data.auto_timer ? "ON" : "OFF";
            document.getElementById('toggleTimerButton').innerText = data.auto_timer ? "Turn OFF" : "Turn ON";
        });
}

function toggleAutoGas() {
    fetch('/toggle_autogas', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            document.getElementById('auto_gas').innerText = data.auto_gas ? "ON" : "OFF";
            document.getElementById('toggleGasButton').innerText = data.auto_gas ? "Turn OFF" : "Turn ON";
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

    const canvas = document.getElementById("currents_canvas");
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


const currents = document.getElementById("currents_tbody");

function colorIt(label, text, clr) {
    const COLOR_OFF = "rgb(255, 255, 0)";
    const COLOR_ON = "rgb(0, 255, 0)";

    let style = "color: white";
    if (typeof clr === 'boolean') { style = `color: ${(clr ? COLOR_ON : COLOR_OFF)}`; }
    else if (typeof clr === 'string') { style = `color: ${clr}`; }

    const tr = createElement("tr");
    const td1 = createElement("td", { className: "currents_label", innerHTML: label });
    const td2 = createElement("td", { className: "currents_text", innerHTML: text, style: style });
    tr.append(td1, td2);
    currents.append(tr);
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

    currents.innerHTML = "";

    colorIt("Outside 󱇜", Tspv, getColor(Tspv))
    colorIt("Solar 󱩳", Tsolar, getColor(Tsolar))
    colorIt("Room ", Tsobna, getColor(Tsobna))
    colorIt("Set ", Tzadata, getColor(Tzadata))
    colorIt("Max ", Tmax, getColor(Tmax))
    colorIt("Mid 󰝹", Tmid, getColor(Tmin))
    colorIt("Min ", Tmin, getColor(Tmin))
    colorIt("Circ. ", Tfs, getColor(Tfs))
    colorIt("Hottest 󰈸", Thottest, getColor(Thottest))
    colorIt("Coldest ", Tcoldest, getColor(Tcoldest))

    drawTemperatureGradient(Tmin, Tmid, Tmax);
}

var socket = io.connect("http://" + document.domain + ":" + location.port);

// Handle fetched data updates
socket.on("vars", function(json) {
    // document.getElementById("currents").innerText = JSON.stringify(json, null, 2);
    process(json);
});

