
from django.contrib import admin
from django.urls import path, include

urlpatterns = [
    path('admin/', admin.site.urls),
    # path('', views.index, name='index'),  # 메인 페이지
    path('', include('myapp.urls')),  # myapp의 URL을 포함

    # path('get-latest-data/', views.get_latest_data, name='get_latest_data'),  # AJAX로 데이터 요청

]
