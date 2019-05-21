Using floats in the mote has two problems:

- A float takes 4 bytes in a frame. Because frame size is small and network is
  a precious ressource (specially Iridium), it's important to make frames as
  small as possible. Send the smaller integer possible instead of float.
  See issue #50 for details.

- With 4 bytes floats errors in accuracy are possible, as we have observed.
  Avoid working with floats in the MCU.
  See issue #9 for details.

This page documents the sensors from a data point of view. We're interested in
three aspects:

1. How the information is read from the sensors: range, resolution and unit.
2. How the information is saved and sent in the frames (transport).
3. How the information is stored in the server.

Unless stated otherwise 1. and 3. are the same.

Eventually the information contained here may be used for more efficient
storage in the server as well.

The ranges are taken from the sensor specifications and/or the data read.
This information is used to determine the type to use in the frame and
the transformation needed to convert the float to an integer.

Unfortunately sometimes the specification does not have this data (e.g. air
temperature in DS-2 and Atmos), or worse, the data in the specification is
wrong (e.g. tilt in the Atmos).

We prefer signed over unsigned integers, even when the value cannot be
negative, unless using an unsigned allows us to reduce the frame size.

# SDI-12

SDI-12 sensors send the data as a text string. We have a library that works
with SDI-12, but we don't use other libraries specifically for a SDI-12 sensor.

This is to say that, unlike I2C sensors, we don't do transformations on the
input data. This makes it much easier to work with.

## CTD-10 (water)

|                   | Depth       | Temp         | BEC        |
| ----------------- | -----------:| ------------:| ----------:|
| Resolution & Unit |        2 mm |       0.1 ºC |    1 µS/cm |
| Input (example)   |        +542 |        +22.3 |       +645 |
| Range (spec)      |   0 : 10000 | -11.0 : 49.0 | 0 : 120000 |
| Range (read)      | -13 :  3959 |  -0.3 : 31.0 | 0 :   1330 |
| Transport (type)  |       int16 |        int16 |      int32 |
| Transform         |             |          x10 |            |


## DS-2 (wind)

|                   | Speed       | Direction    | Temp       | Meridional | Zonal    |     Gust |
| ----------------- | -----------:| ------------:| ----------:| ----------:| --------:| --------:|
| Resolution & Unit |    0.01 m/s |     1 degree |         \* |   0.01 m/s | 0.01 m/s | 0.01 m/s |
| Input (example)   |      +13.05 |          +53 |      +22.3 |     +10.45 |    +7.82 |   +13.05 |
| Range (spec)      |      0 : 30 |      0 : 359 |         \* |     0 : 30 |   0 : 30 |   0 : 30 |
| Transport (type)  |       int16 |        int16 |      int16 |      int16 |    int16 |    int16 |
| Transform         |        x100 |              |        x10 |       x100 |     x100 |     x100 |

\* The specification does not say the air temperature resolution and range.
But from the given example and the data we we can see in the log files: the
resolution is 0.1 ºC


## Atmos-22 (wind)

|                   | Speed       | Direction    | Gust       | Temp       | Tilt X     | Tilt Y     |
| ----------------- | -----------:| ------------:| ----------:| ----------:| ----------:| ----------:|
| Resolution & Unit |    0.01 m/s |     1 degree |   0.01 m/s |         \* | 0.1 degree | 0.1 degree |
| Input (example)   |           ? |            ? |          ? |          ? |          ? |          ? |
| Range (spec)      |      0 : 30 |      0 : 359 |     0 : 30 |         \* |    0 : 180 |    0 : 180 |
| Range (read)      |           ? |            ? |          ? |          ? |  -90 : +90 |  -90 : +90 |
| Transport (type)  |       int16 |        int16 |      int16 |      int16 |      int16 |      int16 |
| Transform         |        x100 |          x10 |       x100 |        x10 |        x10 |        x10 |

\* The specification does not say the air temperature resolution and range.
But from the data we we can see in the log files: the resolution is 0.1 ºC

Actually we have observed the sensor returns decimals for the direction,
unlike what is stated by the datasheet. But we've decided not to keep
those decimals, as they're noise.


## WS100-UMB

We don't use this one, not yet at least. Support for it may be dropped from the
mote, at least temporarily, to save program memory.

So for know we don't touch it. Still sending floats to the server.

TODO


# I2C

- ACC (x, y, z) and VL5L1X (distance)
  Their libraries already return integers.

- AS7263, AS7265, BM280, MLX90614 and TMP102 sensors
  The libraries return floats. Internally they transform integers to float
  doing some conversions. We would need to modify the libraries and do the
  conversions when parsing the frame (in the server or Pi). This is much more
  work.


# GPS

Latitude, longitude, altitude and accuracy are floats.


# Other sensors and data

- Battery level, DS18B20, MB73XX, RSSI
  Their libraries already return integers.

- Battery volts
  This one is float, but it's only used for lead-acid batteries, and we won't
  use it again in future deployments.
