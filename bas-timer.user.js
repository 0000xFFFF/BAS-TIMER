// ==UserScript==
// @name         BAS TIMER
// @namespace    UserScript
// @version      1.1
// @description  BAS Timer - Turn Off Heating After XXX Seconds
// @author       MihailoVukorep
// @grant        GM_addStyle
// @grant        GM_getValue
// @grant        GM_setValue
// @match        http://192.168.1.250:9001/*
// ==/UserScript==

function setting(name, def) {
    let value = GM_getValue(name, null);
    if (value == null) { GM_setValue(name, def); value = def; }
    return value;
}

const setting_countdownsec_ = "countdownsec";
const setting_countdownsec = setting(setting_countdownsec_, "900")
const setting_pingms_ = "pingms";
const setting_pingms = setting(setting_pingms_, "5000");

const URLS = {
    OFF: "http://192.168.1.250:9001/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1",
    ON: "http://192.168.1.250:9001/isc/set_var.aspx?mod_rada=1,-1&=&SESSIONID=-1",
    VARS: "http://192.168.1.250:9001/isc/get_var_js.aspx?sw1=&sw4=&sw3=&sw2=&mod_rada=&mod_rada=&__Time=&__Date=&mod_rezim=&mod_rezim=&mod_rezim=&mod_rada=&mod_rada=&kom2=&red2=&mraz2=&glavni_on=&Tzadata=&Taktualno=&deltaT=&Tspv=&mraz=&rucni_komf=&rucni_red=&mod_rada=&mod_rada=&grejanje_off=&grejanje_off=&glavni_on=&kom1=&red1=&mod_rezim=&Alarm_tank=&Alarm_solar=&&SESSIONID=-1"
};

const STYLES = {
    container: `
        color: rgb(0, 128, 192);
        font-size: 20px;
        background: rgb(192, 192, 192);
        display: flex;
        flex-direction: column;
        gap: 1px;
        width: 400px;
        top: 500px;
        left: 40px;
        position: fixed;
        border-top: 2px double rgb(224, 224, 224);
        border-left: 2px double rgb(224, 224, 224);
        border-right: 2px double black;
        border-bottom: 2px double black;
    `,
    header: `
        background-color: silver;
        border-bottom: 2px double black;
        padding: 2px;
        display: flex;
        justify-content: space-between;
    `,
    body: `
        display: flex;
        flex-direction: column;
        gap: 16px;
        padding: 10px;
    `,
    title: `
        font-weight: bold;
        text-align: center;
        padding: 3px;
    `,
    text: `
        font-weight: bold;
    `,
    input: `
        font-size: 20px;
        color: rgb(0, 0, 127);
        font-weight: bold;
        border-top: 2px double black;
        border-left: 2px double black;
        border-right: 2px double rgb(224, 224, 224);
        border-bottom: 2px double rgb(224, 224, 224);
    `,
    button: `
        font-size: 20px;
        color: rgb(0, 0, 127);
        font-weight: bold;
        border-top: 2px double rgb(224, 224, 224);
        border-left: 2px double rgb(224, 224, 224);
        border-right: 2px double black;
        border-bottom: 2px double black;
        cursor: pointer;
    `,
    buttonContainer: `
        display: flex;
        gap: 10px;
        justify-content: space-between;
    `
};

function createElement(tag, props = {}) {
    const el = document.createElement(tag);
    Object.assign(el, props);
    return el;
}

function makeDraggable(header, container) {
    let offsetX, offsetY, isDragging = false;

    header.onmousedown = (e) => {
        e.preventDefault();
        isDragging = true;
        offsetX = e.clientX - parseInt(container.style.left);
        offsetY = e.clientY - parseInt(container.style.top);
    };

    document.onmouseup = () => isDragging = false;

    document.onmousemove = (e) => {
        if (!isDragging) return;
        container.style.left = `${e.clientX - offsetX}px`;
        container.style.top = `${e.clientY - offsetY}px`;
    };
}

const container = createElement("div", { id: "bt_cont", style: STYLES.container });
const header = createElement("div", { id: "bt_cont_head", style: STYLES.header });
const body = createElement("div", { id: "bt_cont_body", style: STYLES.body });
const title = createElement("span", { innerHTML: "Turn Off Heating Timer", style: STYLES.title });
const closeBtn = createElement("button", { innerHTML: "X", style: STYLES.button, onclick: () => container.remove() });

header.append(title, closeBtn);
container.appendChild(header);
makeDraggable(header, container);

const inputContainer = createElement("div", { style: "display: flex; align-items: center; gap: 10px;" });
const label = createElement("span", { innerHTML: "Seconds:", style: STYLES.text });
const input = createElement("input", { type: "text", id: "myInputId", style: STYLES.input, value: setting_countdownsec });
input.oninput = () => { GM_setValue(setting_countdownsec_, input.value); updateButtonStyles(); }
inputContainer.append(label, input);

var buttons = [];
function updateButtonStyles() {
    buttons.forEach(btn => {
        btn.style.backgroundColor = parseInt(input.value) === parseInt(btn.seconds) ? 'rgb(128, 255, 128)' : '';
    });
}

const buttonContainer = createElement("div", { style: STYLES.buttonContainer });
const timeOptions = [5, 10, 15, 20].map(min => {
    const btn = createElement("button", {
        id: `button_${min}min`,
        innerHTML: `${min} min`,
        style: STYLES.button,
        seconds: min * 60,
        onclick: () => {
            input.value = min * 60;
            GM_setValue(setting_countdownsec_, input.value);
            updateButtonStyles();
        }

    });
    buttons.push(btn);
    buttonContainer.appendChild(btn);
    return btn;
});

updateButtonStyles();

const status = createElement("div", { innerHTML: "...", style: STYLES.text });
let running = false, countdown;

function stopTimer() {
    running = false;
    clearInterval(countdown);
    status.innerHTML = "...";
}

function startTimer() {
    running = true;
    let secondsLeft = parseInt(input.value) || 100;
    countdown = setInterval(() => {
        status.innerHTML = `Time left: ${secondsLeft--} sec`;
        if (secondsLeft < 0) {
            stopTimer();
            status.innerHTML = "Turning off heating.";
            fetch(URLS.OFF);
        }
    }, 1000);
}

function toggleTimer() { running ? stopTimer() : startTimer(); }

const startBtn = createElement("button", { innerHTML: "Toggle", style: STYLES.button, onclick: toggleTimer });

body.append(inputContainer, buttonContainer, startBtn, status);
container.append(body);

function autostart_off() {
    clearInterval(autostart);
}

function autostart_on() {
    autostart = setInterval(async () => {
        try {
            const response = await fetch(URLS.VARS); // Fetch the data
            if (!response.ok) { throw new Error(`HTTP error! Status: ${response.status}`); }
            const json = await response.json(); // Parse JSON

            console.log(json.Tzadata.value);

            if (json.Tzadata.value == 29) { if (!running) { startTimer(); } }
            else { if (running) { stopTimer(); } }

        }
        catch (error) { console.error("Error fetching data:", error); }

    }, setting_pingms);
}

autostart_on();


function appendTimer() {
    if (!document.body) return setTimeout(appendTimer, 100);
    document.body.appendChild(container);
}
appendTimer();

