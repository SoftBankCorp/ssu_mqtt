from django.db import models

class RawSensorData(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True)
    temperature = models.FloatField(default=0.0)
    humidity = models.FloatField(default=0.0)
    current = models.FloatField(default=0.0)
    voltage = models.FloatField(default=0.0)
    def __str__(self):
        return f"Raw Data - {self.timestamp} - Temp: {self.temperature}, Humidity: {self.humidity}, Current: {self.current}, Voltage: {self.voltage}"


class AverageSensorData(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True)  # 데이터 저장 시간
    temperature = models.FloatField(default=0.0) # 평균 온도 데이터
    humidity = models.FloatField(default=0.0)  # 평균 습도 데이터
    current = models.FloatField(default=0.0)  # 평균 전류 데이터
    voltage = models.FloatField(default=0.0)# 평균 전압 데이터

    def __str__(self):
        return f"Average Data - {self.timestamp} - Temp: {self.temperature}, Humidity: {self.humidity}, Current: {self.current}, Voltage: {self.voltage}"

# Motor control model
class MotorControl(models.Model):
    id = models.AutoField(primary_key=True)
    timestamp = models.DateTimeField(auto_now_add=True)
    power = models.IntegerField(default=0)  # 0: OFF, 1: ON
    speed = models.FloatField(default=0.0)  # Motor speed

    def __str__(self):
        return f"Motor Control at {self.timestamp} - Power: {self.power}, Speed: {self.speed}"

# class MotorControl(models.Model):
#     id = models.AutoField(primary_key=True)
#     timestamp = models.DateTimeField(auto_now_add=True)
#     power = models.IntegerField(default=0.0)  # 0: OFF, 1: ON
#     speed = models.FloatField(default=0.0)     # 모터 속도
#
#     def __str__(self):
#         return f"Motor Control at {self.timestamp} - Power: {'ON' if self.power else 'OFF'}, Speed: {self.speed}"