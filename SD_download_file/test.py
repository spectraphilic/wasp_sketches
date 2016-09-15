import serial


try:
    print "Trying..."
    ser = serial.Serial('/dev/ttyUSB0', 115200)
except:
    print "Failed to connect on " + ser.portstr
print "Connected to " + ser.portstr


print "start reading line"

c=ser.readline()
print c

