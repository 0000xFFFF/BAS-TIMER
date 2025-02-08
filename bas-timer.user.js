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

const setting_countdownsec_ = "countdownsec"; var setting_countdownsec = setting(setting_countdownsec_, "900");
const setting_pinger_       = "pinger";       var setting_pinger       = setting(setting_pinger_,       true);
const setting_pingms_       = "pingms";       var setting_pingms       = setting(setting_pingms_,       "5000");
const setting_autotimer_    = "autotimer";    var setting_autotimer    = setting(setting_autotimer_,    true);
const setting_autogas_      = "autogas";      var setting_autogas      = setting(setting_autogas_,      true);
const setting_lowbound4gas_ = "lowbound4gas"; var setting_lowbound4gas = setting(setting_lowbound4gas_, 57);
const setting_higbound4gas_ = "higbound4gas"; var setting_higbound4gas = setting(setting_higbound4gas_, 60);

const setting_lastseen_mod_rada_        = "lastseen_mod_rada";        var setting_lastseen_mod_rada        = setting(setting_lastseen_mod_rada_,        null);
const setting_lastseen_Tzadata_         = "lastseen_Tzadata";         var setting_lastseen_Tzadata         = setting(setting_lastseen_Tzadata_,         null);
const setting_lastseen_RezimRadaPumpe4_ = "lastseen_RezimRadaPumpe4"; var setting_lastseen_RezimRadaPumpe4 = setting(setting_lastseen_RezimRadaPumpe4_, null);
const setting_lastseen_Tmin_            = "lastseen_Tmin";            var setting_lastseen_Tmin            = setting(setting_lastseen_Tmin_,            null);
const setting_lastseen_Tmax_            = "lastseen_Tmax";            var setting_lastseen_Tmax            = setting(setting_lastseen_Tmax_,            null);
const setting_lastseen_Tmid_            = "lastseen_Tmid";            var setting_lastseen_Tmid            = setting(setting_lastseen_Tmid_,            null);
const setting_lastseen_aboveBound_      = "lastseen_aboveBound";      var setting_lastseen_aboveBound      = setting(setting_lastseen_aboveBound_,      null);

const URLS = {
    OFF:     "http://192.168.1.250:9001/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1",
    ON:      "http://192.168.1.250:9001/isc/set_var.aspx?mod_rada=1,-1&=&SESSIONID=-1",
    VARS:    "http://192.168.1.250:9001/isc/get_var_js.aspx?mod_rada&Tzadata&RezimRadaPumpe4&Tmin&Tmax&SESSIONID=-1",
    GAS_OFF: "http://192.168.1.250:9001/isc/set_var.aspx?RezimRadaPumpe4=0,-1&=&SESSIONID=-1",
    GAS_ON:  "http://192.168.1.250:9001/isc/set_var.aspx?RezimRadaPumpe4=3,-1&=&SESSIONID=-1"
};


GM_addStyle(`

#mainCont {
    display: flex;
    flex-direction: row;
}

#bt_cont {
    margin: 20px;
    display: flex;
    color: blue;
    font-size: 20px;
    background: rgb(192, 192, 192);
    flex-direction: column;
    gap: 1px;
    width: 600px;
    border-top: 2px double rgb(224, 224, 224);
    border-left: 2px double rgb(224, 224, 224);
    border-right: 2px double black;
    border-bottom: 2px double black;
}

#bt_cont_head {
    background-color: silver;
    border-bottom: 2px double black;
    padding: 2px;
    display: flex;
    justify-content: space-between;
}

#bt_cont_head_buttons {
    display: flex;
    justify-content: space-between;
    gap: 3px;
}

#bt_cont_body {
    display: flex;
    flex-direction: column;
    gap: 16px;
    padding: 10px;
}

#bt_title {
    font-weight: bold;
    text-align: center;
    padding: 3px;
}

.bt_text {
    font-weight: bold;
}

.bt_input {
    font-size: 20px;
    color: rgb(0, 0, 127);
    font-weight: bold;
    border-top: 2px double black;
    border-left: 2px double black;
    border-right: 2px double rgb(224, 224, 224);
    border-bottom: 2px double rgb(224, 224, 224);
}

.bt_button {
    font-size: 20px;
    color: rgb(0, 0, 127);
    font-weight: bold;
    border-top: 2px double rgb(224, 224, 224);
    border-left: 2px double rgb(224, 224, 224);
    border-right: 2px double black;
    border-bottom: 2px double black;
    cursor: pointer;
}

#btn_cont {
    display: flex;
    gap: 10px;
    justify-content: space-between;
}


#bt_logs {
    display: block;
}

.cb_cont {
    display: flex;
    justify-content: left;
    gap: 2px;
}

#logs {
    color: blue;
    padding: 5px;
    font-size: 20px;
    width: calc(90vw - 400px);
    height: 80vh;
    border-top: 2px double rgb(224, 224, 224);
    border-left: 2px double rgb(224, 224, 224);
    border-right: 2px double black;
    border-bottom: 2px double black;
    overflow-y: auto;
    font-family: monospace;
}

#currents {
    color: blue;
    padding: 5px;
    font-size: 20px;
    width: 530px;
    height: 300px;
    border-top: 2px double rgb(224, 224, 224);
    border-left: 2px double rgb(224, 224, 224);
    border-right: 2px double black;
    border-bottom: 2px double black;
    font-family: monospace;
}

`);

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

function createElement(tag, props = {}) {
    const el = document.createElement(tag);
    Object.assign(el, props);
    return el;
}

function createCheckbox(name, setting, def, text) {
    const cont = createElement("div", { id: `cb_${name}_cont`, className: "cb_cont" });
    const checkbox = createElement("input", {
        id: `cb_${name}`,
        type: "checkbox",
        checked: GM_getValue(setting, def)
    });

    checkbox.addEventListener('change', function (event) {
        GM_setValue(setting, event.target.checked);
        //location.reload();
    });

    const label = createElement("label", { id: `cb_${name}_label`, innerHTML: text });
    cont.append(checkbox, label);
    return cont;
}

const maincont = createElement("div", { id: "mainCont" });
const logscont = createElement("div", { id: "logs" });

function renderlog(time, text) {
    const span = createElement("div", { innerHTML: `${time} | ${text}`, className: "log" })
    logscont.appendChild(span);

    // scroll down
    logscont.scrollTop = logscont.scrollHeight;
}

function loadlogs() {
    var logs = GM_getValue("logs", []);
    logs.forEach(logEntry => {
        const [time, text] = logEntry; // Destructure the log entry
        renderlog(time, text); // Render each log
    });
}
loadlogs();

function log(text) {
    var logs = GM_getValue("logs", []);
    const e = [time(), text];
    logs.push(e);
    console.log(e);
    renderlog(e[0], e[1]);
    GM_setValue("logs", logs);
}

const container = createElement("div", { id: "bt_cont" });
const header = createElement("div", { id: "bt_cont_head" });
const body = createElement("div", { id: "bt_cont_body" });
const title = createElement("span", { id: "bt_title", innerHTML: "Turn Off Heating Timer" });
header.append(title);
container.appendChild(header);

const inputContainer = createElement("div", { style: "display: flex; align-items: center; gap: 10px;" });
const label = createElement("span", { innerHTML: "Seconds:", className: "bt_text" });
const input = createElement("input", { type: "text", id: "txt_input", className: "bt_input", value: setting_countdownsec });
input.oninput = () => { GM_setValue(setting_countdownsec_, input.value); updateButtonStyles(); }
inputContainer.append(label, input);

var buttons = [];
function updateButtonStyles() {
    buttons.forEach(btn => {
        btn.style.backgroundColor = parseInt(input.value) === parseInt(btn.seconds) ? 'rgb(128, 255, 128)' : '';
    });
}

const buttonContainer = createElement("div", { id: "btn_cont" });
const timeOptions = [5, 10, 15, 20, 25].map(min => {
    const btn = createElement("button", {
        id: `button_${min}min`,
        innerHTML: `${min} min`,
        className: "bt_button",
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

const status = createElement("div", { innerHTML: "...", className: "bt_text" });
const currents = createElement("div", { id: "currents" });

let running = false, countdown;

function stopTimer() {
    running = false;
    clearInterval(countdown);
    status.innerHTML = "...";
    log("timer stopped");
}

function startTimer() {
    log("timer started");
    running = true;
    let secondsLeft = parseInt(input.value) || 100;
    countdown = setInterval(() => {
        status.innerHTML = `Time left: ${secondsLeft--} sec`;
        if (secondsLeft < 0) {
            stopTimer();
            status.innerHTML = "Turning off heating.";
            log("HEAT OFF (set mod_rada=0)");
            fetch(URLS.OFF);
        }
    }, 1000);
}

function toggleTimer() { running ? stopTimer() : startTimer(); }

const cbAutoTimer = createCheckbox("autotimer", setting_autotimer_, setting_autotimer, `Auto Timer (mod_rada = 1 for X s => turn off)`);
const cbAutoGas   = createCheckbox("autogas",   setting_autogas_,   setting_autogas,   `Auto Gas (${setting_lowbound4gas}*c < Tmid < ${setting_higbound4gas}*c)`);

if (setting_pinger) {
    pinger = setInterval(async () => {

        if (!setting_pinger) { return; }

        try {
            const response = await fetch(URLS.VARS); // Fetch the data
            if (!response.ok) { throw new Error(`HTTP error! Status: ${response.status}`); }
            const json = await response.json(); // Parse JSON

            const mod_rada = json.mod_rada.value;
            const Tzadata = json.Tzadata.value;
            const RezimRadaPumpe4 = json.RezimRadaPumpe4.value;
            const Tmin = json.Tmin.value;
            const Tmax = json.Tmax.value;
            const Tmid = (json.Tmin.value + json.Tmax.value) / 2;
            const aboveBound = Tmid > setting_lowbound4gas;

            currents.innerHTML = `mod_rada.......: ${mod_rada}<br>`
                               + `Tzadata........: ${Tzadata}<br>`
                               + `RezimRadaPumpe4: ${RezimRadaPumpe4}<br>`
                               + `Tmin...........: ${Tmin}<br>`
                               + `Tmax...........: ${Tmax}<br>`
                               + `Tmid...........: ${Tmid}<br>`
                               + `Tmid>${setting_lowbound4gas}........: ${aboveBound}<br>`;

            console.log(json);

            if (setting_lastseen_mod_rada != mod_rada) {
                setting_lastseen_mod_rada = mod_rada;
                GM_setValue(setting_lastseen_mod_rada_, setting_lastseen_mod_rada)
                log(`mod_rada = ${setting_lastseen_mod_rada}`)
            }

            if (setting_lastseen_Tzadata != Tzadata) {
                setting_lastseen_Tzadata = Tzadata;
                GM_setValue(setting_lastseen_Tzadata_, setting_lastseen_Tzadata)
                log(`Tzadata = ${setting_lastseen_Tzadata}`)
            }

            if (setting_lastseen_RezimRadaPumpe4 != RezimRadaPumpe4) {
                setting_lastseen_RezimRadaPumpe4 = RezimRadaPumpe4;
                GM_setValue(setting_lastseen_RezimRadaPumpe4_, setting_lastseen_RezimRadaPumpe4)
                log(`RezimRadaPumpe4 = ${setting_lastseen_RezimRadaPumpe4}`)
            }

            if (setting_lastseen_Tmin != Tmin) {
                setting_lastseen_Tmin = Tmin;
                GM_setValue(setting_lastseen_Tmin_, setting_lastseen_Tmin)
                //log(`Tmin: ${setting_lastseen_Tmin}`)
            }

            if (setting_lastseen_Tmax != Tmax) {
                setting_lastseen_Tmax = Tmax;
                GM_setValue(setting_lastseen_Tmax_, setting_lastseen_Tmax)
                //log(`Tmax: ${setting_lastseen_Tmax}`)
            }

            if (setting_lastseen_Tmid != Tmid) {
                setting_lastseen_Tmid = Tmid;
                GM_setValue(setting_lastseen_Tmid_, setting_lastseen_Tmid)
                //log(`Tmid: ${setting_lastseen_Tmid}`)
            }

            if (setting_lastseen_aboveBound != aboveBound) {
                setting_lastseen_aboveBound = aboveBound;
                GM_setValue(setting_lastseen_aboveBound_, setting_lastseen_aboveBound)
                log(`Tmid: ${Tmid} > ${setting_lowbound4gas} = ${setting_lastseen_aboveBound}`)

                if (setting_autogas && RezimRadaPumpe4 == 0 && !setting_lastseen_aboveBound) {
                    log("GAS ON (set RezimRadaPumpe4=3)");
                    fetch(URLS.GAS_ON);
                }
            }

            if (setting_autogas) {
                const aboveHigh = Tmid >= setting_higbound4gas;
                if (RezimRadaPumpe4 == 3 && aboveHigh) {
                    log(`Tmid: ${Tmid} >= ${setting_higbound4gas} = ${aboveHigh}`)
                    log("GAS OFF (set RezimRadaPumpe4=0)");
                    fetch(URLS.GAS_OFF);
                }
            }


            if (setting_autotimer) {
                if (mod_rada == 1) { if (!running) { startTimer(); } }
                else { if (running) { stopTimer(); } }
            }

        }
        catch (error) { console.error("Error fetching data:", error); }

    }, setting_pingms);
}


const cbPinger = createCheckbox("pinger", setting_pinger_, setting_pinger, `Ping every ${setting_pingms} ms`);
const startBtn = createElement("button", { innerHTML: "Toggle", className: "bt_button", onclick: toggleTimer });


body.append(inputContainer, buttonContainer, startBtn, cbPinger, cbAutoTimer, cbAutoGas, status, currents);
container.append(body);
maincont.append(container);
maincont.append(logscont);

function appendTimer() {
    if (!document.body) return setTimeout(appendTimer, 100);
    document.body.appendChild(maincont);
}
appendTimer();

