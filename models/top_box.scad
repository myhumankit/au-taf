$fn = 50;

corners = [[6,6,0],[94,6,0],[6,174,0],[94,174,0]];
ir_pods = [[25,160,0],[75,160,0]];
ir_screws = [[42.3, 145.5, 0],[50.0, 159.0, 0],[57.7, 145.5, 0]];

difference() {
	union() {
		for (i = corners) {
			translate(i+[-3,-3,2]) cube([6.0, 6.0, 6.0]);
		}
		translate([26.5, 20.0, 2.0]) cube([47.0, 65.0, 4.0]);
		for (i = ir_pods) {
			translate(i+[0, 0, 2.0]) cylinder(6,r=3);
		}
		difference() {
			translate([0.0, 0.0, 0.0]) cube([100.0, 180.0, 10.0]);
			union() {
				translate([2.0, 2.0, 8.0]) cube([96.0, 176.0, 2.1]);
				translate([4.0, 4.0, 2.0]) cube([92.0, 172.0, 6.1]);
			}
		}
	}
	union() {
		for (i = corners) {
			translate(i+[0,0,-0.1]) cylinder(8.2,r=1.5);
			translate(i+[0,0,-0.1]) cylinder(3,r1=3,r2=0);
		}
		// USB STM32 translate([44.5, -1.0, 6.5]) cube([11.0, 6.0, 3.5]);
		translate([35.0, 82.0, 4.5]) cube([13.0, 4.0, 2.0]);
		translate([28.5, 22.0, 2.0]) cube([43.0, 61.0, 4.1]);
		translate([30.0, 24.0, -0.1]) cube([40.0, 55.0, 2.2]);
		for (i = ir_screws) {
			translate(i+[0, 0, -0.1]) cylinder(4,0r=1);
		}
		translate([50.0, 150.0, -0.1]) cylinder(4.0,r=5.0);
		for (i = ir_pods) {
			translate(i+[0, 0, 4.0]) cylinder(4.1,r=1);
		}
	}
}

