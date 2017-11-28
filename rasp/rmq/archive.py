# Standard Library
import json
import logging
import signal
import sqlite3
import sys

from common import MQ

class Consumer(MQ):

    name = 'archive_frames'
    queue = 'motes_archive'

    def on_message(self, channel, method, header, body):
        frame = json.loads(body)
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

if __name__ == '__main__':
    # Logging
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

        # Run
        consumer = Consumer()
        consumer.connect()
        signal.signal(signal.SIGTERM, consumer.stop)
        try:
            consumer.start()
        except KeyboardInterrupt:
            consumer.stop()

    # End
    logging.shutdown()
