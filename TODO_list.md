# Waspmote programming TODO list
last update: August 24, 2016 by Simon

[TOC]

## 1. setup function
- define catalog of typical waspmote logger sensor configuration
- ~~create a single function in UIO library to generate setup in a single line in main script~~

## 2. record/send data
- ~~create framework for recording data on a tmp.txt file, then send lines (=frames) through xbee later in the day and move simultenuously frames to datalogger.txt file~~
- **test NEEDED**

## 3. ~~log waspmote activity on local SD card~~
- ~~create function that take as argument (message) that need to be logged onto the file log.log~~
- ~~make sure that if ever the RTC is on, it records the timestamp in YYYY:mm:dd HH:MM:SS format~~
- ~~**test the function on a waspmote**~~

## 4. remote access to waspmote
- ~~write function to receive function throuhg OTA on a waspmote, and log progress on logfile~~
- dig into the tricks for accessing remotely the waspmote using the OTA pipeline
- **test NEEDED**

## ~~5. RTC update coordination~~
- come up with a plan on how to synchronize RTC time on every waspmote of the network
_Maybe we can have the Meshlium sending a frame onto the network with a timestamp we use for updating local RTC clocks (would this be feasable?)_
**DONE by J.**

## 6. Comfirm locally connection to Meshlium
- Implement a function that check connection to Meshlium before sending any frame out on the network.
- **test NEEDED**

## 7. Practical script

- write script that allow various manipulation of the waspmote (e.g. read file SDcard, format SDcard, check sensors,... )






# TODO table
| programmer | task | date | DONE |
|--------|--------|--------|--------|
|      john  |   library: setup function     | august 23 |In Progress|
|      simon  |   library: SD card      | august 29 |Done! |
|      simon  |   script: read lines logfile on SD      | august 29 |Done! |
|      simon  |   script: format SD      | august 29 |Done! |
|      simon  |   library: log activity     | august 29 | Done! |
|      simon  |   library: send frame     | august 29 | TEST |
|      simon  |   library: OTA      | august 29 | Nope |


## Useful ressources
- [Waspmote API](https://www.libelium.com/api/waspmote/)
- [Markdown cheat sheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet)


