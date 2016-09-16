#!/usr/bin/python

#Script to download a file from Waspmote SD through USB serial.

# USAGE:
# python download_file.py -o fileout -i inputfile
# python download_file.py -o fileout
# python download_file.py
# python download_file.py -i inputfile

#####################################################################################
## import the serial library
import serial
import sys, getopt
# Store input and output file names
inputfile =  None
outputfile = None
 
# Read command line args
myopts, args = getopt.getopt(sys.argv[1:],"i:o:")
 
###############################
# o == option
# a == argument passed to the o
###############################
for o, a in myopts:
    if o == '-i':
        inputfile=a
    elif o == '-o':
        outputfile=a
    else:
        print("Usage: %s -i input -o output" % sys.argv[0])
 
# Display input and output file name passed as the args
print ("Input file : %s and output file: %s" % (inputfile,outputfile) )

if outputfile is None:
    text_file = "downloaded_data.txt"
elif inputfile is None:
    filename = None
else:
    text_file = outputfile
    filename = inputfile

## Boolean variable that will represent 
## whether or not the arduino is connected
connected = False
locations=['/dev/ttyUSB0','/dev/ttyUSB1','/dev/ttyUSB2','/dev/ttyUSB3']
## establish connection to the serial port that your arduino 
## is connected to.
for device in locations:
    try:
        print "Trying...",device
        ser = serial.Serial(device, 115200)
        print "Connected to " + ser.portstr
        break
    except:
        print "Failed to connect on",device

# ## loop until the arduino tells us it is ready
# while not connected:
#     readLine()
#     connected = True

## open text file to store the current 
##gps co-ordinates received from the rover    
dump = open(text_file, 'w')

## read serial data from arduino and 
## write it to the text file 'position.txt'
while True:
    x = ser.readline()
    print(x)
    if x[0:5] == "Filen":
        if filename is None:
            filename = raw_input()
        ser.write(filename)
        ser.flush()
        print("filename flushed...")

        while True:
            y = ser.readline()
            print(y)
            if y[0:5] == "Ready":
                print("yoohoo")
                while True:
                    tmp = ser.readline()
                    if tmp[0:6] == "Finish":
                        print("yoohoo end")
                        dump.close()
                        ser.close()
                        print("Serial port closed")
                        print("+++ Download of " + filename + " done! +++ ")
                        break
                    else:
                        dump.write(tmp)
                        print("writing...")
                else:
                    continue
                break
        else:
            continue
        break

## close the serial connection and text file





