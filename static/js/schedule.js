
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

async function fetch_schedules_get() {

    function createScheduleElement(e) {
        const d1 = document.createElement("div");
        d1.className = "schedules_item";

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

    try {
        const response = await fetch('/api/schedules');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        const data_schedules = data.schedules;

        const schedules = document.getElementById("schedules");
        schedules.innerHTML = ""; // clear elements
        for (let i = 0; i < data_schedules.length; i++) {
            schedules.appendChild(createScheduleElement(data_schedules[i]));
        }

    } catch (error) {
        console.error('Error fetching data:', error);
    }
}

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


function showOverlay() {
    const schedules_overlay = document.getElementById('schedules_overlay');
    schedules_overlay.classList.add('active');
    fetch_schedules_get();
}

function hideOverlay() {
    const schedules_overlay = document.getElementById('schedules_overlay');
    schedules_overlay.classList.remove('active');
}

function setup_schedules() {

    const schedules_edit = document.getElementById("schedules_edit");
    schedules_edit.addEventListener("click", showOverlay);

    const schedules_add = document.getElementById("schedules_add");

    function create_new_schedule() {
        const from = getTimePickerValue("schedules_time_picker_from");
        const to = getTimePickerValue("schedules_time_picker_to");
        const duration = getTimePickerValue("schedules_time_picker_duration");
        console.log(from, to, duration);
        fetch_schedules_post(from, to, duration);
    }
    schedules_add.addEventListener("click", create_new_schedule);

    schedules_overlay.addEventListener("click", (e) => {
        if (e.target !== e.currentTarget) return;
        hideOverlay();
    });

    const picker_from = document.getElementById('schedules_time_picker_from');

    picker_from.addEventListener('timechange', e => {
        const newTime = e.detail + 15 * 60; // add 15 minutes
        setTimePickerFromSeconds('schedules_time_picker_to', newTime);
    });

    setTimePickerToNow("schedules_time_picker_from");

    setTimePickerFromSeconds('schedules_time_picker_duration', 5 * 60); // 5 min duration
}

