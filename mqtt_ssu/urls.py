# myproject/urls.py
from django.contrib import admin
from django.urls import path
from myapp import views

urlpatterns = [
    path('admin/', admin.site.urls),
    path('', views.index, name='index'),  # 메인 페이지
    path('get-latest-data/', views.get_latest_data, name='get_latest_data'),  # AJAX로 데이터 요청
    path('control-motor/', views.control_motor, name='control_motor'),  # 모터 제어 요청
]
