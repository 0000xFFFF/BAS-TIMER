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

function process(json) {
    const Tmax = json.Tmax;
    const Tmid = json.Tmid;
    const Tmin = json.Tmin;
    drawTemperatureGradient(Tmin, Tmid, Tmax);
}

var socket = io.connect("http://" + document.domain + ":" + location.port);

// Handle fetched data updates
socket.on("vars", function(json) {
    // document.getElementById("currents").innerText = JSON.stringify(json, null, 2);
    process(json);
});


const term = document.getElementById("term");
socket.on("term", function(text) {
    term.innerHTML = text;
});
