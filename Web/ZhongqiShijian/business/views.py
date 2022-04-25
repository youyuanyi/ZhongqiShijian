import csv

from django.core import serializers
from django.http import JsonResponse, FileResponse, StreamingHttpResponse
from django.shortcuts import render, HttpResponse, redirect
from django.urls import reverse
from django.core.paginator import Paginator, PageNotAnInteger, EmptyPage
from django.db.models import Avg, Max, Min, Count, Sum
from rbac.models import *
import hashlib
import re
from business.models import *

"""
管理员的权限正则为.*
普通用户的为'^((?!Frame|Whois).)*$'
即不包括查看帧大小和Whois功能
"""


def login(request):
    if request.method == "POST":
        md5 = hashlib.md5()
        user = request.POST.get('name')
        pwd = request.POST.get('pwd')
        md5.update(pwd.encode('utf-8'))
        try:
            account = User.objects.get(name=user)
        except:
            error_msg='用户不存在'
            return render(request,'newLogin.html',{'error_msg':error_msg})
        if md5.hexdigest() == account.pwd:
            user_obj = User.objects.filter(name=user).first()
        else:
            user_obj = None
            error_msg = '密码错误'
            return render(request, 'newLogin.html', {'error_msg': error_msg})
        if user_obj:
            # 把 user的id放入session
            request.session['user_id'] = user_obj.pk
            # 先把用户对应的拥有的url添加取出放到session中备用（通过user_obj跨表到role再到permission拿到url）
            role_obj = user_obj.roles.all()
            role_title = role_obj[0]

            role_id = Role.objects.get(title=role_title).pk
            # 这里判断是什么角色
            per_list = role_obj.values(
                'permission')  # 取出的url是queryset对象 <QuerySet [{'permission': 3}, {'permission': 4}, {'permission': 5}]>
            permission_list = []

            for i in per_list:
                for per in Permission.objects.all():
                    if per.pk == i['permission']:
                        permission_list.append(per.url)

            request.session['permissions_list'] = permission_list
            return redirect(reverse("user", kwargs={'role_id': role_id}))
    return render(request, 'newLogin.html')


def user(request, role_id):
    # 获取帧长度，最大帧，最小帧，平均帧
    # 获取饼状图类型
    if User.objects.filter(roles=role_id):  # 防止跨角色登录
        user_list = User.objects.all()
        # 首先从HeadTable表中获取帧信息
        srcframe_data = Headtable.objects.filter(type='HttpRequest').aggregate(Max('caplen'), Min('caplen'),
                                                                               Avg('caplen'))
        srcMax = srcframe_data['caplen__max']
        srcMin = srcframe_data['caplen__min']
        srcAvg = int(srcframe_data['caplen__avg'])

        dstframe_data = Headtable.objects.filter(type='HttpResponse').aggregate(Max('caplen'), Min('caplen'),
                                                                                Avg('caplen'))
        dstMax = dstframe_data['caplen__max']
        dstMin = dstframe_data['caplen__min']
        dstAvg = int(dstframe_data['caplen__avg'])

        # 获取请求包的类型数目，响应包的类型数目
        srcType_data = Httpgettable.objects.values('httpget').annotate(num=Count(Httpgettable.httpget)).values(
            'httpget', 'num')
        httpget = srcType_data[0]['num']
        httppost = srcType_data[1]['num']
        httphead = srcType_data[2]['num']

        dstType_data = Httpresponse.objects.values('contenttype').annotate(num=Count(Httpresponse.contenttype)).values(
            'contenttype', 'num')
        content_type_gif = dstType_data[0]['num']
        content_type_js = dstType_data[1]['num']
        content_type_html = dstType_data[3]['num'] + dstType_data[4]['num'] + dstType_data[11]['num'] + \
                            dstType_data[12]['num'] + dstType_data[13]['num']
        content_type_xml = dstType_data[5]['num']
        content_type_jpeg = dstType_data[7]['num']
        content_type_png = dstType_data[9]['num']

        return render(request, 'user.html', locals())
    else:
        return render(request, 'login.html')


def add_user(request):
    # 从session取出url列表
    permissions_list = request.session.get('permissions_list')
    # 取得当前的url用于和session中的进行比对
    current_url = request.path_info
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
        return HttpResponse('你没有权限访问！！！')

    return HttpResponse("添加用户成功!")


def role(request):
    user_list = User.objects.all()
    role_obj = user_list.values('name', 'roles__title')  # 只取这两个字段

    return render(request, 'role.html', {'role_obj': role_obj})


def pagination_function(paginator, page, is_paginated=True):
    if not is_paginated:
        # 如果没有分页，则无需显示分页导航条，不用任何分页导航条的数据，因此返回一个空的字典
        return {}

    # 当前页左边连续的页码号，初始值为空
    left = []

    # 当前页右边连续的页码号，初始值为空
    right = []

    # 标示第 1 页页码后是否需要显示省略号
    left_has_more = False

    # 标示最后一页页码前是否需要显示省略号
    right_has_more = False

    # 标示是否需要显示第 1 页的页码号。
    # 因为如果当前页左边的连续页码号中已经含有第 1 页的页码号，此时就无需再显示第 1 页的页码号，
    # 其它情况下第一页的页码是始终需要显示的。
    # 初始值为 False
    first = False

    # 标示是否需要显示最后一页的页码号。
    # 需要此指示变量的理由和上面相同。
    last = False

    # 获得用户当前请求的页码号
    page_number = page.number

    # 获得分页后的总页数
    total_pages = paginator.num_pages

    # 获得整个分页页码列表，比如分了四页，那么就是 [1, 2, 3, 4]
    page_range = paginator.page_range

    if page_number == 1:
        # 如果用户请求的是第一页的数据，那么当前页左边的不需要数据，因此 left=[]（已默认为空）。
        # 此时只要获取当前页右边的连续页码号，
        # 比如分页页码列表是 [1, 2, 3, 4]，那么获取的就是 right = [2, 3]。
        # 注意这里只获取了当前页码后连续两个页码，你可以更改这个数字以获取更多页码。
        right = page_range[page_number:page_number + 2]
        # 如果最右边的页码号比最后一页的页码号减去 1 还要小，
        # 说明最右边的页码号和最后一页的页码号之间还有其它页码，因此需要显示省略号，通过 right_has_more 来指示。
        if len(right) != 0:
            if right[-1] < total_pages - 1:
                right_has_more = True
            #
            # 如果最右边的页码号比最后一页的页码号小，说明当前页右边的连续页码号中不包含最后一页的页码
            # 所以需要显示最后一页的页码号，通过 last 来指示
            if right[-1] < total_pages:
                last = True

    elif page_number == total_pages:
        # 如果用户请求的是最后一页的数据，那么当前页右边就不需要数据，因此 right=[]（已默认为空），
        # 此时只要获取当前页左边的连续页码号。
        # 比如分页页码列表是 [1, 2, 3, 4]，那么获取的就是 left = [2, 3]
        # 这里只获取了当前页码后连续两个页码，你可以更改这个数字以获取更多页码。
        left = page_range[(page_number - 3) if (page_number - 3) > 0 else 0:page_number - 1]

        # 如果最左边的页码号比第 2 页页码号还大，
        # 说明最左边的页码号和第 1 页的页码号之间还有其它页码，因此需要显示省略号，通过 left_has_more 来指示。
        if left[0] > 2:
            left_has_more = True

        # 如果最左边的页码号比第 1 页的页码号大，说明当前页左边的连续页码号中不包含第一页的页码，
        # 所以需要显示第一页的页码号，通过 first 来指示
        if left[0] > 1:
            first = True
    else:
        # 用户请求的既不是最后一页，也不是第 1 页，则需要获取当前页左右两边的连续页码号，
        # 这里只获取了当前页码前后连续两个页码，你可以更改这个数字以获取更多页码。
        left = page_range[(page_number - 3) if (page_number - 3) > 0 else 0:page_number - 1]
        right = page_range[page_number:page_number + 2]

        # 是否需要显示最后一页和最后一页前的省略号
        if right[-1] < total_pages - 1:
            right_has_more = True
        if right[-1] < total_pages:
            last = True

        # 是否需要显示第 1 页和第 1 页后的省略号
        if left[0] > 2:
            left_has_more = True
        if left[0] > 1:
            first = True

    data = {
        'left': left,
        'right': right,
        'left_has_more': left_has_more,
        'right_has_more': right_has_more,
        'first': first,
        'last': last,
    }

    return data


# 展示WhoisTable表
def ShowWhois(request, role_id, pIndex=1):
    try:
        list = Whoistable.objects.all()[:300]
        p = Paginator(list, 20)  # 以20条数据一页实例化分页对象
        page_num = pIndex
        print(page_num)
        # 判断页码值是否有效
        try:
            page_num = int(page_num)
        except:
            page_num = 1

        if page_num < 1 or page_num > p.num_pages:
            page_num = 1

        is_pagniated = True
        page_object = p.get_page(page_num)
        data = pagination_function(p, page_object, is_pagniated)
        ulist = p.page(pIndex)  # 获取当前页数据

        for u in ulist:
            if u.host == None:
                u.host = '未知'
            if u.name == None:
                u.name = '未知'
            if u.emails == None:
                u.emails = '未知'
            if u.type == None:
                u.type = '未指定'

        context = {"ulist": ulist, "pIndex": pIndex, "pagelist": p.page_range, 'role_id': role_id,
                   'num_pages': p.num_pages}
        return render(request, 'ShowWhoisTable.html', locals())
    except:
        return HttpResponse("404 NOT FOUND")


# 指定Host类型
def DefineHostType(request):
    hostType = request.POST.get("hostType")
    headId = request.POST.get("HeadIDText")
    try:
        data_obj = Whoistable.objects.get(headid=int(headId))
        data_obj.type = hostType
        data_obj.save()
        return ShowWhois(request, 1, 1)

    except:
        return HttpResponse('Save Fail')

    return HttpResponse('ok')


# 统计请求报文各源IP的数量
def GetHttpRequestNum(request, role_id, pIndex=1):
    import pymysql
    # 1.连接
    conn = pymysql.connect(host='127.0.0.1', port=3306, user='root', password='', db='lzc', charset='utf8')
    print(conn)
    # 2.创建游标
    cursor = conn.cursor()
    # 注意%s需要加引号
    sql = "select srcIP,COUNT(*) as Num from HeadTable where type='HttpRequest' GROUP BY srcIP"

    cursor.execute(sql)
    data_list = cursor.fetchall()
    cursor.close()
    conn.close()

    p = Paginator(data_list, 20)  # 以20条数据一页实例化分页对象
    page_num = pIndex
    print('页码数', p.num_pages)
    # 判断页码值是否有效
    try:
        page_num = int(page_num)
    except:
        page_num = 1

    if page_num < 1 or page_num > p.num_pages:
        page_num = 1

    is_pagniated = True
    page_object = p.get_page(page_num)
    data = pagination_function(p, page_object, is_pagniated)
    ulist = p.page(pIndex)  # 获取当前页数据

    return render(request, 'GetHttpRequestNum.html', locals())


# 统计目的报文各源IP的数量
def GetHttpResponseNum(request, role_id, pIndex=1):
    import pymysql
    # 1.连接
    conn = pymysql.connect(host='127.0.0.1', port=3306, user='root', password='', db='lzc', charset='utf8')
    print(conn)
    # 2.创建游标
    cursor = conn.cursor()
    # 注意%s需要加引号
    sql = "select dstIP,COUNT(*) as Num from HeadTable where type='HttpResponse' GROUP BY dstIP"

    cursor.execute(sql)
    data_list = cursor.fetchall()
    cursor.close()
    conn.close()
    p = Paginator(data_list, 20)  # 以20条数据一页实例化分页对象
    page_num = pIndex
    print('页码数', p.num_pages)
    # 判断页码值是否有效
    try:
        page_num = int(page_num)
    except:
        page_num = 1

    if page_num < 1 or page_num > p.num_pages:
        page_num = 1

    is_pagniated = True
    page_object = p.get_page(page_num)
    data = pagination_function(p, page_object, is_pagniated)
    ulist = p.page(pIndex)  # 获取当前页数据

    return render(request, 'GetHttpResponseNum.html', locals())


# 列出所有请求报文的特征
"""
特征：
HeadID
源IP地址
目的IP地址
源端口
目的端口
Host
FullURL
User-Agent
cookie
caplen
外部文件存储位置
"""


def ListRequest(request, role_id, pIndex=1):
    RequestList = Httpgettable.objects.all()[:300]
    res = []

    for data in RequestList:
        fullurlList = []
        hostList = []
        outfileList = []
        user_agentList = []
        cookieList = []
        headidlist = []
        srcIPlist = []
        dstIPlist = []
        srcPortList = []
        dstPortList = []
        caplenList = []

        fullurl = data.httpfullurl  # 完整的URL
        fullurlList.append(fullurl)
        host = data.httphost  # Host
        hostList.append(host)

        outfile = data.outfile  # 外部文件路径
        if outfile == None:
            outfile = '无'
        outfileList.append(outfile)
        user_agent = '未知'  # User-Agent
        cookie = '无'  # Cookie

        headtableData = Headtable.objects.get(head_id=data.headid)
        headID = headtableData.head_id  # HeadID
        headidlist.append(headID)
        srcIP = headtableData.srcip  # srcIP
        srcIPlist.append(srcIP)
        dstip = headtableData.dstip  # dstIP
        dstIPlist.append(dstip)
        srcport = headtableData.srcport  # srcPort
        srcPortList.append(srcport)
        dstport = headtableData.dstport  # dstPort
        dstPortList.append(dstport)
        caplen = headtableData.caplen  # caplen
        caplenList.append(caplen)

        packet = Httpheader.objects.filter(headid=int(headID))  # 有多种请况
        # 遍历取到的HttpHeader对象
        for i in packet:
            if i.httpheadername == 'User-Agent':
                user_agent = i.httpvalue
            if i.httpheadername == 'Cookie':
                cookie = i.httpvalue

        user_agentList.append(user_agent)
        cookieList.append(cookie)

        res.append(
            zip(headidlist, srcIPlist, dstIPlist, srcPortList, dstPortList, hostList, fullurlList, user_agentList,
                cookieList, caplenList, outfileList))

    p = Paginator(res, 20)  # 以20条数据一页实例化分页对象
    page_num = pIndex
    # 判断页码值是否有效
    try:
        page_num = int(page_num)
    except:
        page_num = 1

    if page_num < 1 or page_num > p.num_pages:
        page_num = 1

    is_pagniated = True
    page_object = p.get_page(page_num)
    data = pagination_function(p, page_object, is_pagniated)
    ulist = p.page(pIndex)  # 获取当前页数据

    return render(request, 'ListRequest.html', locals())

def DownloadRequest(request,role_id):
    RequestList = Httpgettable.objects.all()[:300]
    res = []

    for data in RequestList:
        fullurlList = []
        hostList = []
        outfileList = []
        user_agentList = []
        cookieList = []
        headidlist = []
        srcIPlist = []
        dstIPlist = []
        srcPortList = []
        dstPortList = []
        caplenList = []

        fullurl = data.httpfullurl  # 完整的URL
        fullurlList.append(fullurl)
        host = data.httphost  # Host
        hostList.append(host)

        outfile = data.outfile  # 外部文件路径
        if outfile == None:
            outfile = '无'
        outfileList.append(outfile)
        user_agent = '未知'  # User-Agent
        cookie = '无'  # Cookie

        headtableData = Headtable.objects.get(head_id=data.headid)
        headID = headtableData.head_id  # HeadID
        headidlist.append(headID)
        srcIP = headtableData.srcip  # srcIP
        srcIPlist.append(srcIP)
        dstip = headtableData.dstip  # dstIP
        dstIPlist.append(dstip)
        srcport = headtableData.srcport  # srcPort
        srcPortList.append(srcport)
        dstport = headtableData.dstport  # dstPort
        dstPortList.append(dstport)
        caplen = headtableData.caplen  # caplen
        caplenList.append(caplen)

        packet = Httpheader.objects.filter(headid=int(headID))  # 有多种请况
        # 遍历取到的HttpHeader对象
        for i in packet:
            if i.httpheadername == 'User-Agent':
                user_agent = i.httpvalue
            if i.httpheadername == 'Cookie':
                cookie = i.httpvalue

        user_agentList.append(user_agent)
        cookieList.append(cookie)

        res.append(
            zip(headidlist, srcIPlist, dstIPlist, srcPortList, dstPortList, hostList, fullurlList, user_agentList,
                cookieList, caplenList, outfileList))

    response = HttpResponse(content_type='text/csv', charset='UTF-8')
    filename='Request' #文件名
    response['Content-Disposition'] = f'attachment; filename="{filename}.csv"'
    # 生成一个对象
    writer = csv.writer(response)
    # 定义表头
    writer.writerow(['HeadID', '源IP', '目的IP','源端口','目的端口','Host','Request-URI','User-Agent','帧长度','外部文件路径','Cookie'])
    for idx in res:
        for headID,srcIP,dstIP,srcPort,dstPort,host,fullurl,user_agent,cookie,caplen,outfile in idx:
            writer.writerow([headID,srcIP,dstIP,srcPort,dstPort,host,fullurl,user_agent,cookie,caplen,outfile])
    return response

# 点击Host，然后在新的页面显示该域名信息
def FromHostListWhois(request, role_id, headid):
    whoisData = Whoistable.objects.get(headid=headid)

    return render(request, 'FromHostListWhois.html', locals())


# 展示响应包的特征
"""
HeadID
源IP
目的IP
源端口
目的端口
Content-type值
caplen
outfile
"""


def ListResponse(request, role_id, pIndex=1):
    responseData = Httpresponse.objects.all()
    res = []
    import pymysql
    for data in responseData:
        contenttypeList = []
        bodyfileList = []
        headidlist = []
        srcIPlist = []
        dstIPlist = []
        srcPortList = []
        dstPortList = []
        caplenList = []

        contenttypeList.append(data.contenttype)
        bodyfileList.append(data.bodyname)
        headidlist.append(data.headid)
        # 打开数据库连接
        # 1.连接
        conn = pymysql.connect(host='127.0.0.1', port=3306, user='root', password='', db='lzc', charset='utf8')

        cursor = conn.cursor()

        sql = "select * from HeadTable where Head_ID=%s"
        # 3.执行sql语句

        cursor.execute(sql, [data.headid])

        headtableData = cursor.fetchall()  # 获取查询的所有记录

        cursor.close()
        conn.close()
        headtableData = list(headtableData)

        try:  # 这里的try一定不能删除，否则会报list为空的error
            x = headtableData[0]
        except:
            print('error')

        srcIPlist.append(x[3])
        dstIPlist.append(x[4])
        srcPortList.append(x[5])
        dstPortList.append(x[6])
        caplenList.append(x[7])

        res.append(
            zip(headidlist, srcIPlist, dstIPlist, srcPortList, dstPortList, caplenList, contenttypeList, bodyfileList))

    p = Paginator(res, 20)  # 以20条数据一页实例化分页对象
    page_num = pIndex
    # 判断页码值是否有效
    try:
        page_num = int(page_num)
    except:
        page_num = 1

    if page_num < 1 or page_num > p.num_pages:
        page_num = 1

    is_pagniated = True
    page_object = p.get_page(page_num)
    data = pagination_function(p, page_object, is_pagniated)
    ulist = p.page(pIndex)  # 获取当前页数据
    return render(request, 'ListResponse.html', locals())


# 展示响应包的载荷内容
def FromContentTypeListPayload(request, role_id, headid):
    file = "./resfile/files/" + str(headid)
    fo = open(file, 'r', encoding='unicode_escape')

    content = fo.read()

    fo.close()
    return render(request, 'FromContentTypeListPayload.html', locals())


# 下载响应包payload
def DownloadPayload(request, role_id, headid):
    # 先判断类型
    file = "./resfile/files/" + str(headid)
    fo = open(file, 'r', encoding='unicode_escape')
    content = fo.read()
    fo.close()

    response = StreamingHttpResponse(content)
    response['Content-Type'] = 'application/octet-stream'
    response['Content-Disposition'] = 'attachment;filename="{0}"'.format(file)
    return response


# list出每个源地址的host公司类型
# 补充一个搜索功能，可以根据srcIP进行搜索
def ListPerSrcIPHost(request, role_id, pIndex=1):
    whoisData = Whoistable.objects.all()[:300]
    res = []
    # 获取并判断搜索条件
    kw = request.GET.get('keyword', None)
    mywhere = ""  # 条件变量
    for w in whoisData:
        headIDList = []
        srcIPList = []
        HostList = []
        RegistrarList = []
        TypeList = []
        headTableData = Headtable.objects.get(head_id=w.headid)
        if kw:  # 如果有查询条件
            kw = str(kw)
            kw = kw.strip()
            mywhere = "?keyword="+kw # 追加搜索条件  # 封装搜索条件，分页时条件不丢失
            if headTableData.srcip == kw:  # 判断srcIP是否和kw相等
                srcIPList.append(headTableData.srcip)
            else:
                continue
        else:
            srcIPList.append(headTableData.srcip)

        headIDList.append(w.headid)

        HostList.append(w.host)
        RegistrarList.append(w.registrar)
        hosttype = w.type
        if hosttype == None:
            hosttype = '未指定'
        TypeList.append(hosttype)

        res.append(zip(headIDList, srcIPList, HostList, RegistrarList, TypeList))

    p = Paginator(res, 15)  # 以20条数据一页实例化分页对象
    page_num = pIndex
    # 判断页码值是否有效
    try:
        page_num = int(page_num)
    except:
        page_num = 1

    if page_num < 1 or page_num > p.num_pages:
        page_num = 1

    is_pagniated = True
    page_object = p.get_page(page_num)
    data = pagination_function(p, page_object, is_pagniated)
    ulist = p.page(pIndex)  # 获取当前页数据

    return render(request, 'ListPerSrcIPHost.html', locals())

# 分析HTML性质
def AnalysizeHtml(request,role_id,pIndex=1):
    from .Kw import UnicodeAcAutomation
    ac = UnicodeAcAutomation()
    import os
    # 读入词库
    for line in open("dict", 'r', encoding='utf-8', errors='ignore'):
        tmp = ''
        for i in line:
            if i != '\n':
                tmp += i
        ac.insert(tmp);

    ac.build_automation();

    file_path = "./kwresfile/resfile/"
    file_names = os.listdir(file_path)
    mylist=[]
    for i in file_names:
        headidList=[]
        srcipList=[]
        dstipList=[]
        typeList=[]
        ContentList=[]

        try:
            with open(file_path + i, 'r', encoding='gb2312') as f:
                txt = f.read()
                a = "%12s" % (i)
                if a.endswith('.html') or a.endswith('.txt'):
                    dotpos=a.find('.')
                    if a[dotpos+1]=='h':
                        responseType='html'
                    elif a[dotpos+1]=='t':
                        responseType='txt'
                    headid=a[0:dotpos] #取到headid，和Headtable表进行匹配
                    try:

                        headid=str(headid)
                        headid=headid.strip()
                        packetobj=Headtable.objects.get(head_id=headid)
                        srcip=packetobj.srcip
                        dstip=packetobj.dstip
                        urltype=ac.matchOne(txt)[1]
                        headidList.append(headid)
                        srcipList.append(srcip)
                        dstipList.append(dstip)
                        typeList.append(responseType)
                        ContentList.append(urltype)
                        mylist.append(zip(headidList,srcipList,dstipList,typeList,ContentList))
                    except:
                        continue

        except:
            continue

    p = Paginator(mylist, 15)  # 以20条数据一页实例化分页对象
    page_num = pIndex
    # 判断页码值是否有效
    try:
        page_num = int(page_num)
    except:
        page_num = 1

    if page_num < 1 or page_num > p.num_pages:
        page_num = 1

    is_pagniated = True
    page_object = p.get_page(page_num)
    data = pagination_function(p, page_object, is_pagniated)
    ulist = p.page(pIndex)  # 获取当前页数据

    return render(request,'AnalysizeHtml.html',locals())
