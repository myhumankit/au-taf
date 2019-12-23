$fn=100;
w=74.5;
W=78.5;
L=95;
r = 4;
d=8;
D=13;
a=18;
b=8;
z = 4.01;
profil = [[0,0],[-0.9,0.3],[-2.2,1.3],[-4.0,3.0],[-4.1,4],[-4.1,8.0],[-3.7,8.7],[-2.2,9.5],[0.0,10.3]];
difference() {
    union() {
        cube([W,L,D],[0,0,0]);
        translate([W/2-10,-25,0]) cube([20,25,D]);
        polyhedron([[0,20,0],[-5,15,0],[-5,L-15,0],[0,L-20,0],
                     [0,20,D],[-5,15,D],[-5,L-15,D],[0,L-20,D]],
                    [[0,3,2,1],[4,5,6,7],
                     [4,0,1,5],[5,1,2,6],[6,2,3,7],[7,3,0,4]]);
        polyhedron([[W,20,0],[W,L-20,0],[W+5,L-15,0],[W+5,15,0],
                     [W,20,D],[W,L-20,D],[W+5,L-15,D],[W+5,15,D]],
                    [[0,3,2,1],[4,5,6,7],
                     [4,0,1,5],[5,1,2,6],[6,2,3,7],[7,3,0,4]]);
        
        polyhedron([[5,0,0],[W/2-10,0,0],[W/2-10,-20,0],
                     [5,0,D],[W/2-10,0,D],[W/2-10,-20,D]],
                    [[0,1,2],[5,4,3],
                     [0,3,4,1],[1,4,5,2],[2,5,3,0]]);
        polyhedron([[W-5,0,0],[W/2+10,0,0],[W/2+10,-20,0],
                     [W-5,0,D],[W/2+10,0,D],[W/2+10,-20,D]],
                    [[0,2,1],[5,3,4],
                     [1,4,3,0],[1,2,5,4],[0,3,5,2]]);
        
    }
    translate([7,10,2]) rotate([0,0,180]) rotate_extrude(angle=90, convexity=1) {polygon(profil);};
    translate([W-7.5,10,2]) rotate([0,0,180]) rotate_extrude(angle=90, convexity=1) {polygon(profil);};
        polyhedron([[a,b,z],[a+8,b,z],[a+6,b-5,z],[a+2,b-5,z],
                     [a-2,b,z+9],[a+10,b,z+9],[a+13,b-15,z+9],[a-5,b-15,z+9]],
                    [[3,2,1,0],[4,5,6,7],
                     [0,1,5,4],[2,3,7,6],
                     [0,4,7],[7,3,0],[5,1,6],[2,6,1]]);
    //translate(,[1,5,1]) cube([74,91,8]);
    translate([6.5,10,2]) cube([65.5,86,D+1]);
    //translate([(W-w)/2+r,15.5,5.5])  rotate([270,0,0]) cylinder(r=r,h=81);

    translate([(W-w)/2+r+1,60,2]) rotate([90,0,0]) linear_extrude(height = 100, center = true, convexity = 10, twist = 0, slices = 20, scale = 1.0, $fn = 16) 
    {polygon(profil);};

    //translate([W-((W-w)/2+r),15.5,5.5])         rotate([270,0,0]) cylinder(r=r,h=81);
    translate([w-3,60,2]) rotate([90,0,180]) linear_extrude(height = 100, center = true, convexity = 10, twist = 0, slices = 20, scale = 1.0, $fn = 16) 
    {polygon(profil);};

    //translate([12,9,5.5])         rotate([0,90,0]) cylinder(r=r,h=W-24);
    translate([W/2,10.01,2]) rotate([90,0,0]) rotate([90,90,90]) linear_extrude(height = 63, center = true, convexity = 10, twist = 0, slices = 20, scale = 1.0, $fn = 16) 
    {polygon(profil);};
    //translate([13,16,5.5]) rotate([0,0,180]) rotate_extrude(angle=90, convexity=1) translate([7, 0]) circle(r=r);
    //translate([W-13,16,5.5]) rotate([0,0,270]) rotate_extrude(angle=90, convexity=1) translate([7, 0]) circle(r=r);
    translate([W/2-2,-26,6]) cube([4,31,8]);
    translate([W/2,-26,6]) rotate([-90,0,0]) cylinder(r=2,h=31);
    translate([W/2,-22.5,6]) rotate([-90,0,0]) cylinder(r=3.6,h=10);
    translate([W/2-4,-22.5,6]) cube([8,10,8]);
    translate([W/2-6.5,-13,2]) cube([13,20,12]);
}
//translate([0,60,0])
//   rotate_extrude(angle=270, convexity=10)
//       translate([40, 0]) circle(10);
//rotate_extrude(angle=90, convexity=10)
//   translate([20, 0]) circle(10);

//translate([20,0,0]) 
//   rotate([90,0,0]) cylinder(r=10,h=80);

