

#Script to record serial monitor to file

# see if we can push hex sketch to board from script

# print to serial list of file on SD
# input to serial filename to download
# get number of line of the file and size
#!/usr/bin/python

# choose number of line to download from the end of file ar entire file

# line by line download data on file
# 
# should be able to run as a python script with arguments from Terminal. 

# python download_file.py -fileout -list_file -

#####################################################################################
# code from http://stackoverflow.com/questions/20892133/storing-string-from-arduino-to-text-file-using-python
## import the serial library
import serial

## Boolean variable that will represent 
## whether or not the arduino is connected
connected = False
text_file = "log_WSP_ID.TXT"
filename = "log.TXT"
## establish connection to the serial port that your arduino 
## is connected to.

locations=['/dev/ttyUSB0','/dev/ttyUSB1','/dev/ttyUSB2','/dev/ttyUSB3']

for device in locations:
    try:
        print "Trying...",device
        ser = serial.Serial(device, 115200)
        break
    except:
        print "Failed to connect on",device

## loop until the arduino tells us it is ready
while not connected:
    serin = ser.read()
    connected = True

## open text file to store the current 
##gps co-ordinates received from the rover    
text_file = open(text_file, 'w')
## read serial data from arduino and 
## write it to the text file 'position.txt'
while ser.read():
    x=ser.read()
    print(x) 
    if x == "Filename to download?":
        ser.write(filename)
        ser.read()
    if x == "Ready to upload":
        while ser.read() is not "done":
            tmp = ser.read()
            if tmp == "Finish uploading file":
                break
            text_file.write(tmp)

## close the serial connection and text file
text_file.close()
ser.close()