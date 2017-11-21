import base64
import json
import logging
import signal
import sqlite3
import sys

import paho.mqtt.client as mqtt
import requests

logger = logging.getLogger('send_to_server')

def on_connect(client, userdata, flags, rc):
    logger.info('Connected to MQTT server')
    # Subscribe
    topic = 'frames'
    result, mid = client.subscribe(topic, qos=2)
    if result == mqtt.MQTT_ERR_SUCCESS:
        logger.info('Subscribe to "%s", mid=%s', topic, mid)
    else:
        error = mqtt.error_string(result)
        logger.error('Subscribe to "%s", mid=%s: %s', topic, mid, error)

def on_subscribe(client, userdata, mid, granted_qos):
    logger.info('Subscription mid=%s qos=%s', mid, granted_qos)

def on_message(client, userdata, msg):
    cursor = userdata
    frame = json.loads(msg.payload)
    logger.info('FRAME %s', frame)

    # Archive to SQL
    cursor.execute(
        """INSERT INTO frames VALUES (?, ?, ?, ?, ?, ?)""",
        (
            0,
            unicode(frame['id']),
            sqlite3.Binary(frame['rf_data']),
            sqlite3.Binary(frame['source_addr']),
            sqlite3.Binary(frame['source_addr_long']),
            sqlite3.Binary(frame['options']),
        )
    )
    conn.commit()

    # Send to server
    headers = {'Authorization': 'Token b14609d9aa6cb0a129885f9a7088abcb700e16ee'}
    requests.post(
        'http://hycamp.org/wsn/api/create/', json=frame, headers=headers,
    )

def sigterm(signum, frame):
    logger.info('SIGTERM received')
    client.disconnect()

def on_log(client, userdata, level, buf):
    f = {
        mqtt.MQTT_LOG_INFO: logger.info,
        mqtt.MQTT_LOG_NOTICE: logger.info, # XXX
        mqtt.MQTT_LOG_WARNING: logger.warning,
        mqtt.MQTT_LOG_ERR: logger.error,
        mqtt.MQTT_LOG_DEBUG: logger.debug,
    }.get(level)
    if f is None:
        logger.error('on_log unexepected log level %s', level)
        f = logger.info
    f(buf)


if __name__ == '__main__':
    # Configure logger
    level = logging.INFO
    logger.setLevel(level)
    ch = logging.StreamHandler(sys.stdout)
    ch.setLevel(level)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    logger.addHandler(ch)

    conn = sqlite3.connect('frames.db')
    with conn:
        cursor = conn.cursor()
        # Create table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS frames
            (sent integer, id text, rf_data blob, source_addr blob, source_addr_long blob, options blob)
            """)
        conn.commit()

        # Connect to MQTT server
        client = mqtt.Client(
            client_id='send_to_server', clean_session=False,
            userdata=cursor
        )
        client.on_connect = on_connect
        client.on_subscribe = on_subscribe
        client.on_message = on_message
        client.on_log = on_log
        client.connect('localhost')

        # Stop gracefully
        signal.signal(signal.SIGTERM, sigterm)

        client.loop_forever()
