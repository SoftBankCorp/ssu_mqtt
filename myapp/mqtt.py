# myapp/mqtt.py

import paho.mqtt.client as mqtt
from .models import SensorData
import json

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    # 클라이언트1에서 발행하는 주제를 구독
    client.subscribe("BMS/volt")
    client.subscribe("BMS/current")
    client.subscribe("BMS/temp")
    client.subscribe("BMS/humi")

def on_message(client, userdata, msg):
    print(f"Message received from {msg.topic}: {msg.payload.decode()}")
    payload = float(msg.payload.decode())  # 수신된 메시지를 float으로 변환

    if msg.topic == "BMS/volt":
        voltage = payload
        SensorData.objects.create(voltage=voltage)
    elif msg.topic == "BMS/current":
        current = payload
        SensorData.objects.create(current=current)
    elif msg.topic == "BMS/temp":
        temperature = payload
        SensorData.objects.create(temperature=temperature)
    elif msg.topic == "BMS/humi":
        humidity = payload
        SensorData.objects.create(humidity=humidity)

def mqtt_subscribe():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect("broker.mqtt-dashboard.com", 1883, 60)
    client.loop_start()
