from django.db import models

class SensorData(models.Model):
    current = models.FloatField(null=True, blank=True)
    voltage = models.FloatField(null=True, blank=True)
    temperature = models.FloatField(null=True, blank=True)
    humidity = models.FloatField(null=True, blank=True)
    timestamp = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f"Current: {self.current}, Voltage: {self.voltage}, Temp: {self.temperature}, Humidity: {self.humidity}"

class MotorControl(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True)
    power = models.IntegerField()  # 0: OFF, 1: ON
    speed = models.FloatField()     # 모터 속도

    def __str__(self):
        return f"Motor Control at {self.timestamp} - Power: {'ON' if self.power else 'OFF'}, Speed: {self.speed}"
