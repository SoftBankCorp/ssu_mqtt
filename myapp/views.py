# myapp/views.py

from django.shortcuts import render
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
from .models import SensorData
import paho.mqtt.publish as publish
import json

def index(request):
    # 데이터베이스에서 모든 데이터 가져오기
    data = SensorData.objects.all().order_by('-timestamp')

    # 최신 데이터 1개 가져오기
    try:
        latest_data = SensorData.objects.latest('timestamp')
    except SensorData.DoesNotExist:
        latest_data = None

    context = {
        'data': data,
        'latest_data': latest_data
    }

    return render(request, 'myapp/index.html', context)


def get_latest_data(request):
    try:
        # 최신 데이터 1개를 가져옵니다.
        latest_data = SensorData.objects.latest('timestamp')

        data = {
            'current': latest_data.current,
            'voltage': latest_data.voltage,
            'temperature': latest_data.temperature,
            'humidity': latest_data.humidity,
            'timestamp': latest_data.timestamp.isoformat()
        }
    except SensorData.DoesNotExist:
        data = {
            'current': None,
            'voltage': None,
            'temperature': None,
            'humidity': None,
            'timestamp': None
        }

    return JsonResponse(data)
@csrf_exempt
def control_motor(request):
    """모터 제어 요청 처리"""
    if request.method == 'POST':
        motor_status = request.POST.get('motor_status') == 'true'
        motor_speed = request.POST.get('motor_speed')

        # MQTT 메시지로 모터 제어 명령 전송
        publish.single("BMS/power", json.dumps({'status': motor_status}), hostname="broker.mqtt-dashboard.com")
        publish.single("MOTOR/speed", motor_speed, hostname="broker.mqtt-dashboard.com")

        return JsonResponse({'status': 'success'})
    return JsonResponse({'status': 'failed'}, status=400)
