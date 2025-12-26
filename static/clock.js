function setup() {
	var cnv = createCanvas(400, 400);
	cnv.id('aclock');
	cnv.style('display', 'block');
	cnv.parent('clock_box');
	
	angleMode(DEGREES);
}

function clock_color_sec()    { stroke(190, 190, 190); }
function clock_color_min()    { stroke(150, 150, 150); }
function clock_color_hour()   { stroke(110, 110, 110); }
function clock_color_button() { stroke(70,   70,  70); }

function draw() {
	clear();
	translate(200, 200);

	
	fill(255);
	noStroke(2);
	let fontSize = 50;
	textSize(fontSize);

	rotate(-90);
	
	let hr = hour();
	let mn = minute();
	let sc = second();
	
	let dis = 360;
	let disf = 40;

	// all
	strokeWeight(15);
	noFill();

	// sec
	clock_color_sec();
	let secondAngle = map(sc, 0, 60, 0, 360);
	arc(0, 0, dis, dis, 0, secondAngle);
	
	// min
	clock_color_min();
	let minuteAngle = map(mn, 0, 60, 0, 360);
	arc(0, 0, dis-disf, dis-disf, 0, minuteAngle);
	
	// hour
	clock_color_hour();
	let hourAngle = map(hr % 12, 0, 12, 0, 360);
	arc(0, 0, dis-disf*2, dis-disf*2, 0, hourAngle);
	
	push();
	strokeWeight(3);
	rotate(secondAngle);
	clock_color_sec();
	line(0, 0, 125, 0);
	pop();
	
	push();
	strokeWeight(6);
	rotate(minuteAngle);
	clock_color_min();
	line(0, 0, 100, 0);
	pop();
	
	push();
	strokeWeight(9);
	rotate(hourAngle);
	clock_color_hour();
	line(0, 0, 75, 0);
	pop();
	
	clock_color_button();
	strokeWeight(20);
	point(0, 0);
}
