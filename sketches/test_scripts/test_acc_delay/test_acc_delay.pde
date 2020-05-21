#include <WaspUIO.h>


void setup()
{
  int ms=0;

  UIO.pwr_i2c(1);
  ACC.ON();
  delay(ms);

  cr_printf("delay=%d\n", ms);
}


void loop()
{
  int x, y, z;
  int x_min, y_min, z_min;
  int x_max, y_max, z_max;

  x_min = y_min = z_min = INT_MAX;
  x_max = y_max = z_max = INT_MIN;

  for (int i=0; i < 100; i++)
  {
    x = ACC.getX();
    y = ACC.getY();
    z = ACC.getZ();

    if (x < x_min) x_min = x;
    if (y < y_min) y_min = y;
    if (z < z_min) z_min = z;

    if (x > x_max) x_max = x;
    if (y > y_max) y_max = y;
    if (z > z_max) z_max = z;
  }

  cr_printf("min: %d %d %d\n", x_min, y_min, z_min);
  cr_printf("max: %d %d %d\n", x_max, y_max, z_max);
  cr_printf("\n");

  delay(1000);
}
