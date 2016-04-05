//
// An air duct to pump air through the Shinyei PPD42NS PM sensor.
//

// sensor input hole (see http://sca-shinyei.com/pdf/PPD42NS.pdf)
sensorInX = 12.0;
sensorInY = 3.5;
sensorInZ = 7.0;
sensorScrewHole = 4.5;
sensorScrewToInDistance = 11.6 + (sensorScrewHole / 2);

// input nozzle
nozzleReduction = 0.1 * 2; 
nozzleX = sensorInX - nozzleReduction;
nozzleY = sensorInY - nozzleReduction;
nozzleZ = sensorInZ;
nozzleWallThickness = 0.5;

// fan (based on sunon 40mm fan specs: http://www.sunon.com/tw/products/pdf/acfan.pdf)
fanWidth = 40;
fanHoleDiameter = 37.5;
fanPlateThickness = 5;
fanPlateScrewHole = 4.3;
fanPlateDistanceBetweenScrews = 32;

// duct
ductHeight = 15;
ductWallThickness = 3;
ductTopThickness = 2;
ductBaseWidth = fanHoleDiameter + 2; // bit bigger then the hole
ductScale = sensorInX / ductBaseWidth;

// nozzle outer (walls)
module nozzle_outer() {
    cube([nozzleX, nozzleY, nozzleZ], center=true);
}
// nozzle inner (air)
module nozzle_inner() {
    translate([0, 0, -(ductTopThickness + 1)])
        cube([nozzleX - (nozzleWallThickness * 2), 
              nozzleY - (nozzleWallThickness * 2), 
              nozzleZ + duckTopThickness + 3],  // enough to punch hole through top and bottom 
              center=true);
}

module duct() {
    difference() {
        // duct outer (walls)
        linear_extrude(height = ductHeight, scale = ductScale)
            circle(d=ductBaseWidth, center=true);
        // duct inner (air)
        ductInnerWidth = ductBaseWidth - (ductWallThickness * 2);
        linear_extrude(height = ductHeight-ductTopThickness, scale = ductScale)
            circle(d=ductInnerWidth, center=true);
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

module duct_to_sensor_connector() {
    plateWidth = sensorScrewHole + 2;
    plateHeight = 2;
    rotate([180, 0, 180]) union() {
        // connecting arm
        translate([sensorScrewHole/2, -plateWidth/2, -plateHeight/2])
            cube([2, plateWidth, 7]);
        // screw plate
        difference() {
            cube([plateWidth, plateWidth, plateHeight], center=true);
            cylinder(d=sensorScrewHole, h=plateHeight+0.2, center=true);
        }
    }
}


// Put together and add some holes
nozzlePositionZ = fanPlateThickness+ductHeight+3;
difference() {
    difference() {
        difference() {
            union() {
                fanplate(fanWidth, 
                        fanPlateDistanceBetweenScrews, 
                        fanPlateThickness, 
                        fanPlateScrewHole);
                translate([0, 0, fanPlateThickness]) 
                    duct();
                translate([0, 0, nozzlePositionZ]) 
                    nozzle_outer();
                translate([sensorScrewToInDistance, 0,
                          fanPlateThickness+ductHeight])
                    duct_to_sensor_connector();        
            }
            nozzle_inner();
        }
        // fanplate hole breaking through the duct
        translate([0, 0, -0.1]) 
            cylinder(d=fanHoleDiameter, h=fanPlateThickness+0.2);
    }
    // punch a hole where the nozzle and duct join
    translate([0, 0, nozzlePositionZ-2])
        cube([nozzleX - (nozzleWallThickness * 2), 
              nozzleY - (nozzleWallThickness * 2), 
              3], 
              center=true);
}
