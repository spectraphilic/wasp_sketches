# Standard Library
from configparser import RawConfigParser as ConfigParser
import json
import logging
import sys

import pika


class MQ(object):
    """
    This class is to be used as a base class. Its main purpose is to provide
    access to the message broker, either as publisher or consumer.
    Additionnaly, it wraps config and logging.

    It is meant to be used as a context manager:

        class Publisher(MQ):
            name = 'xxx'

        with Publisher() as publisher:
            ...
    """

    name = ''
    host = 'localhost'
    exchange = 'motes'
    queue = None

    def __init__(self):
        self.connection = None
        self.channel = None
        self.logger = logging.getLogger(self.name)
        self.started = False
        # Read configuration
        config = ConfigParser()
        config.read('config.ini')
        self.config = config[self.name]

    def connect(self):
        self.debug('Connecting to broker at "%s" ...', self.host)
        parameters = pika.ConnectionParameters(host=self.host)
        self.connection = pika.SelectConnection(parameters, self.on_connected)

    def start(self):
        self.info('Loop starting... To exit press CTRL+C')
        self.started = True
        self.connection.ioloop.start()

    def stop(self):
        self.info('Loop stopping...')
        self.connection.close()
        if self.started:
            self.connection.ioloop.start() # Graceful stop
        self.info('Loop stopped.')

    def on_connected(self, connection):
        self.debug('Connected to broker. Opening channel...')
        connection.channel(self.on_channel_open)

    def on_channel_open(self, channel):
        self.debug('Channel open. Declaring exchange "%s" ...', self.exchange)
        self.channel = channel
        channel.exchange_declare(
            self.on_exchange_declare,
            exchange=self.exchange,
            exchange_type='fanout',
            durable=True,
        )

    def on_exchange_declare(self, unused_frame):
        self.debug('Exchange declared')
        if self.queue is not None:
            self.debug('Declaring queue "%s"...', self.queue)
            self.channel.queue_declare(
                self.on_queue_declare,
                self.queue,
                durable=True,
            )

    def on_queue_declare(self, frame):
        self.debug(
            'Queue declared. Binding queue "%s" to exchange "%s" ...',
            self.queue,
            self.exchange
        )
        self.channel.queue_bind(self.on_queue_bind, self.queue, self.exchange)#, self.routing_key)

    def on_queue_bind(self, frame):
        self.debug('Queue bound to exchange')
        self.channel.basic_consume(self.on_message, queue=self.queue)

    #
    # Publisher
    #
    def publish(self, body):
        self.debug('Publishing...')
        body = json.dumps(body)
        properties = pika.BasicProperties(
            delivery_mode=2, # persistent message
            content_type='application/json',
        )
        self.channel.basic_publish(
            exchange=self.exchange, #routing_key=queue,
            properties=properties,
            body=body,
        )

    #
    # Consumer
    #
    def on_message(self, channel, method, header, body):
        self.info('Message received')
        try:
            body = json.loads(body)
            self._on_message(body)
        except Exception:
            self.exception('Message handling failed')
            #channel.basic_reject(delivery_tag=method.delivery_tag)
        else:
            channel.basic_ack(delivery_tag=method.delivery_tag)
            self.info('Message done')

    def _on_message(self, body):
        pass

    #
    # Logging helpers
    #
    def init_logging(self):
        # Logging
        log_format = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        log_level = self.config.get('log_level', 'info').upper()
        log_level = logging.getLevelName(log_level)
        logging.basicConfig(format=log_format, level=log_level, stream=sys.stdout)

    def debug(self, *args):
        self.logger.debug(*args)

    def info(self, *args):
        self.logger.info(*args)

    def warning(self, *args):
        self.logger.warning(*args)

    def error(self, *args):
        self.logger.error(*args)

    def critical(self, *args):
        self.logger.critical(*args)


    #
    # Context manager
    #
    def __enter__(self):
        self.init_logging()
        self.connect()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.stop()
        logging.shutdown()
