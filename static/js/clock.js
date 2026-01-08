let heatTimes = [];
let hoveredHeat = null; // track which heat period is hovered

const RADIUS_CLOCK = 400;
const RADIUS_OUTER = 350;
const RADIUS_INNER = 320;

const COLOR_PM_BEFORE = "#D1BE45";
const COLOR_PM_AFTER = "#FF7850";

const COLOR_NOW = "#FF0000";

function preload() {
    fetch("/api/times")
        .then((res) => res.json())
        .then((data) => {
            heatTimes = data;
        })
        .catch((err) => {
            console.error("Failed to load /times", err);
        });
}

function setup() {
    let cnv = createCanvas(400, 400);
    cnv.id("aclock");
    cnv.style("display", "block");
    cnv.parent("clock_box");

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

function formatSeconds(sec) {
    const h = Math.floor(sec / 3600);
    const m = Math.floor((sec % 3600) / 60);
    const s = Math.floor(sec % 60);
    let parts = [];
    if (h > 0) parts.push(h + "h");
    if (m > 0) parts.push(m + "m");
    if (s > 0 || parts.length === 0) parts.push(s + "s");
    return parts.join(" ");
}

function formatTimeOfDay(sec) {
    const h = Math.floor((sec % 86400) / 3600);
    const m = Math.floor((sec % 3600) / 60);
    const s = Math.floor(sec % 60);
    return `${h.toString().padStart(2, "0")}:${m.toString().padStart(2, "0")}:${s
        .toString()
        .padStart(2, "0")}`;
}

function drawHeatTimes() {
    if (!heatTimes) heatTimes = [];
    const nowSec = hour() * 3600 + minute() * 60 + second();

    strokeCap(ROUND);
    noFill();

    // Find if there's already an ongoing heat (-1)
    let ongoingIndex = heatTimes.findIndex((t) => t.end === -1);

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

    // Mouse position relative to center (accounting for -90 rotation)
    const mx = mouseX - width / 2;
    const my = mouseY - height / 2;
    const mouseDist = sqrt(mx * mx + my * my);
    let mouseAngle = atan2(my, mx) + 90; // +90 to undo the -90 rotation
    if (mouseAngle < 0) mouseAngle += 360;

    hoveredHeat = null;

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

        // Check hover
        const arcRadius = r / 2;
        const hitMargin = wo;
        const inRadius =
            mouseDist > arcRadius - hitMargin && mouseDist < arcRadius + hitMargin;
        let inAngle = false;
        if (endAngle >= startAngle) {
            inAngle = mouseAngle >= startAngle && mouseAngle <= endAngle;
        } else {
            inAngle = mouseAngle >= startAngle || mouseAngle <= endAngle;
        }
        if (inRadius && inAngle) {
            hoveredHeat = { start, end, duration: end - start, active };
        }

        // background arc
        stroke(255, 200, 180, 40);
        strokeWeight(wo);
        arc(0, 0, r, r, startAngle, endAngle);

        // main arc color
        if (active) {
            stroke(COLOR_NOW);
            strokeWeight(wi + 2);
        } else {
            stroke((isPM ? COLOR_PM_AFTER : COLOR_PM_BEFORE) + "B4");
            strokeWeight(wi);
        }
        arc(0, 0, r, r, startAngle, endAngle);
    }
}

function drawTooltip() {
    if (!hoveredHeat) return;

    resetMatrix();

    const padding = 8;
    const lineHeight = 16;

    const lines = [
        `Start: ${formatTimeOfDay(hoveredHeat.start)}`,
        `End: ${hoveredHeat.active ? "ongoing" : formatTimeOfDay(hoveredHeat.end)}`,
        `Duration: ${formatSeconds(hoveredHeat.duration)}`,
    ];

    textSize(12);
    textAlign(LEFT, TOP);

    const maxW = Math.max(...lines.map((l) => textWidth(l)));
    const boxW = maxW + padding * 2;
    const boxH = lines.length * lineHeight + padding * 2;

    let tx = mouseX + 15;
    let ty = mouseY + 15;
    if (tx + boxW > width) tx = mouseX - boxW - 5;
    if (ty + boxH > height) ty = mouseY - boxH - 5;

    fill(40, 40, 40, 230);
    stroke(100);
    strokeWeight(1);
    rect(tx, ty, boxW, boxH, 4);

    noStroke();
    fill(255);
    for (let i = 0; i < lines.length; i++) {
        text(lines[i], tx + padding, ty + padding + i * lineHeight);
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
    const nowSec = hour() * 3600 + minute() * 60 + second();
    const isPM = nowSec >= (12 * 3600)
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
        const is_hour = i % 5 === 0;
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

    drawTooltip();
}
