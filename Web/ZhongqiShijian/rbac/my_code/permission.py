from django.shortcuts import HttpResponse,redirect
from django.utils.deprecation import MiddlewareMixin
import re

"""
权限限制
Session查询
"""
from django.shortcuts import render
class PermissionMiddleware(MiddlewareMixin):
    def process_request(self,request):
        # 取得当前的url用于和session中的进行比对
        current_url = request.path_info
        #进行url白名单筛选
        white_url=['/login/','/admin/.*']
        for url in white_url:
            my_url=re.match(url,current_url)
            if my_url:
                # 返回空就不会继续走后面的代码
                return None

        # 判断用户是否登录使用前面session中的user_id,如果用户没有登录，则跳转到登录界面
        user_id=request.session.get('user_id')
        if not user_id:
            return redirect('/login/')

        # 从session取出url列表
        permissions_list = request.session.get('permissions_list', [])
        trance = False

        for i in permissions_list:
            # 进行前后限定。使其匹配准确度为百分百
            i = '^{0}$'.format(i)
            match = re.match(i, current_url)
            # 如果匹配成功，状态阀改为True，并跳出匹配
            if match:
                trance = True
                break
            # 根据状态阀来判断当前用户是否有访问权限
        if not trance:
            return render(request,'newLogin.html')
