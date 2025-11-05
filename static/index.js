const txt_input = document.getElementById('txt_input');


function bas_heat_on() {
    fetch('/api/bas_heat_on')
        .then(response => response.json())
        .then(data => console.log(data))
        .catch(error => console.error('Error fetching data:', error));
}

function bas_heat_off() {
    fetch('/api/bas_heat_off')
        .then(response => response.json())
        .then(data => console.log(data))
        .catch(error => console.error('Error fetching data:', error));
}

function bas_gas_on() {
    fetch('/api/bas_gas_on')
        .then(response => response.json())
        .then(data => console.log(data))
        .catch(error => console.error('Error fetching data:', error));
}

function bas_gas_off() {
    fetch('/api/bas_gas_off')
        .then(response => response.json())
        .then(data => console.log(data))
        .catch(error => console.error('Error fetching data:', error));
}

function fetch_state() {
    fetch("/api/state")
        .then(response => response.json())
        .then(data => {

            txt_input.value = data.seconds;
            colorButtons();

            document.getElementById("toggleTimerButton").classList.toggle("on", data.auto_timer);
            document.getElementById("toggleGasButton").classList.toggle("on", data.auto_gas);
        });
}

function fetch_sumtime() {
    fetch("/api/sumtime")
        .then(response => response.json())
        .then(data => {
            document.getElementById("sumtime_mod_rada").innerHTML = data.mod_rada;
            document.getElementById("sumtime_StatusPumpe4").innerHTML = data.StatusPumpe4;
        });
}

fetch_sumtime();

function fetch_all() {
    fetch_state();
    fetch_sumtime();
}

setInterval(fetch_all, 3600000); // 1000 * 60 * 60 -- every hour fetch all

function updateTime() {
    const seconds = txt_input.value;

    fetch('/api/set_timer_seconds', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ seconds: parseInt(seconds) })
    })
        .then(response => response.json())
        .then(data => {
            console.log('Server updated:', data);
            fetch_state();
        })
        .catch(error => console.error('Error:', error));
}

txt_input.addEventListener("keydown", function(event) {
    if (event.key === "Enter") {
        event.preventDefault();
        updateTime();
    }
});


function setTime(btn) {
    const seconds = parseInt(btn.textContent) * 60;
    txt_input.value = seconds;
    updateTime(seconds);
}


document.addEventListener("DOMContentLoaded", function() {
    fetch_state();
});


function colorButton(btn, isOn) {
    btn.classList.remove('on');
    if (isOn) {
        btn.classList.add('on');
    }
}

function colorButtons() {
    document.querySelectorAll('.btn_time').forEach(btn => {
        colorButton(btn, parseInt(btn.textContent) * 60 == parseInt(txt_input.value));
    });
}

function toggleAutoTimer() {
    fetch('/api/toggle_auto_timer', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            const button = document.getElementById('toggleTimerButton');
            button.classList.toggle('on', data.auto_timer);
        });
}

function toggleAutoGas() {
    fetch('/api/toggle_auto_gas', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            const button = document.getElementById('toggleGasButton');
            button.classList.toggle('on', data.auto_gas);
        });
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

function drawTemperatureGradient(temp_min, temp_max) {

    const canvas = document.getElementById("canv");
    const ctx = canvas.getContext("2d");

    const gradient = ctx.createLinearGradient(0, 0, 0, canvas.height);
    gradient.addColorStop(0, getColor(temp_max)); // Top (max temp)
    gradient.addColorStop(1, getColor(temp_min)); // Bottom (min temp)

    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, canvas.width, canvas.height);
}

let ws = null;
const RECONN_INTERVAL_MS = 1000;
const termcont = document.getElementById("termcont");
const term = document.getElementById("term");
let reconnectInterval = null;

let term_w = term.width;
let term_h = term.height;

function perror(error) {
    ws = null;
    termcont.style.backgroundColor = "#600000";
    if (reconnectInterval == null) {
        reconnectInterval = setInterval(connect, RECONN_INTERVAL_MS);
    }
}

function updateTermContent(newContent) {
    // Update the content of the div
    term.innerHTML = newContent;

    // Get the computed width and height of the div
    const computedStyle = window.getComputedStyle(term);

    // Set the minimum size constraints based on the current size of the content
    const minWidth = parseInt(computedStyle.minWidth, 10);
    const minHeight = parseInt(computedStyle.minHeight, 10);

    // Adjust the width and height of the div if necessary
    term.style.width = Math.max(term.scrollWidth, minWidth) + 'px';
    term.style.height = Math.max(term.scrollHeight, minHeight) + 'px';
}

function connect() {
    try {
        ws = new WebSocket("ws://" + document.domain + ":8001/ws");

        ws.onopen = function() {
            termcont.style.backgroundColor = "#000000";
            clearInterval(reconnectInterval);
            reconnectInterval = null;

            fetch_all();
        };

        ws.onmessage = function(event) {
            const json = JSON.parse(event.data);
            updateTermContent(json.term);
            drawTemperatureGradient(json.Tmin, json.Tmax);
            colorButton(document.getElementById("bas_heat_off"), json.mod_rada == 0);
            colorButton(document.getElementById("bas_heat_on"), json.mod_rada == 1);
            colorButton(document.getElementById("bas_gas_off"), json.StatusPumpe4 == 0 || json.StatusPumpe4 == 2);
            colorButton(document.getElementById("bas_gas_on"), json.StatusPumpe4 == 1 || json.StatusPumpe4 == 3);
        };

        ws.onerror = function(error) { perror(error); };
        ws.onclose = function(error) { perror(error); };

    }
    catch (error) { perror(error); }
}

connect();
