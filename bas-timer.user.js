// ==UserScript==
// @name         BAS TIMER
// @namespace    UserScript
// @version      1
// @description  BAS Timer - Turn Off Heating After XXX Seconds
// @author       MihailoVukorep
// @match        http://192.168.1.250:9001/*
// ==/UserScript==



const url_off = "http://192.168.1.250:9001/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1"
const url_on  = "http://192.168.1.250:9001/isc/set_var.aspx?mod_rada=1,-1&=&SESSIONID=-1"
const url_vars = "http://192.168.1.250:9001/isc/get_var_js.aspx?sw1=&sw4=&sw3=&sw2=&mod_rada=&mod_rada=&__Time=&__Date=&mod_rezim=&mod_rezim=&mod_rezim=&mod_rada=&mod_rada=&kom2=&red2=&mraz2=&glavni_on=&Tzadata=&Taktualno=&deltaT=&Tspv=&mraz=&rucni_komf=&rucni_red=&mod_rada=&mod_rada=&grejanje_off=&grejanje_off=&glavni_on=&kom1=&red1=&mod_rezim=&Alarm_tank=&Alarm_solar=&&SESSIONID=-1"

function dragSetup(head, cont) {
    let initialX;
    let initialY;
    let isDragging = false;

    head.onmousedown = function(event) {
        event.preventDefault();  // Prevent default action (like select text in input fields)
        isDragging = true;
        initialX = event.clientX - parseInt(cont.style.left);
        initialY = event.clientY - parseInt(cont.style.top);
        head.focus();  // Ensure the head has focus
    }

    document.onmouseup = function() {
        if (isDragging) {
            cont.style.position = "fixed";  // Set the position to fixed when drag ends
            isDragging = false;

            let x = event.clientX - initialX,
                y = event.clientY - initialY;

            cont.style.left = `${x}px`;  // Update the left position of the element
            cont.style.top = `${y}px`;  // Update the top position of the element
        }
    }

    head.onmousemove = function(event) {
        event.preventDefault();  // Prevent default action (like selecting text in input fields)

        if (!isDragging) return;

        let x = event.clientX - initialX,
            y = event.clientY - initialY;

        cont.style.left = `${x}px`;  // Update the left position of the element
        cont.style.top = `${y}px`;  // Update the top position of the element
    }
}

const cont_css = `
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
`;

const cont_head_css = `
    background-color; gray;
    border-bottom: 1px solid black;
    padding: 2px;
    display: flex;
    justify-content: right;
`
const title_css = `
    width: 370px;
    font-weight: bold;
    text-align: center;
    padding: 3px;
`
const text_css = `
    font-weight: bold;
`

const cont_body_css = `
    display: flex;
    flex-direction: column;
    gap: 16px;
    padding: 10px;
`

const txt_input_css = `
    font-size: 20px;
    color: rgb(0, 0, 127);
    font-weight: bold;
    border-top: 2px double black;
    border-left: 2px double black;
    border-right: 2px double rgb(224, 224, 224);
    border-bottom: 2px double rgb(224, 224, 224);
`

const btn_css = `
    font-size: 20px;
    color: rgb(0, 0, 127);
    font-weight: bold;
    border-top: 2px double rgb(224, 224, 224);
    border-left: 2px double rgb(224, 224, 224);
    border-right: 2px double black;
    border-bottom: 2px double black;
    cursor: pointer;
`

const btn_container_css = `
    display: flex;
    gap: 10px;
    justify-content: space-between;
`

function appendTimer() {
    if (document.body) {
        const cont = document.createElement("div");
        cont.id = "bt_cont";
        cont.style = cont_css;

        const cont_head = document.createElement("div");
        cont_head.id = "bt_cont_head";
        cont_head.style = cont_head_css;

        const cont_body = document.createElement("div");
        cont_body.id = "bt_cont_body";
        cont_body.style = cont_body_css;

        cont.appendChild(cont_head);
        cont.appendChild(cont_body);

        dragSetup(cont_head, cont);

        const title = document.createElement("span");
        title.style = title_css;
        title.innerHTML = "Turn Off Heating Timer";

        const btn_close = document.createElement("button");
        btn_close.style = btn_css;
        btn_close.innerHTML = "X";


        cont_head.appendChild(title);
        cont_head.appendChild(btn_close);

        // Create flex container for label and input
        const inputContainer = document.createElement("div");
        inputContainer.style = "display: flex; align-items: center; gap: 10px;";

        const label = document.createElement("span");
        label.style = text_css;
        label.innerHTML = "Seconds:";

        const txt_input = document.createElement('input');
        txt_input.setAttribute('type', 'text');
        txt_input.setAttribute('id', 'myInputId');
        txt_input.setAttribute('placeholder', 'wait for X seconds.');
        txt_input.style = txt_input_css;


        // Load saved value
        const savedValue = localStorage.getItem('basTimerValue');
        txt_input.value = savedValue ? savedValue : 30 * 60;

        // Save value when changed
        txt_input.addEventListener('input', function() {
            localStorage.setItem('basTimerValue', txt_input.value);
            updateButtonStyles();
        });


        inputContainer.appendChild(label);
        inputContainer.appendChild(txt_input);

        let running = false;
        const label_begin = "Begin Countdown";
        const label_stop  = "Stop Countdown";
        const label_dots = "...";

        const txt_status = document.createElement("div");
        txt_status.style = text_css;
        txt_status.innerHTML = label_dots;

        const btn_begin = document.createElement("button");
        btn_begin.style = btn_css;
        btn_begin.innerHTML = label_begin;


        function timer_stop() {
            running = false;
            clearInterval(countdown);

            btn_begin.innerHTML = label_begin;
            txt_status.innerHTML = label_dots;
        }

        function timer_start() {
            running = true;
            btn_begin.innerHTML = label_stop;
            let totalSeconds = parseInt(txt_input.value) || 100; // Convert the value to an integer
            countdown = setInterval(() => {
                txt_status.innerHTML = `Time left: ${totalSeconds} seconds`;
                if(--totalSeconds < 0) {
                    timer_stop();
                    txt_status.innerHTML = "Countdown ended! Turning off heating.";

                    // TURN HEATING OFF
                    fetch(url_off);
                }
            }, 1000); // Run the block every second (1000ms)
        }

        function timer_switch () {
            if (running) { timer_stop(); } else { timer_start(); }
        }

        btn_begin.onclick = timer_switch;

        // Create button container
        const btn_container = document.createElement("div");
        btn_container.style = btn_container_css;

        function createTimeButton(label, seconds) {
            const button = document.createElement("button");
            button.style = btn_css;
            button.innerHTML = label;
            button.onclick = function () {
                txt_input.value = seconds;
                localStorage.setItem('basTimerValue', txt_input.value);
                updateButtonStyles();
            }
            button.dataset.seconds = seconds;
            return button;
        }

        const btn_15min = createTimeButton("15 min", 15 * 60);
        const btn_30min = createTimeButton("30 min", 30 * 60);
        const btn_45min = createTimeButton("45 min", 45 * 60);
        const btn_1hour = createTimeButton("1 hour", 60 * 60);

        btn_container.appendChild(btn_15min);
        btn_container.appendChild(btn_30min);
        btn_container.appendChild(btn_45min);
        btn_container.appendChild(btn_1hour);

        function updateButtonStyles() {
            const buttons = [btn_15min, btn_30min, btn_45min, btn_1hour];
            buttons.forEach(btn => {
                btn.style.backgroundColor = parseInt(txt_input.value) === parseInt(btn.dataset.seconds) ? 'rgb(128, 255, 128)' : '';
            });
        }

        updateButtonStyles();

        cont_body.appendChild(inputContainer);
        cont_body.appendChild(btn_container);
        //cont_body.appendChild(btn_begin);
        cont_body.appendChild(txt_status);

        document.body.appendChild(cont);

        function autostart_off() {
            clearInterval(autostart);
        }

        function autostart_on() {
            autostart = setInterval(async () => {
                try {
                    const response = await fetch(url_vars); // Fetch the data
                    if (!response.ok) {
                        throw new Error(`HTTP error! Status: ${response.status}`);
                    }
                    const json = await response.json(); // Parse JSON

                    console.log(json.Tzadata.value);

                    if (json.Tzadata.value == 29) {
                        if (!running) {
                            timer_start();
                        }
                    }
                    else {
                        if (running) {
                            timer_stop();
                        }
                    }

                } catch (error) {
                    console.error("Error fetching data:", error);
                }

            }, 10000);
        }

        autostart_on();

        btn_close.onclick = function () {
            cont.remove();
            autostart_off();
        }

    } else {
        setTimeout(appendTimer, 100);
    }
}
appendTimer();
