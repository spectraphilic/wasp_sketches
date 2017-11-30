# Standard Library
import base64
from datetime import date
import os
import signal
import struct

from common import MQ

class Consumer(MQ):

    name = 'archive'
    queue = 'motes_archive'

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
    datadir = os.path.join(os.getcwd(), 'data')
    with Consumer() as consumer:
        signal.signal(signal.SIGTERM, consumer.stop)
        consumer.start()
