import os
import django
from django.core.asgi import get_asgi_application
from channels.routing import ProtocolTypeRouter, URLRouter
from channels.auth import AuthMiddlewareStack
import myapp.routing
import logging

# Set the default Django settings module
os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'mqtt_ssu.settings')
django.setup()  # Django 앱들을 명시적으로 로드

# Initialize Django application
django_application = get_asgi_application()

# Define the ASGI application
application = ProtocolTypeRouter({
    "http": django_application,
    "websocket": AuthMiddlewareStack(
        URLRouter(
            myapp.routing.websocket_urlpatterns
        )
    ),
})




logger = logging.getLogger(__name__)

def log_startup():
    logger.info("Starting ASGI application...")

log_startup()
