let heatTimes = [];

const RADIUS_CLOCK = 400;
const RADIUS_OUTER = 350;
const RADIUS_INNER = 320;

const COLOR_PM_BEFORE = "#D1BE45";
const COLOR_PM_AFTER = "#FF7850";

const COLOR_NOW = "#FF0000";

function preload() {
    fetch('/api/times')
        .then(res => res.json())
        .then(data => {
            heatTimes = data;
        })
        .catch(err => {
            console.error('Failed to load /times', err);
        });
}

function setup() {
    let cnv = createCanvas(400, 400);
    cnv.id('aclock');
    cnv.style('display', 'block');
    cnv.parent('clock_box');

    angleMode(DEGREES);
}


function drawHands() {
    let hr = hour();
    let mn = minute();
    let sc = second();

    let secondAngle = map(sc, 0, 60, 0, 360);
    let minuteAngle = map(mn + sc / 60, 0, 60, 0, 360);
    let hourAngle = map((hr % 12) + mn / 60, 0, 12, 0, 360);

    // second hand
    push();
    rotate(secondAngle);
    stroke(190);
    strokeWeight(2);
    line(0, 0, 140, 0);
    pop();

    // minute hand
    push();
    rotate(minuteAngle);
    stroke(150);
    strokeWeight(4);
    line(0, 0, 115, 0);
    pop();

    // hour hand
    push();
    rotate(hourAngle);
    stroke(110);
    strokeWeight(6);
    line(0, 0, 85, 0);
    pop();
}

let ongoingHeatStart = null; // tracks frontend checkbox heat start

function drawHeatTimes() {
    if (!heatTimes) heatTimes = [];
    const nowSec = hour() * 3600 + minute() * 60 + second();

    strokeCap(ROUND);
    noFill();

    // Find if there’s already an ongoing heat (-1)
    let ongoingIndex = heatTimes.findIndex(t => t.end === -1);

    // If checkbox is checked and no ongoing heat exists, create one
    const btn_heat_cb = document.getElementById("btn_heat_cb");
    const btn_heat_cb_checked = btn_heat_cb && btn_heat_cb.checked;

    if (btn_heat_cb_checked && ongoingIndex === -1) {
        heatTimes.push({ start: ongoingHeatStart ?? nowSec, end: -1 });
        ongoingIndex = heatTimes.length - 1;
        ongoingHeatStart = heatTimes[ongoingIndex].start;
    }

    // If checkbox is unchecked and ongoing exists, close it
    if (!btn_heat_cb_checked && ongoingIndex !== -1) {
        heatTimes[ongoingIndex].end = nowSec;
        ongoingHeatStart = null;
        ongoingIndex = -1;
    }

    // Draw all heat periods
    for (let i = 0; i < heatTimes.length; i++) {
        let t = heatTimes[i];
        let start = t.start;
        let end = t.end;

        // If ongoing, extend to now
        let active = false;
        if (end === -1) {
            end = nowSec;
            active = true;
        }

        const wi = 10;
        const wo = 15;

        const startAngle = map(start % 43200, 0, 43200, 0, 360);
        const endAngle = map(end % 43200, 0, 43200, 0, 360);
        const isPM = start >= 43200;
        const r = (isPM ? RADIUS_INNER : RADIUS_OUTER) + wo * 2;

        // background arc
        stroke(255, 200, 180, 40);
        strokeWeight(wo);
        arc(0, 0, r, r, startAngle, endAngle);

        // main arc color
        if (active) {
            stroke(COLOR_NOW);
            strokeWeight(wi + 2);
        } else {
            stroke((isPM ? COLOR_PM_AFTER : COLOR_PM_BEFORE) + "B4"); // orangy for past
            strokeWeight(wi);
        }
        arc(0, 0, r, r, startAngle, endAngle);
    }
}


function drawCenterDot() {
    stroke(80);
    strokeWeight(18);
    point(0, 0);
}


function drawClockBorder() {
    noFill();
    stroke(80, 80, 80, 120);
    strokeWeight(1);
    ellipse(0, 0, RADIUS_CLOCK, RADIUS_CLOCK);
}

function drawHeatBackground() {
    noStroke();

    fill(COLOR_PM_BEFORE + "10");
    ellipse(0, 0, RADIUS_CLOCK, RADIUS_CLOCK);

    fill(COLOR_PM_AFTER + "10");
    ellipse(0, 0, RADIUS_OUTER, RADIUS_OUTER);
}

function drawClockNumbers() {
    push();

    rotate(90); // undo clock rotation so text is upright

    textAlign(CENTER, CENTER);
    noStroke();

    const r = 120; // radius for numbers
    const hrNow = hour(); // current hour in 24h format

    // offset numbers if current hour > 12
    const isPM = hrNow > 12;
    const offset = isPM ? 12 : 0;

    for (let i = 1; i <= 12; i++) {
        textSize(i % 3 === 0 ? 25 : 20);

        let angle = map(i, 0, 12, 0, 360) - 90;
        let x = cos(angle) * r;
        let y = sin(angle) * r;

        fill((isPM ? COLOR_PM_AFTER : COLOR_PM_BEFORE) + "A0");
        text(i + offset, x, y);
    }

    pop();
}


function drawClockTicks() {
    stroke(0);
    strokeWeight(2);

    for (let i = 0; i < 60; i++) {
        let angle = map(i, 0, 60, 0, 360);
        push();
        stroke(150, 150, 150, 40);
        rotate(angle);
        const is_hour = (i % 5 === 0);
        let len = is_hour ? 20 : 7; // longer tick for hours
        let a = 320 / 2 + (is_hour ? len / 4 : 0);
        let b = 0;
        line(a, b, a - len, b);
        pop();
    }
}


function draw() {
    clear();
    translate(width / 2, height / 2);
    rotate(-90);

    drawClockBorder();
    drawClockTicks();
    drawClockNumbers();
    drawHands();
    //drawHeatBackground();
    drawHeatTimes();
    drawCenterDot();
}
