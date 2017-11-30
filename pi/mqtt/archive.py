# Standard Library
import base64
from datetime import date
import logging
import os
import signal
import struct
import sys

from common import MQ

class Consumer(MQ):

    name = 'archive'
    sub_to = 'frames'

    def _on_message(self, body):
        # Create parent directory
        mac = base64.b64decode(body['source_addr_long'])
        mac = '%016X' % struct.unpack(">Q", mac)
        dirpath = os.join(datadir, mac)
        try:
            os.makedirs(dirpath) # XXX Use exist_ok with Python 3
        except OSError:
            pass

        # Append frame to archive file
        received = date.fromtimestamp(body['received']).strftime('%Y%m%d')
        filepath = os.join(dirpath, received)
        with open(filepath, 'a+') as f:
            f.write(body + '\n')


if __name__ == '__main__':
    # Configure logger
    log_format = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    log_level = logging.INFO
    logging.basicConfig(format=log_format, level=log_level, stream=sys.stdout)

    # Data dir
    datadir = os.path.join(os.getcwd(), 'data')

    # Main
    consumer = Consumer()
    consumer.connect()
    signal.signal(signal.SIGTERM, consumer.stop)
    try:
        consumer.start()
    except KeyboardInterrupt:
        consumer.stop()

    # End
    logging.shutdown()
