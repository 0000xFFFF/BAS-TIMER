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
            document.getElementById("toggleTimerButton").classList.toggle("on", data.auto_timer);
            document.getElementById("toggleGasButton").classList.toggle("on", data.auto_gas);
        });
}

function fetch_seconds() {

    fetch('/api/get_timer_seconds')
        .then(response => response.json())
        .then(data => {
            txt_input.value = data.seconds;
            colorButtons();
        })
        .catch(error => console.error('Error fetching timer seconds:', error));
}

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
            fetch_seconds();
        })
        .catch(error => console.error('Error:', error));
}

txt_input.addEventListener("keydown", function(event) {
    if (event.key === "Enter") {
        event.preventDefault();
        updateTime();
    }
});


function setTime(seconds) {
    txt_input.value = seconds;
    updateTime(seconds);
}


document.addEventListener("DOMContentLoaded", function() {
    fetch_seconds();
    fetch_state();
});



function colorButtons() {
    document.querySelectorAll('.btn_time').forEach(btn => {
        btn.classList.remove('on');
        if (parseInt(btn.textContent) * 60 == parseInt(txt_input.value)) {
            btn.classList.add('on');
        }
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

let ws;
let reconnectInterval = 3000; // Initial reconnect interval (1s)
const term = document.getElementById("term");

function connectWebSocket() {
    ws = new WebSocket("ws://" + document.domain + ":8001/ws");

    ws.onopen = function() {
        term.style.backgroundColor = "#000000";
    };

    ws.onmessage = function(event) {
        const json = JSON.parse(event.data);
        term.innerHTML = json.term;
        drawTemperatureGradient(json.Tmin, json.Tmax);
    };

    ws.onerror = function() {
        term.style.backgroundColor = "#600000";
        setTimeout(connectWebSocket, reconnectInterval);
    };

    ws.onclose = function() {
        term.style.backgroundColor = "#400000";
        setTimeout(connectWebSocket, reconnectInterval);
    };
}

// Start WebSocket connection
connectWebSocket();
