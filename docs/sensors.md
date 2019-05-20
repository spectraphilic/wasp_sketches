This page documents the sensors from a data point of view. We're interested in
three aspects:

1. How the information is read from the sensors: range, resolution and unit.
2. How the information is saved and sent in the frames (transport).
3. How the information is stored in the server.

Unless stated otherwise 1. and 3. are the same.

The information may be transformed for transport. Here we try to avoid floats
and use only integer, for two purposes:

- Reduce the frame size, see issue #50
- Avoid accuracy errors, see issue #9

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


CTD-10 (water)
========================================================================

|                   | Depth       | Temp         | BEC        |
| ----------------- | -----------:| ------------:| ----------:|
| Resolution & Unit |        2 mm |       0.1 ºC |    1 µS/cm |
| Input (example)   |        +542 |        +22.3 |       +645 |
| Range (spec)      |   0 : 10000 | -11.0 : 49.0 | 0 : 120000 |
| Range (read)      | -13 :  3959 |  -0.3 : 31.0 | 0 :   1330 |
| Transport (type)  |       int16 |        int16 |      int32 |
| Transform         |             |          x10 |            |


DS-2 (wind)
========================================================================

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


Atmos (wind)
========================================================================

|                   | Speed       | Direction    | Gust       | Temp       | Tilt X     | Tilt Y     |
| ----------------- | -----------:| ------------:| ----------:| ----------:| ----------:| ----------:|
| Resolution & Unit |    0.01 m/s |     1 degree |   0.01 m/s |         \* | 0.1 degree | 0.1 degree |
| Input (example)   |           ? |            ? |          ? |          ? |          ? |          ? |
| Range (spec)      |      0 : 30 |      0 : 359 |     0 : 30 |         \* |    0 : 180 |    0 : 180 |
| Range (read)      |           ? |            ? |          ? |          ? |  -90 : +90 |  -90 : +90 |
| Transport (type)  |       int16 |        int16 |      int16 |      int16 |       int8 |       int8 |
| Transform         |        x100 |              |       x100 |        x10 |        x10 |        x10 |

\* The specification does not say the air temperature resolution and range.
But from the data we we can see in the log files: the resolution is 0.1 ºC
