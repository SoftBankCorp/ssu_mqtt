from django.urls import path
from .consumers import SensorDataConsumer

websocket_urlpatterns = [
    path('ws/sensordata/', SensorDataConsumer.as_asgi()),
]
