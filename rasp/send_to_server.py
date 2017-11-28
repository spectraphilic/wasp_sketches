import json
import logging
import signal
import sqlite3
import sys

import paho.mqtt.client as mqtt
import requests

from .common import MQ

class Consumer(MQ):

    name = 'send_to_server'

    def on_connect(self, client, userdata, flags, rc):
        self.info('Connected to MQTT server')
        # Subscribe
        topic = 'frames'
        result, mid = client.subscribe(topic, qos=2)
        if result == mqtt.MQTT_ERR_SUCCESS:
            self.info('Subscribe to "%s", mid=%s', topic, mid)
        else:
            error = mqtt.error_string(result)
            self.error('Subscribe to "%s", mid=%s: %s', topic, mid, error)

    def on_message(self, client, userdata, msg):
        frame = json.loads(msg.payload)
        self.info('FRAME %s', frame)

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

if __name__ == '__main__':
    # Configure logger
    log_format = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    log_level = logging.INFO
    logging.basicConfig(format=log_format, level=log_level, stream=sys.stdout)

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
        consumer = Consumer()
        consumer.connect()
        signal.signal(signal.SIGTERM, consumer.stop) # Stop gracefully
        consumer.start()
