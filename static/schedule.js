
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

async function fetch_schedules() {
    try {
        const response = await fetch('/api/schedules');
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        const data_schedules = data.schedules;

        schedules.innerHTML = ""; // clear elements
        for (let i = 0; i < data_schedules.length; i++) {
            const e = data_schedules[i];
            console.log(e);
            const data_div = document.createElement("div");
            data_div.innerHTML = `${secondsToHHMMSS(e.from)} &rarr; ${secondsToHHMMSS(e.to)}`
            schedules.appendChild(data_div);
        }

    } catch (error) {
        console.error('Error fetching data:', error);
    }
}

const overlay = document.querySelector('.tp-overlay');

function showOverlay() {
    overlay.classList.add('active');
    fetch_schedules();
}

function hideOverlay() {
    overlay.classList.remove('active');
}

const edit_schedules = document.getElementById("edit_schedules");
edit_schedules.addEventListener("click", showOverlay);

overlay.addEventListener("click", (e) => {
    if (e.target !== e.currentTarget) return;
    hideOverlay();
});

