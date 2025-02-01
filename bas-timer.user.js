// ==UserScript==
// @name         BAS TIMER
// @namespace    UserScript
// @version      1
// @description  BAS Timer - Turn Off Heating After XXX Seconds
// @author       MihailoVukorep
// @match        http://192.168.1.250:9001/*
// ==/UserScript==


// GET http://192.168.1.250:9001/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1&_=1738365399411    == OFF
// GET http://192.168.1.250:9001/isc/set_var.aspx?mod_rada=1,-1&=&SESSIONID=-1&_=1738365399419    == ON

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
    height: 200px;
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
        btn_close.onclick = function () { cont.remove(); }

        cont_head.appendChild(title);
        cont_head.appendChild(btn_close);

        const txt_input = document.createElement('input');
        txt_input.setAttribute('type', 'text');
        txt_input.setAttribute('id', 'myInputId');
        txt_input.setAttribute('placeholder', 'wait for X seconds.');
        txt_input.style = txt_input_css;
        txt_input.value = 15 * 60;

        let isGoing = true;

        const label_begin = "Begin Countdown";
        const label_stop  = "Stop Countdown";

        const txt_begin = document.createElement("button");
        txt_begin.style = btn_css;
        txt_begin.innerHTML = label_begin;
        txt_begin.onclick = function () {
            if (isGoing) {
                isGoing = !isGoing;
                txt_begin.innerHTML = label_stop;
                let totalSeconds = parseInt(txt_input.value); // Convert the value to an integer
                countdown = setInterval(() => {
                    txt_status.innerHTML = `Time left: ${totalSeconds} seconds`;
                    if(--totalSeconds < 0) {
                        clearInterval(countdown);
                        txt_status.innerHTML = "Countdown ended! Turning off heating.";
                        isGoing = !isGoing;

                        // TURN HEATING OFF

                        // TODO: GET http://192.168.1.250:9001/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1
                        const url = "http://192.168.1.250:9001/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1";
                        fetch(url);

                        txt_begin.innerHTML = label_begin;
                    }
                }, 1000); // Run the block every second (1000ms)
            } else {
                clearInterval(countdown);
                isGoing = !isGoing;
                txt_begin.innerHTML = label_begin;
            }
        }

        const txt_status = document.createElement("div");
        txt_status.style = text_css;
        txt_status.innerHTML = "...";

        cont_body.appendChild(txt_input);
        cont_body.appendChild(txt_begin);
        cont_body.appendChild(txt_status);


        document.body.appendChild(cont);

    } else {
        setTimeout(appendTimer, 100);
    }
}
appendTimer();

