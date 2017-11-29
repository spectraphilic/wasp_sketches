# Standard Library
import json
import logging

import paho.mqtt.client as mqtt


class MQ(object):

    name = ''
    host = 'localhost'
    threaded = False
    sub_to = ''
    pub_to = ''

    def __init__(self):
        self.client = None
        self.logger = logging.getLogger(self.name)

    def on_connect(self, client, userdata, flags, rc):
        self.info('Connected to MQTT server')
        # Subscribe
        topic = self.sub_to
        if topic:
            result, mid = client.subscribe(topic, qos=2)
            if result == mqtt.MQTT_ERR_SUCCESS:
                self.info('Subscribe to "%s", mid=%s', topic, mid)
            else:
                error = mqtt.error_string(result)
                self.error('Subscribe to "%s", mid=%s: %s', topic, mid, error)

    def on_disconnect(self, client, userdata, rc):
        self.info('Disconnected from MQTT server')

    def on_publish(self, client, userdata, mid):
        self.info('Message published mid=%s', mid)

    def on_subscribe(self, client, userdata, mid, granted_qos):
        self.info('Subscription mid=%s qos=%s', mid, granted_qos)

    def on_message(self, client, userdata, msg):
        self.info('Message received')
        try:
            body = json.loads(msg.payload)
            self._on_message(body)
        except Exception:
            self.exception('Message handling failed')
        else:
            self.info('Message done')

    def _on_message(self, body):
        pass

    def on_log(self, client, userdata, level, buf):
        f = {
            mqtt.MQTT_LOG_INFO: self.info,
            mqtt.MQTT_LOG_NOTICE: self.info, # XXX
            mqtt.MQTT_LOG_WARNING: self.warning,
            mqtt.MQTT_LOG_ERR: self.error,
            mqtt.MQTT_LOG_DEBUG: self.debug,
        }.get(level)
        if f is None:
            self.error('on_log unexepected log level %s', level)
            f = self.info
        f(buf)

    def connect(self):
        self.client = mqtt.Client(client_id=self.name, clean_session=False)
        client = self.client
        client.on_connect = self.on_connect
        client.on_disconnect = self.on_disconnect
        client.on_log = self.on_log
        client.on_subscribe = self.on_subscribe
        client.on_message = self.on_message
        client.connect(self.host)

    def start(self):
        if self.threaded:
            self.client.loop_start() # Start MQTT thread
        else:
            self.client.loop_forever()

    def stop(self):
        self.info('Stop, close connection.')
        if self.threaded:
            self.client.loop_stop() # Stop MQTT thread
        self.client.disconnect()

    #
    # Publisher
    #
    def publish(self, body):
        body = json.dumps(body)
        topic = self.pub_to
        if topic:
            result, mid = self.client.publish(topic, body, qos=2)
            if result == mqtt.MQTT_ERR_SUCCESS:
                self.info('Publish to "%s", mid=%s', topic, mid)
            else:
                error = mqtt.error_string(result)
                self.error('Publish to "%s", mid=%s: %s', topic, mid, error)

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
