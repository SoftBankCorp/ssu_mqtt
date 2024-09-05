
from django.urls import path
from . import views

urlpatterns = [
    path('', views.index, name='index'),
    path('get-latest-data/', views.get_latest_data, name='get_latest_data'),
    path('control-motor/', views.control_motor, name='control_motor'),
]
