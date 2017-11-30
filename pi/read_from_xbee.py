# Standard Library
import argparse
import base64
import logging
import signal
import sys
import time

from serial import Serial
from xbee import XBee

from common import MQ


class Publisher(MQ):
    name = 'read_from_xbee'

    def pub_frame(self, frame):
        # {'source_addr_long': '\x00\x13\xa2\x00Aj\x07#', 'rf_data': "<=>\x06\x1eb'g\x05|\x10T\x13#\xc3{\xa8\n\xf3Y4b\xc8\x00\x00PA33\xabA\x00\x00\x00\x00", 'source_addr': '\xff\xfe', 'id': 'rx', 'options': '\xc2'}
        t0 = time.time()
        self.info('FRAME %s', frame)
        if frame['id'] != 'rx':
            self.warning('UNEXPECTED ID %s', frame['id'])
            return

        for k in frame.keys():
            if k != 'id':
                v = frame[k]
                frame[k] = base64.b64encode(v)

        # Add timestamp
        frame['received'] = int(t0)

        # Publish
        self.publish(frame)
        self.debug('Message sent in %f seconds', time.time() - t0)


stop = False
def sigterm(signum, frame):
    global stop
    stop = True

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('bauds', nargs='?', type=int, default=9600)
    args = parser.parse_args()
    bauds = args.bauds

    # Configure logger
    log_format = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    log_level = logging.INFO
    logging.basicConfig(format=log_format, level=log_level, stream=sys.stdout)

    # Connect to broker
    publisher = Publisher()
    publisher.connect()
    #publisher.start()

    with Serial('/dev/serial0', bauds) as serial:
        xbee = XBee(serial, callback=publisher.pub_frame) # Start XBee thread
        signal.signal(signal.SIGTERM, sigterm) # Signal sent by Supervisor
        while not stop:
            try:
                time.sleep(0.01)
            except KeyboardInterrupt:
                break

        xbee.halt() # Stop XBee thread

    # End
    publisher.stop()
    logging.shutdown()
