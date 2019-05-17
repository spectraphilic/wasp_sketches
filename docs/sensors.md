
This page documents the sensors from a data point of view. We're intersted in
three aspects:

1. How the information is read from the sensors: range, resolution and unit.
2. How the information is saved and sent in the frames.
3. How the information is stored in the server.

Unless stated otherwise 1. and 3. are the same.

The information may be transformed for transport. Here we try to avoid floats
and use only integer, for two purposes:

- Reduce the frame size, see issue #50
- Avoid accuracy errors, see issue #9

Eventually the information contained here may be used for more efficient
storage in the server as well.

The ranges are taken from the sensor specifications, and are used to determine
the type to use in the frame. However we have observed sometimes values not
within the specified range, so it's best to analyze first the real data we've.


CTD-10
========================================================================

|                  | Depth       | Temp         | BEC        |
| ---------------- | -----------:| ------------:| ----------:|
| Input (example)  |        +542 |        +22.3 |       +645 |
| Range (spec)     |   0 : 10000 | -11.0 : 49.0 | 0 : 120000 |
| Range (read)     | -13 :  3959 |  -0.3 : 31.0 | 0 :   1330 |
| Transport (type) |       int16 |        int16 |      int32 |
| Transform        |             |          x10 |            |
