const overlay = document.querySelector('.tp-overlay');

function showOverlay() {
    overlay.classList.add('active');
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
