# Standard Library
import json
import logging

import pika


class MQ(object):

    name = ''
    host = 'localhost'
    exchange = 'motes'
    queue = None

    def __init__(self):
        self.connection = None
        self.channel = None
        self.logger = logging.getLogger(self.name)
        self.started = False

    def connect(self, parameters):
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

    def on_exchange_declare(self):
        self.debug('Exchange declared.')
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
        self.debug('Queue bound to exchange.')
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
        self.debug('Message received.')
        self.debug('%s %s %s %s', type(channel), type(method), type(header), type(body))
        self.debug(body)
        channel.basic_ack(delivery_tag=method.delivery_tag)

    #
    # Logging helpers
    #
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
