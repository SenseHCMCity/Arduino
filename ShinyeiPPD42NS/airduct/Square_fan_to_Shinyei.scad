/*
   Creates an air duct connector from a square frame fan to the rectangular Shinyei sensor input.
*/

// Using the This-To-That tweening library from
// https://www.thingiverse.com/thing:161646 to produce the main part 
// of the pipe.
use <tween_loft.scad>
include <tween_shapes.scad>

// -------------------Tween Parameters ----------------------------------------

// The lower shape
shape1				= tween_rectangle;
shape1Size 		= 4;			// Size of the lower shape
shape1Rotation 	= 0;			// Rotation of the lower shape
shape1Extension 	= 2;			// Extend the profile (space for tube clamp, etc.)
shape1Centroid  	= [0,0];			// Location of center point

// The upper shape
shape2				= tween_square;
shape2Size 		= 40;			// Size of the lower shape
shape2Rotation 	= 0;			// Rotation of the lower shape
shape2Extension 	= 0;			// Extend the profile (space for tube clamp, etc.)
shape2Centroid  	= [0,0];			// Location of center point

wallThickness		= 2;
isHollow 			= 1;			// If 1, create a tube.  If 0, create a solid.
extrusionHeight	= 30;			// Height of the loft

extrusionSlices 	= 30;	
sliceAdjustment	= 0;				// Ensure the slices intersect by this amount, 
									// needed if OpenSCAD is generating more than 2 volumes for an STL file

sliceHeight = extrusionHeight * 1.0 / extrusionSlices;	// Calculate the height of each slice

// Sensor air input hole (mm)
sensorInWidth = 11.0;
sensorInDepth = 3.0;
sensorInHeight = 7.0; 
shinyeiInsertHeight = sensorInHeight;
shinyeiInsertWidth = sensorInWidth - 1;
shinyeiInsertDepth = sensorInDepth - 1;

module shinyeiInsert() {
  rotate([0, 0, 0]) {
    translate([0,0,funnelHeight + sensorInDepth + base_thickness]) difference() { 
      // make the insert slightly smaller then the hole itself  
      cube([shinyeiInsertWidth, shinyeiInsertDepth, shinyeiInsertHeight], center=true);
      // inner air shinyei rectangle smaller again
      cube([shinyeiInsertWidth - 1, shinyeiInsertDepth - 1, shinyeiInsertHeight + 1], center=true);
    }
  }
}

module tube() {
  // Generate the tube
  difference()
  {
	// Make the container
	tweenLoft(shape1, shape1Size, shape1Rotation, shape1Centroid, shape1Extension,
			shape2, shape2Size, shape2Rotation, shape2Centroid, shape2Extension, shape2ExtensionAdjustment,
			extrusionSlices, sliceHeight, sliceAdjustment, wallThickness/2, isHollow);
  }
}

tube();

difference() {
  union() {
    // shinyei insert
    translate([0,0,-5.25]) shinyeiInsert();
    // a lid closing the tube and the insert
    translate([0,0,-2]) cube([shinyeiInsertWidth, shinyeiInsertDepth * 3, 0.5], center=true);
  }
  translate([0,0,-2]) cube([shinyeiInsertWidth - 1, shinyeiInsertDepth - 1, 2], center=true);
}