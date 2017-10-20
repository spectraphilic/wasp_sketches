from paho.mqtt.client import Client


def on_connect(client, userdata, flags, rc):
    print('Connected %s' % rc)
    client.subscribe('frames')

def on_message(client, userdata, msg):
    print(msg.payload)

if __name__ == '__main__':
    client = Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect('localhost')
    client.loop_forever()
