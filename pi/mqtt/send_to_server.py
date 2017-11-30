# Standard Library
import logging
import signal
import sys

import requests

from common import MQ

class Consumer(MQ):

    name = 'send_to_server'
    sub_to = 'frames'

    def _on_message(self, body):
        # Send to server
        headers = {'Authorization': 'Token b14609d9aa6cb0a129885f9a7088abcb700e16ee'}
        requests.post(
            'http://hycamp.org/wsn/api/create/', json=body, headers=headers,
        )

if __name__ == '__main__':
    # Configure logger
    log_format = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    log_level = logging.INFO
    logging.basicConfig(format=log_format, level=log_level, stream=sys.stdout)

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
