# urls.py
from django.urls import path
from myapp import views

urlpatterns = [
    path('', views.first_view, name='average_data_view'),
    path('average-data/', views.average_data_in_views, name='average_data_in_views'),
    path('rawdata/', views.second_view, name='raw_data_view'),
    path('raw-data/', views.raw_data_in_views, name='raw_data_in_views'),
    path('control-motor/', views.control_motor, name='control_motor'),
]
