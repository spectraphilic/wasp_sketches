# Waspmote Journal

Journal of the deployment of Waspmotes at Finse. 

Started Nov. 2017 by S. Filhol

**Last update: Jan. 30, 2018**

## Summary table



|   WASPMOTE_id    |      eAST |      nORTH | location Description |             time-period |
| :--------------: | --------: | ---------: | :------------------: | ----------------------: |
| 023D67057C105474 | 416970.35 | 6715977.25 |    Appelsinhytta     |      21/11/2017 - today |
| 197067057C10549F |        NA |         NA |          NA          |                         |
| 1F566F057C105487 | 417284.06 | 6717123.73 |        Hills         | 21/11/2017 - 24/11/2017 |
| 3F7C67057C105419 | 417951.37 | 6716590.57 | Middaselvi dsicharge |                         |
| 40516F057C105437 | 418116.92 |  6721025.8 |  Finselvi discharge  |                         |
| 607867057C1054D2 |           |            |      Marshland       |        13/12/2017-today |
| 667767057C10548E | 417284.06 | 6717123.73 |        Hills         |      14/12/2017 - Today |
| 6D4467057C1054DC | 419401.31 | 6718414.19 |  Drift lower lidar   |                         |
|                  |           |            |                      |                         |

Coordinates in UTM 32N, unit meter.

---

## Thomas Station

### 2018-01-27

 - Installation of a waspmote at the Thomas station in replacement to the pre-existing station (hobo with air temp and wind speed/dir + snowmo)
 - Sensors: DS-2, and ds18b20 for air temperature. 
 - Program: commit b627f18d72330e0ddb736dd03ad9d7960bbe921b
 - Lead acid battery
 - did not collected the mote ID and xbee ID. 

---

## Marshland

### 2018-01-26

Waspmote taken down and replaced by the mobile flux station. **WARNING** Any data passed this date are meaningless. 

### 2017-12-13

- Installation of a waspmote with Maxbotix and DS1820 sensor
- Waspmote ID: 607867057C1054D2
- Lead acid battery




---

## Drift lower lidar (6718414.19, 419401.31) 

### 2017-11-19

- waspmote ID 6D4467057C1054DC 
- uploaded script Finse_3 commit  [ceb90f9](https://github.com/spectraphilic/wasp_sketches/commit/ceb90f985209c339bfe596dd1aee61f71033df72) on . 
- UTC time, SD, GPS, lithium Bat, solar panel, Libelium box. 
- Sensors: DS-2. Sampling interval: 3m. Moved on the same pole as the drift lower lidar, right next to the radar. 


### 2018-01-25

Waspmote got burried in snow, sensor still out of snow.

------

## Applesinhytta (6715977.25, 416970.35)	

### 2017-11-20

Deployement of a waspmote v15 at appelsinhytta with the following config:

- waspmote ID: 023D67057C105474
- DS 2 sensor, 5min sampling
- Script commit [ceb90f9](https://github.com/spectraphilic/wasp_sketches/commit/ceb90f985209c339bfe596dd1aee61f71033df72)
- Lead acid battery with 10W solar panel


------

## The hill that wants to be a mountain (6717123.73, 417284.06)

### 2017-11-20

Deployement of a waspmote v15 with the following config:

- waspmote ID: 01f566f057c105487
- DS 2 sensor, 5min sampling
- Script commit [ceb90f9](https://github.com/spectraphilic/wasp_sketches/commit/ceb90f985209c339bfe596dd1aee61f71033df72)
- Lead acid battery with 10W solar panel

### 2017-12-14

This staion stopped nov 24. 
 - Battery voltage: 12.4
 - Station was totaly covered in ice
 - Waspmote box full of snow, my theory is that the lid was installed 90 deg of
 - Copied log-file and data from SD
 - Connecting with serial monitor gave no thing
 - Tried to upload a new program but that didn't work
 - Changed to a new waspmote
 - Changed to a longer antenna (8 dbi)
 - Broke the GPS antenna when i changed waspmote, so now there are no GPS installed
 - I have tried the unit i changed, and now when it's dry it works fine
 - ...but with the GPS it keeps bugging for me. Might be that the GPS is more sensitive to humidity???
 - I have nice pictures showing riming on the wind sensor.
 - New station ID: 667767057C10548E


------

## Middalselvi (6716590.57, 417951.37)

### 2017-11-20

Collect of the data from SD card. battery at 94%. Sven try pushing new script with no success. needs maintenance. Waspmote v12 with Libelium box and solar panel. Lithium battery. CTD-10 sensor.

### 2017-11-20

Swithced the v12 waspmote for a v15 (serial ID: 3F7C67057C105419) 

------

## Finselvi discharge (6721025.8, 418116.92)

Waspmote v15 Lead Acid battery, 10W solar panel, CTD 10 sensor. GPS. 40516F057C105437

### 2017-11-21

Pushed latest script but SD card would not be accessible byt the waspmote. Sd card read fine on the laptop. 

------

## Finselvi Dam ()

### 2017-11-21

Switched waspmote v12 for a v15 (wasp ID: 01857f322),. No sensor, lithium battery with solar panel (libelium box). Script [dad3fb4](https://github.com/spectraphilic/wasp_sketches/commit/dad3fb407ec0cb60f96c1c493ae6eb938b39006b) pushed to it.



------

## Flux station location ( Deprecated)

### 2017-11-18 08:26

waspmote ID 6D4467057C1054DC: uploaded script Finse_3 commit  [ceb90f9](https://github.com/spectraphilic/wasp_sketches/commit/ceb90f985209c339bfe596dd1aee61f71033df72) on . UTC time, SD, GPS, lithium Bat, solar panel, Libelium box. Sensors: DS-2. Sampling interval: 3m

### 2017-11-19

**NO MORE WASPMOTE, Being moved to Drift Lower Lidar **