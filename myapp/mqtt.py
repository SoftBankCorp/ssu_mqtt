# myapp/mqtt.py

import paho.mqtt.client as mqtt
from .models import AverageSensorData, RawSensorData
import struct
import json

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    # 클라이언트1에서 발행하는 주제를 구독
    client.subscribe("BMS/raw")
    client.subscribe("BMS/average")
    client.subscribe("BMS/power")


    client.subscribe("BMS/volt")
    client.subscribe("BMS/current")
    client.subscribe("BMS/temp")
    client.subscribe("BMS/humi")
    ####


def on_message(client, userdata, msg):
    print(f"Message received on topic {msg.topic}")
    # payload = float(msg.payload.decode())  # 수신된 메시지를 float으로 변환

    # if msg.topic == "BMS/volt":
    #     voltage = payload
    #     SensorData.objects.create(voltage=voltage)
    # if msg.topic == "BMS/current":
    #     current = payload
    #     SensorData.objects.create(current=current)
    # if msg.topic == "BMS/temp":
    #     temperature = payload
    #     SensorData.objects.create(temperature=temperature)
    # if msg.topic == "BMS/humi":
    #     humidity = payload
    #     SensorData.objects.create(humidity=humidity)
    # ##
        # 메시지 페이로드를 해석하는 부분
    if msg.topic == "BMS/raw":
        process_raw_data(msg.payload)
    if msg.topic == "BMS/average":
        process_average_data(msg.payload)


# raw 데이터를 처리하는 함수
def process_raw_data(payload):
    # 4개의 float 값을 수신하는 패킷으로 가정 (온도, 습도, 전류, 전압)
    temp, hum, current, voltage = struct.unpack('ffff', payload)

    # SensorData 모델에 데이터를 저장
    RawSensorData.objects.create(
        temperature=temp,
        humidity=hum,
        current=current,
        voltage=voltage
    )
    print(f"Raw Data - Temp: {temp}, Humidity: {hum}, Current: {current}, Voltage: {voltage}")


# 평균 데이터를 처리하는 함수
def process_average_data(payload):
    # 평균 데이터 처리 (온도, 습도, 전류, 전압)
    temp_avg, hum_avg, current_avg, voltage_avg = struct.unpack('ffff', payload)

    # SensorData 모델에 데이터를 저장 (평균 값 저장)
    AverageSensorData.objects.create(
        temperature=temp_avg,
        humidity=hum_avg,
        current=current_avg,
        voltage=voltage_avg
    )
    print(f"Average Data - Temp: {temp_avg}, Humidity: {hum_avg}, Current: {current_avg}, Voltage: {voltage_avg}")


def mqtt_subscribe():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect("broker.mqtt-dashboard.com", 1883, 60)
    client.loop_start()
