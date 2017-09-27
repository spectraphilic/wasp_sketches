'''
Script to parse frames from waspmote

Simon Filhol
'''

from __future__ import print_function, unicode_literals
import os
import struct

import numpy as np
import pandas as pd
#import matplotlib as mpl
#mpl.use('PS')
import matplotlib.pyplot as plt


USHORT = 0 # uint8_t
INT    = 1 # int16_t
FLOAT  = 2 # double
STR    = 3 # char*
ULONG  = 4 # uint32_t

SENSORS = {
     15: (b'PA', FLOAT, 1),
     33: (b'TCB', FLOAT, 1),
     35: (b'HUMB', FLOAT, 1),
     38: (b'LW', FLOAT, 1),
     52: (b'BAT', USHORT, 1),
#    53: (b'GPS', ),
     54: (b'RSSI', INT, 1),
     55: (b'MAC', STR, 1),
     62: (b'IN_TEMP', FLOAT, 1),
#    63: (b'ACC', ),
     65: (b'STR', STR, 1),
     74: (b'BME_TC', FLOAT, 1),
     76: (b'BME_HUM', FLOAT, 1),
     77: (b'BME_PRES', FLOAT, 1),
     85: (b'TX_PWR', USHORT, 1),
#    89: (b'SPEED_OG', ),
#    90: (b'COURSE_OG', ),
#    91: (b'ALT', ),
    123: (b'TST', ULONG, 1),
    200: (b'SDI12_CTD10', FLOAT, 3),
    201: (b'SDI12_DS2_1', FLOAT, 3),
    202: (b'SDI12_DS2_2', FLOAT, 3),
    203: (b'DS1820', FLOAT, 1),
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


def search_frame(data):
    """
    Search the frame start delimiter <=> and return the data just after the
    delimter.  Anything found before is considered garbage and discarded. If
    the delimeter is not found return None.
    """
    index = data.find(b'<=>')
    if index == -1:
        return None

    if index > 0:
        print('Warning: garbage before frame found and discarded')

    return data[index+3:]


def parse_frame(line):
    """
    Parse the frame starting at the given byte string. We consider that the
    frame start delimeter has already been read.
    """
    # Frame type
    frame_type = struct.unpack_from("B", line)[0]
    line = line[1:]

    # Number of fields (ASCII) or bytes (Binary)
    n = struct.unpack_from("B", line)[0]
    line = line[1:]

    frame = frameObj()

    # ASCII
    # <=><80>^C#408518488##0#TST:1499340600#BAT:98#IN_TEMP:31.00#
    if frame_type & 0x80:
        spline = line.split(b'#', n + 4)
        serial_id = spline[1]
#       waspmote_id = spline[2]
        sequence = spline[3]
        payload = spline[4:4+n]
        rest = spline[4+n]

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
        rest = line[n:]
        line = line[:n]
        if frame_type == 0x00:
            v15 = False
        elif frame_type == 0x06:
            v15 = True
        else:
            print("Warning: %d frame type not supported" % frame_type)
            return None

        # Serial id
        if v15:
            serial_id = struct.unpack_from("Q", line)[0]
            line = line[8:]
        else:
            serial_id = struct.unpack_from("I", line)[0]
            line = line[4:]

        waspmote_id, line = line.split(b'#', 1)
        sequence = struct.unpack_from("B", line)[0]
        line = line[1:] # Payload

        frame.serialID = serial_id
        frame.frameID = sequence

        while line:
            sensor_id = struct.unpack_from("B", line)[0]
            line = line[1:]
            sensor = SENSORS.get(sensor_id, ())
            if not sensor:
                print("Warning: %d sensor type not supported" % sensor_id)
                return None

            key, sensor_type, nvalues = sensor
            values = []
            for i in range(0, nvalues):
                if sensor_type == USHORT:
                    value = struct.unpack_from("B", line)[0]
                    line = line[1:]
                elif sensor_type == INT:
                    value = struct.unpack_from("h", line)[0]
                    line = line[2:]
                elif sensor_type == FLOAT:
                    value = struct.unpack_from("f", line)[0]
                    line = line[4:]
                elif sensor_type == ULONG:
                    value = struct.unpack_from("I", line)[0]
                    line = line[4:]
                elif sensor_type == STR:
                    length = struct.unpack_from("B", line)[0]
                    line = line[1:]
                    value = line[:length]
                    line = line[length:]

                values.append(value)

            if len(values) == 1:
                values = values[0]

            key = key.decode('ascii').lower()
            setattr(frame, key, values)

    return frame, rest


def read_wasp_data(filename, data):
    src = open(filename, 'rb').read()
    while src:
        src = search_frame(src)
        if src:
            frame, src = parse_frame(src)
            if frame is not None:
                data.append(frame.__dict__)

            # read end of frame: \n
            if src and src[0] == '\n':
                src = src[1:]


if __name__ == '__main__':
    names = [
#       '../../data/data_20170710/TMP.TXT',
#       '../../data/data_20170710/DATA/170706.TXT',
#       'data/170926/DATA',
        'data/170924-finse/DATA/170921.TXT',
        'data/170924-finse/DATA/170922.TXT',
        'data/170924-finse/DATA/170923.TXT',
        'data/170924-finse/DATA/170924.TXT',
    ]

    data = []
    for name in names:
        if os.path.isdir(name):
            for filename in os.listdir(name):
                filename = os.path.join(name, filename)
                read_wasp_data(filename, data)
        else:
            read_wasp_data(name, data)
    data_frame = pd.DataFrame(data)

    data_frame.sort_values(by='tst', inplace=True)
    data_frame['timestamp'] = pd.to_datetime(data_frame['tst'], unit='s')
    data_frame = data_frame.set_index('timestamp')

    #plt.ion()

    # Battery
    plt.subplot(2,1,1)
    data_frame.bat.dropna().plot()
    plt.ylabel('Battery level (%)')

    # Internal Temperture
    plt.subplot(2,1,2)
    data_frame.in_temp.dropna().plot()
    #data_frame.tcb.dropna().plot()
    plt.ylabel('Internal Temperature (degC)')

    # Plot
    plt.plot()
    plt.show()
