
function secondsToHHMMSS(totalSeconds) {
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;

    // Pad numbers with leading zeros
    const hh = String(hours).padStart(2, '0');
    const mm = String(minutes).padStart(2, '0');
    const ss = String(seconds).padStart(2, '0');

    return `${hh}:${mm}:${ss}`;
}

const schedules = document.getElementById("schedules");

async function fetch_schedules_post(from, to, duration) {

    const response = await fetch('/api/schedules', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            from: parseInt(from),
            to: parseInt(to),
            duration: parseInt(duration)
        })
    });
    if (!response.ok) {
        console.log(`HTTP error! status: ${response.status}`);
        return false;
    }


    fetch_schedules_get();
    return true;
}

async function fetch_schedules_delete(id) {

    const response = await fetch('/api/schedules', {
        method: 'DELETE',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ id: parseInt(id) })
    });
    if (!response.ok) {
        console.log(`HTTP error! status: ${response.status}`);
        return false;
    }
    return true;
}

function createScheduleElement(e) {
    const d1 = document.createElement("div");
    d1.className = "schedules-item";

    const s1 = document.createElement("span");
    s1.innerHTML = `${secondsToHHMMSS(e.from)} &rarr; ${secondsToHHMMSS(e.to)} = ${secondsToHHMMSS(e.duration)}`

    const b1 = document.createElement("button");
    b1.innerHTML = "remove";
    b1.addEventListener("click", () => {
        if (fetch_schedules_delete(e.id)) {
            d1.remove();
        }
    });

    d1.appendChild(s1);
    d1.appendChild(b1);
    return d1;
}

async function fetch_schedules_get() {
    try {
        const response = await fetch('/api/schedules');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        const data_schedules = data.schedules;

        schedules.innerHTML = ""; // clear elements
        for (let i = 0; i < data_schedules.length; i++) {
            schedules.appendChild(createScheduleElement(data_schedules[i]));
        }

    } catch (error) {
        console.error('Error fetching data:', error);
    }
}

const overlay = document.getElementById('schedules-overlay');

function showOverlay() {
    overlay.classList.add('active');
    fetch_schedules_get();
}

function hideOverlay() {
    overlay.classList.remove('active');
}

const schedules_edit = document.getElementById("schedules_edit");
schedules_edit.addEventListener("click", showOverlay);


function CreateNew() {
    const from = getTimePickerValue("schedules-timer-picker-1");
    const to = getTimePickerValue("schedules-timer-picker-2");
    const duration = getTimePickerValue("schedules-timer-picker-3");
    console.log(from, to, duration);
    fetch_schedules_post(from, to, duration);
}

const schedules_add = document.getElementById("schedules_add");
schedules_add.addEventListener("click", CreateNew);

overlay.addEventListener("click", (e) => {
    if (e.target !== e.currentTarget) return;
    hideOverlay();
});

