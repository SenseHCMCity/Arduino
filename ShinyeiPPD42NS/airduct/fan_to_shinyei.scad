//
// An air duct to pump air through the Shinyei PPD42NS PM sensor.
//

// sensor input hole (see http://sca-shinyei.com/pdf/PPD42NS.pdf)
sensorInX = 12.0;
sensorInY = 3.5;
sensorInZ = 7.0;

// input nozzle
nozzleReduction = 0.1 * 2; 
nozzleX = sensorInX - nozzleReduction;
nozzleY = sensorInY - nozzleReduction;
nozzleZ = sensorInZ;
nozzleWallThickness = 0.5;

// fan
fanWidth = 40;
fanHoleDiameter = 30;
fanPlateThickness = 5;
fanPlateScrewHole = 5;
fanPlateDistanceBetweenScrews = 30;

// duct
ductHeight = 20;
ductWallThickness = 1;

module nozzle() {
    difference() { 
      // nozzle outer (walls)
      cube([nozzleX, nozzleY, nozzleZ], 
            center=true);
      // nozzle inner (air)
      cube([nozzleX - (nozzleWallThickness * 2), 
            nozzleY - (nozzleWallThickness * 2), 
            nozzleZ + 1], 
            center=true);
    }
}

module duct() {
    ductScale = sensorInX / fanWidth;
    ductBaseWidth = fanWidth - 1; // bit smaller as it sits on the fanplate
    difference() {
        // duct outer (walls)
        linear_extrude(height = ductHeight, scale = ductScale)
            square([ductBaseWidth, ductBaseWidth], 
                    center=true);
        // duct inner (air)
        ductInnerWidth = fanWidth - (ductWallThickness * 2);
        linear_extrude(height = ductHeight, scale = ductScale)
            square([ductInnerWidth, ductInnerWidth],
                    center=true);
    }
}

// Extended the fanplate from thingiverse:
//   Create by : Sherif Eid
//   sherif.eid@gmail.com
//   https://www.thingiverse.com/thing:735038
//
// Changes:
//  - added screw_hole module and extended the height and transform
//    positions of screw holes to ensure a complete hole
module fanplate(d,ls,t,ds) 
{
    /*
    d   : diameter of the fan
    ls  : distance between screws
    t   : wall thickness
    ds  : diameter of screws 
    */
    difference()
    {
        difference()
        {
            difference()
            {
                difference()
                {
                    translate([-0.45*d,-0.45*d,0]) minkowski()
                        { 
                         cube([d*0.9,d*0.9,t/2]);
                         cylinder(h=t/2,r=d*0.05);
                        }
                    screw_hole(ls/2, ls/2, ds, t);
                }
                screw_hole(ls/-2, ls/2, ds, t);
            }
            screw_hole(ls/-2, ls/-2, ds, t);
        }
        screw_hole(ls/2, ls/-2, ds, t);
    }
}
module screw_hole(tX, tY, ds, t) {
    translate([tX,tY,-1]) cylinder(d=ds,h=t+2);
}    

// Put together and add some holes
nozzlePosition = fanPlateThickness+ductHeight+3;
difference() {
    difference() {
        union() {
            fanplate(fanWidth, 
                    fanPlateDistanceBetweenScrews, 
                    fanPlateThickness, 
                    fanPlateScrewHole);
            color("red") translate([0, 0, fanPlateThickness]) 
                duct();
            color("green") translate([0, 0, nozzlePosition]) 
                nozzle();
        }
        // fanplate hole breaking through the duct
        translate([0, 0, -1]) 
            cylinder(d=fanHoleDiameter, h=fanPlateThickness+2);
    }
    // punch a hole where the nozzle and duct join
    translate([0, 0, nozzlePosition-2])
        cube([nozzleX - (nozzleWallThickness * 2), 
              nozzleY - (nozzleWallThickness * 2), 
              3], 
              center=true);
}


