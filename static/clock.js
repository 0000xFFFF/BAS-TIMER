let heatTimes = [
    { "start": 28800, "end": 32400 },
    { "start": 61200, "end": 64800 }
];

function preload() {
    fetch('/times')
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

function drawHeatTimes() {
    if (!heatTimes || heatTimes.length === 0) return;

    let outerRadius = 320; // AM
    let innerRadius = 250; // PM

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
    ellipse(0, 0, 360, 360);
}

function drawHeatBackground() {
    noStroke();

    // AM background (outer ring)
    fill(255, 120, 80, 20);
    ellipse(0, 0, 320, 320);

    // PM background (inner ring)
    fill(255, 120, 80, 12);
    ellipse(0, 0, 250, 250);
}


function draw() {
    clear();
    translate(width / 2, height / 2);
    rotate(-90);

    drawClockBorder();
    drawHands();
    drawHeatBackground();
    drawHeatTimes();
    drawCenterDot();
}
