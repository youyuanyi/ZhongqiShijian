"""ZhongqiShijian URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/3.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path
from business import views as bussiness_views

urlpatterns = [
    path('',bussiness_views.login),
    path('admin/', admin.site.urls),
    path('login/',bussiness_views.login,name='login'),
    path('user/role_id=<int:role_id>',bussiness_views.user,name='user'),
    path('add/user/',bussiness_views.add_user,name='add_user'),
    path('user/role_id=<int:role_id>/ShowWhois/pIndex=<int:pIndex>',bussiness_views.ShowWhois,name='ShowWhois'),
    path('user/DefineHostType',bussiness_views.DefineHostType,name='DefineHostType'), #指定HostType类型,
    path('user/role_id=<int:role_id>/GetHttpRequestNum/pIndex=<int:pIndex>',bussiness_views.GetHttpRequestNum,name='GetHttpRequestNum'),
    path('user/role_id=<int:role_id>/GetHttpResponse/pIndex=<int:pIndex>',bussiness_views.GetHttpResponseNum,name='GetHttpResponseNum'),
    path('user/role_id=<int:role_id>/ListRequest/pIndex=<int:pIndex>',bussiness_views.ListRequest,name='ListRequest'),
    path('user/role_id=<int:role_id>/FromHostListWhois/headid=<int:headid>',bussiness_views.FromHostListWhois,name='FromHostListWhois'),
    path('user/role_id=<int:role_id>/ListResponse/pIndex=<int:pIndex>',bussiness_views.ListResponse,name='ListResponse'),
    path('user/role_id=<int:role_id>/FromContentTypeListPayload/headid=<int:headid>',bussiness_views.FromContentTypeListPayload,name='FromContentTypeListPayload'),
    path('user/role_id=<int:role_id>/DownloadPayload/headid=<int:headid>',bussiness_views.DownloadPayload,name='DownloadPayload'),
    path('user/role_id=<int:role_id>/ListPerSrcIPHost/pIndex=<int:pIndex>',bussiness_views.ListPerSrcIPHost,name='ListPerSrcIPHost'),
    path('user/role_id=<int:role_id>/AnalysizeHtml/pIndex=<int:pIndex>',bussiness_views.AnalysizeHtml,name='AnalysizeHtml'),
    path('user/role_id=<int:role_id>/DownloadRequest/',bussiness_views.DownloadRequest,name='DownloadRequest'),
]
