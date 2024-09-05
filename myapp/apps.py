from django.apps import AppConfig

from django.apps import AppConfig

class MyAppConfig(AppConfig):
    name = 'myapp'

    def ready(self):
        from . import mqtt
        mqtt.mqtt_subscribe()
