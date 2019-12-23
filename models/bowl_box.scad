l1 = 15.5;
l2 = 6.5;
w = 2;
h = -22;
points = [[0,-w/2,0], [l1,-w/2,0], [l1+l2,-w/2,h], [0,w/2,0], [l1,w/2,0], [l1+l2,w/2,h]];
faces=[[0,1,2],[3,5,4],[0,3,4,1],[0,2,5,3],[1,4,5,2]];
profil = [[0,0],[21,0],[30,25],[21,50],
         [5,50],[5,47],[19.5,47],[27,25],[19.5,3],[0,3]];
union() {
    difference()
    {   
        rotate_extrude($fn=200) polygon( points=profil);
		for (i = [0,90,180,270]) {
			rotate(i,[0,0,1]) translate([17,0,36]) rotate(71,[0,1,0]) cylinder($fn=50,10,r=2);
		}
		for (i = [45,135,225,315]) {
			rotate(i,[0,0,1]) translate([17,0,14]) rotate(109,[0,1,0]) cylinder($fn=50,10,r=2);
		}
        translate([9,0,46]) cylinder($fn=50,5,r=1);
        translate([-4.5,-7.7,46]) cylinder($fn=50,5,r=1);
        translate([-4.5,7.7,46]) cylinder($fn=50,5,r=1);
    }
    
	for (i = [45,135,225,315]) {
		rotate(i,[0,0,1]) translate ([5,0,47]) polyhedron(points,faces);
	}
}