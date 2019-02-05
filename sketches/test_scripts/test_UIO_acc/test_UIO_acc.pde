#include <WaspUIO.h>

void setup()
{
  UIO.pwr_i2c(1);
  //ACC.ON(); 
}

void loop()
{
  int x_acc, y_acc, z_acc;
  float angle_x, angle_y, angle_z;
  float tilt_x, tilt_y;

  x_acc = ACC.getX();
  y_acc = ACC.getY();
  z_acc = ACC.getZ();

  cr.println(F("raw (mG): %d %d %d"), x_acc, y_acc, z_acc);

  delay(500);
}
