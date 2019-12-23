L=95;
$fn=100;
r = 4;
D=13;
h=35;
a=35;
f=3;
encoche=5;
zz = 25;
c = 20;
d = 5;
butee = 2;
points1 = [[-D,encoche,0],
           [-D+h*cos(a),encoche-h*sin(a),0],
           [-D+h*cos(a)+(L+zz)*sin(a)*cos(a),encoche-h*sin(a)+(L+zz)*cos(a)*cos(a),0],
		   [0,L-2*encoche,0],
           [-D,encoche,f],
		   [-D+h*cos(a),encoche-h*sin(a),f],
           [-D+h*cos(a)+(L+zz)*sin(a)*cos(a),encoche-h*sin(a)+(L+zz)*cos(a)*cos(a),f],
		   [0,L-2*encoche,f]];
faces1 = [[0,1,2,3],[7,6,5,4],
          [0,4,5,1],[1,5,6,2],
          [2,6,7,3],[3,7,4,0]];
				
points2 = [[0,c,0],[-5,c-d,0],[-5,L-c+d,0],[0,L-c,0],
           [0,c,D],[-5,c-d,D],[-5,L-c+d,D],[0,L-c,D],
           [0,5,0],[-10,5,0],[-10,L-10,0],[0,L-10,0],
           [0,5,D],[-10,5,D],[-10,L-10,D],[0,L-10,D]];  
faces2 = [[4,0,8,12],[12,8,9,13],[9,10,14,13],[10,11,15,14],
		  [11,3,7,15],[5,1,0,4],[3,2,6,7], [1,5,6,2],
          [8,0,1,2,3,11,10,9],[5,4,12,13,14,15,7,6]];

cylinder_pos = [-D+h*cos(a)+(L+15)/2*sin(a)*cos(a),5-h*sin(a)+(L+15)/2*cos(a)*cos(a),-0.5];

difference() {
	union() {
		translate([0,5,0]) cube([butee,L-15,2*d]);
		polyhedron(points1,faces1);
        translate([0,0,10]) rotate([0,270,0]) 
        polyhedron(points2, faces2);
	};
	translate(cylinder_pos) cylinder(r=h*0.8,h=f+1);
}

translate([-32,0,0]) mirror(0,0,1) difference() {
	union() {
		translate([0,5,0]) cube([2,L-15,2*d]);
		polyhedron(points1,faces1);
        translate([0,0,10]) rotate([0,270,0]) 
        polyhedron(points2, faces2);
	};
	translate(cylinder_pos) cylinder(r=h*0.8,h=f+1);
}

