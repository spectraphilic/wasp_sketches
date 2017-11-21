import base64
import json
import logging
import signal
import sys
import time

import paho.mqtt.client as mqtt
from serial import Serial
from xbee import XBee


logger = logging.getLogger('read_from_xbee')

stop = False

def on_connect(client, userdata, flags, rc):
    logger.info('Connected to MQTT server')

def on_disconnect(client, userdata, rc):
    logger.info('Disconnected from MQTT server')

def on_publish(client, userdata, mid):
    logger.info('Message %s published', mid)


def sigterm(signum, frame):
    logger.info('SIGTERM received')
    global stop
    stop = True

def callback(frame):
    t0 = time.time()
    logger.debug('FRAME %s', frame)
    if frame['id'] != 'rx':
        logger.warning('UNEXPECTED ID %s', frame['id'])
        return

    for k in frame.keys():
        if k != 'id':
            v = frame[k]
            frame[k] = base64.b64encode(v)

    # Publish
    topic = 'frames'
    result, mid = client.publish(topic, json.dumps(frame), qos=2)
    if result == mqtt.MQTT_ERR_SUCCESS:
        t = time.time() - t0
        logger.debug('Publish to "%s", mid=%s time=%f', topic, mid, t)
    else:
        error = mqtt.error_string(result)
        logger.error('Publish to "%s", mid=%s: %s', topic, mid, error)

if __name__ == '__main__':
    # Configure logger
    logger.setLevel(logging.DEBUG)
    ch = logging.StreamHandler(sys.stdout)
    ch.setLevel(logging.DEBUG)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    logger.addHandler(ch)

    # Connect to MQTT server
    client = mqtt.Client(client_id='read_from_xbee')
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.connect('localhost')
    client.loop_start()

    # Stop gracefully
    signal.signal(signal.SIGTERM, sigterm)

    # {'source_addr_long': '\x00\x13\xa2\x00Aj\x07#', 'rf_data': "<=>\x06\x1eb'g\x05|\x10T\x13#\xc3{\xa8\n\xf3Y4b\xc8\x00\x00PA33\xabA\x00\x00\x00\x00", 'source_addr': '\xff\xfe', 'id': 'rx', 'options': '\xc2'}
    with Serial('/dev/serial0', 9600) as serial:
        xbee = XBee(serial, callback=callback)
        while not stop:
            try:
                time.sleep(0.001)
            except KeyboardInterrupt:
                break

        xbee.halt()

    client.loop_stop()
    client.disconnect()
