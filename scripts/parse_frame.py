'''
Script to parse frames from waspmote

Simon Filhol
'''

from __future__ import print_function, unicode_literals

import numpy as np
import pandas as pd
#import matplotlib as mpl
#mpl.use('PS')
import matplotlib.pyplot as plt
import struct

USHORT = 0 # uint8_t
INT    = 1 # int16_t
FLOAT  = 2 # double
STR    = 3 # char*
ULONG  = 4 # uint32_t

SENSORS = {
#    15: (b'PA', ),
     33: (b'TCB', FLOAT, 1),
     35: (b'HUMB', FLOAT, 1),
#    38: (b'LW', ),
     52: (b'BAT', USHORT, 1),
#    53: (b'GPS', ),
#    54: (b'RSSI', ),
#    55: (b'MAC', ),
     62: (b'IN_TEMP', FLOAT, 1),
#    63: (b'ACC', ),
#    74: (b'BME_TC', ),
#    76: (b'BME_HUM', ),
#    77: (b'BME_PRES', ),
#    85: (b'TX_PWR', ),
#    89: (b'SPEED_OG', ),
#    90: (b'COURSE_OG', ),
#    91: (b'ALT', ),
    123: (b'TST', ULONG, 1),
#   200: (b'SDI12_CTD10', ),
#   201: (b'SDI12_DS2_1', ),
#   202: (b'SDI12_DS2_2', ),
#   203: (b'DS1820', ),
}

SENSORS_STR = {v[0]: v for k, v in SENSORS.items()}

class frameObj(object):
    def __init__(self):
        self.serialID = np.nan
        self.frameID = np.nan
        self.tst = np.nan
        self.bat = np.nan
        self.tcb = np.nan
        self.in_temp = np.nan
        self.humb = np.nan

def parse_frame(src):
    line = src

    # Start delimiter
    if line[:3] != b'<=>':
        print("Warning: not a frame")
        print(src)
        return None
    line = line[3:]

    # Frame type
    frame_type = struct.unpack_from("B", line)[0]
    line = line[1:]

    # Number of fields (ASCII) or bytes (Binary)
    n = struct.unpack_from("B", line)[0]
    line = line[1:]

    frame = frameObj()

    # ASCII
    if frame_type & 0x80:
        spline = line.split(b'#')
        serial_id = spline[1]
#       waspmote_id = spline[2]
        sequence = spline[3]
        payload = spline[4:-1]

        if len(payload) != n:
            print("Warning: number of fields does not match %d != %d" % (len(payload), n))
            print(src)
            return None

#       print('Serial ', serial_id)
#       print('Wasp   ', waspmote_id)
#       print('Seq    ', sequence)
#       print('Payload', payload)

        frame.serialID = int(serial_id)
        frame.frameID = int(sequence)

        # Payload
        for field in payload:
            key, value = field.split(b':')
            sensor = SENSORS_STR.get(key, ())
            if not sensor:
                print("Warning: %s sensor type not supported" % key)
                print(src)
                return None

            key, sensor_type, n = sensor
            if sensor_type in (USHORT, INT, ULONG):
                value = [int(x) for x in value.split(b';')]
            elif sensor_type == FLOAT:
                value = [float(x) for x in value.split(b';')]
            elif sensor_type == STR:
                assert n == 1
                value = [value]

            assert len(value) == n
            if len(value) == 1:
                value = value[0]

            key = key.decode('ascii').lower()
            setattr(frame, key, value)

    # Binary
    else:
        if frame_type == 0x00:
            v15 = False
        elif frame_type == 0x06:
            v15 = True
        else:
            print("Warning: %d frame type not supported" % frame_type)
            print(src)
            return None

        # Serial id
        if v15:
            serial_id = struct.unpack_from("Q", line)
            line = line[8:]
        else:
            serial_id = struct.unpack_from("L", line)
            line = line[4:]

        waspmote_id, line = line.split(b'#', 1)
        sequence = struct.unpack_from("B", line)
        payload = line[1:]

        while line:
            sensor_id = struct.unpack_from("B", line)
            line = line[1:]
            sensor = SENSORS.get(sensor_id, ())
            if not sensor:
                print("Warning: %d sensor type not supported" % sensor_id)
                print(src)
                return None

            key, sensor_type, n = sensor
            for i in range(0, n):
                if sensor_type == USHORT:
                    value = struct.unpack_from("B", line)
                    line = line[1:]
                elif sensor_type == INT:
                    value = struct.unpack_from("h", line)
                    line = line[2:]
                elif sensor_type == FLOAT:
                    value = struct.unpack_from("f", line)
                    line = line[4:]
                elif sensor_type == ULONG:
                    value = struct.unpack_from("L", line)
                    line = line[4:]
                elif sensor_type == STR:
                    length = struct.unpack_from("B", line)
                    line = line[1:]
                    value = line[:length]
                    line = line[length:]

    return frame


def read_wasp_data(filename):
    f = open(filename, 'rb')
    data = pd.DataFrame()
    for line in f:
        frame = parse_frame(line)
        if frame is not None:
            data = data.append(frame.__dict__, ignore_index=True)

    return data

def plot(filename):
    data = read_wasp_data(filename)
    data.sort_values(by='tst', inplace=True)
    data['timestamp'] = pd.to_datetime(data['tst'], unit='s')
    data = data.set_index('timestamp')
    data.bat = data['bat'].astype(float)
    data.in_temp = data['in_temp'].astype(float)
    return data


if __name__ == '__main__':
    filenames = [
#       '../../data/data_20170710/TMP.TXT',
#       '../../data/data_20170710/DATA/170706.TXT',
        'data/170914.TXT'
    ]
    datas = [plot(x) for x in filenames]

    #plt.ion()

    # Battery
    plt.subplot(2,1,1)
    for data in datas:
        data.bat.dropna().plot()
    plt.ylabel('Battery level (%)')

    # Internal Temperture
    plt.subplot(2,1,2)
    for data in datas:
        data.in_temp.dropna().plot()
    plt.ylabel('Internal Temperature (degC)')

    # Plot
    plt.plot()
    plt.show()
