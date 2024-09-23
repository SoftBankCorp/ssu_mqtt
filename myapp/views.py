# views.py
from django.shortcuts import render
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
from .models import RawSensorData, AverageSensorData, MotorControl
import paho.mqtt.publish as publish
import json

def first_view(request):
    # Render the average data template
    return render(request, 'myapp/average_data.html')


def average_data_in_views(request):
    # Get the last 50 data points
    data_points = AverageSensorData.objects.order_by('-timestamp')[:50]
    # Reverse to get chronological order
    data_points = reversed(data_points)
    data_list = []
    for data in data_points:
        data_list.append({
            'current': data.current,
            'voltage': data.voltage,
            'temperature': data.temperature,
            'humidity': data.humidity,
            'timestamp': data.timestamp.isoformat()
        })
    return JsonResponse({'data': data_list})

def second_view(request):
    # Render the raw data template
    return render(request, 'myapp/raw_data.html')

def raw_data_in_views(request):
    # Get the last 50 data points
    data_points = RawSensorData.objects.order_by('-timestamp')[:50]
    # Reverse to get chronological order
    data_points = reversed(data_points)
    data_list = []
    for data in data_points:
        data_list.append({
            'current': data.current,
            'voltage': data.voltage,
            'temperature': data.temperature,
            'humidity': data.humidity,
            'timestamp': data.timestamp.isoformat()
        })
    return JsonResponse({'data': data_list})

def control_motor(request):
    if request.method == 'POST':
        motor_status = request.POST.get('motor_status') == 'true'
        motor_speed = request.POST.get('motor_speed')

        # Convert motor_status to integer for database storage (0 or 1)
        motor_power = 1 if motor_status else 0

        # Save to the database
        MotorControl.objects.create(power=motor_power, speed=motor_speed)

        # Publish to MQTT broker
        publish.single("motor/control", json.dumps({
            'status': motor_status,
            'speed': motor_speed
        }), hostname="broker.mqtt-dashboard.com")

        return JsonResponse({'status': 'success'})
    return JsonResponse({'status': 'failed'}, status=400)