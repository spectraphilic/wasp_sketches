from serial import Serial
from xbee import XBee
from paho.mqtt.client import Client


def on_connect(client, userdata, flags, rc):
    print('Connected %s' % rc)

if __name__ == '__main__':
    client = Client()
    client.on_connect = on_connect
    client.connect('localhost')
    client.loop_start()

    with Serial('/dev/serial0', 9600) as serial:
        xbee = XBee(serial)
        while True:
            try:
                frame = xbee.wait_read_frame()
                print(frame)
                client.publish('frames', frame['rf_data'])
            except KeyboardInterrupt:
                break

    client.loop_stop()
    client.disconnect()
