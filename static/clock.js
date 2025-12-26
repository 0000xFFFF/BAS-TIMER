let heatTimes = [];

const RADIUS_CLOCK = 400;
const RADIUS_OUTER = 380;
const RADIUS_INNER = 350;

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
    let cnv = createCanvas(500, 500);
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

function drawHeatTimes() {
    if (!heatTimes || heatTimes.length === 0) return;

    let outerRadius = RADIUS_OUTER; // AM
    let innerRadius = RADIUS_INNER; // PM

    strokeCap(ROUND);
    noFill();

    for (let t of heatTimes) {
        let start = constrain(t.start, 0, 86400);
        let end = constrain(t.end, 0, 86400);

        // Map 24h -> 12h
        let startAngle = map(start % 43200, 0, 43200, 0, 360);
        let endAngle = map(end % 43200, 0, 43200, 0, 360);

        let isPM = start >= 43200;
        let r = isPM ? innerRadius : outerRadius;

        // glow
        stroke(255, 120, 80, 40);
        strokeWeight(18);
        arc(0, 0, r, r, startAngle, endAngle);

        // main arc
        stroke(255, 120, 80, 180);
        strokeWeight(10);
        arc(0, 0, r, r, startAngle, endAngle);
    }

    // TODO: draw current heat
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

    // AM background (outer ring)
    fill(255, 120, 80, 8);
    ellipse(0, 0, RADIUS_OUTER, RADIUS_OUTER);

    // PM background (inner ring)
    fill(255, 120, 80, 5);
    ellipse(0, 0, RADIUS_INNER, RADIUS_INNER);
}

function drawClockNumbers() {
    push();

    // undo clock rotation so text stays upright
    rotate(90);

    textAlign(CENTER, CENTER);
    textSize(14);
    fill(255, 80, 80, 160);
    noStroke();

    let r = 120; // radius for numbers

    for (let i = 1; i <= 12; i++) {
        if (i % 3 === 0) {
            textSize(25);
        } else {
            textSize(20);
        }

        let angle = map(i, 0, 12, 0, 360) - 90;

        let x = cos(angle) * r;
        let y = sin(angle) * r;

        text(i.toString(), x, y);
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
        let a = 320/2 + (is_hour ? len/4 : 0);
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
    drawHeatBackground();
    drawHeatTimes();
    drawCenterDot();
}
