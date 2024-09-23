
import json
import paho.mqtt.publish as publish

from .models import MotorControl


class SensorDataConsumer(AsyncWebsocketConsumer):
    async def connect(self):
        """WebSocket 연결 시 호출"""
        await self.channel_layer.group_add('sensor_data_group', self.channel_name)
        await self.accept()

    async def disconnect(self, close_code):
        """WebSocket 연결 종료 시 호출"""
        await self.channel_layer.group_discard('sensor_data_group', self.channel_name)

    async def receive(self, text_data):
        """클라이언트로부터 메시지 수신 시 호출"""
        data = json.loads(text_data)

        if 'motor_status' in data:
            # 모터 ON/OFF 제어 명령 처리
            publish.single('motor/control', json.dumps({
                'status': data['motor_status']
            }), hostname='mqtt_broker_address')
            # 데이터베이스에 기록
            MotorControl.objects.create(status=data['motor_status'])

        if 'motor_speed' in data:
            # 모터 속도 제어 명령 처리
            publish.single('motor/control', json.dumps({
                'speed': data['motor_speed']
            }), hostname='mqtt_broker_address')
            # 데이터베이스에 기록
            MotorControl.objects.create(speed=data['motor_speed'])

    async def send_sensor_data(self, event):
        """센서 데이터를 클라이언트로 전송"""
        await self.send(text_data=json.dumps(event['data']))
