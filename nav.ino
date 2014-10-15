//const float Pi = 3.14159;

int getHeading() {
  int heading;
  
  compass.read();
  heading = compass.heading((LSM303::vector<int>){0,-1,0}); 
  heading = heading + compassOffset;
  if (heading < 0) {
    heading = 360 + heading;
  }
    
  return heading;
}



int compassDirection(float compassHeading) { //returns which led should light up
  float sep = 360.0/16.0;
  int ledNum = 0;
  compassHeading = compassHeading + (sep/2); // shift it so lets will be in the middle of the range
  if(compassHeading > 360) { compassHeading = compassHeading - 360; }

  ledNum = compassHeading / sep;

  return ledNum;

}


int calcBearing(float flat1, float flon1, float flat2, float flon2) {
  float calc;
  float bear_calc;

  float x = 69.1 * (flat2 - flat1); 
  float y = 69.1 * (flon2 - flon1) * cos(flat1/57.3);

  calc = atan2(y,x);

  bear_calc= degrees(calc);

  if(bear_calc<=1){
    bear_calc=360+bear_calc; 
  }
  return 360 - bear_calc;
}
