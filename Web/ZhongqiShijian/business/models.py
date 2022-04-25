# This is an auto-generated Django model module.
# You'll have to do the following manually to clean this up:
#   * Rearrange models' order
#   * Make sure each model has one field with primary_key=True
#   * Make sure each ForeignKey and OneToOneField has `on_delete` set to the desired behavior
#   * Remove `managed = False` lines if you wish to allow Django to create, modify, and delete the table
# Feel free to rename the models, but don't rename db_table values or field names.
from django.db import models


class Headtable(models.Model):
    head_id = models.IntegerField(db_column='Head_ID', primary_key=True)  # Field name made lowercase.
    protocol = models.CharField(db_column='Protocol', max_length=255, blank=True, null=True)  # Field name made lowercase.
    ip_version = models.CharField(db_column='IP_version', max_length=30, blank=True, null=True)  # Field name made lowercase.
    srcip = models.CharField(db_column='srcIP', max_length=255, blank=True, null=True)  # Field name made lowercase.
    dstip = models.CharField(db_column='dstIP', max_length=255, blank=True, null=True)  # Field name made lowercase.
    srcport = models.IntegerField(db_column='srcPort', blank=True, null=True)  # Field name made lowercase.
    dstport = models.IntegerField(db_column='dstPort', blank=True, null=True)  # Field name made lowercase.
    caplen = models.IntegerField(blank=True, null=True)
    len = models.IntegerField(blank=True, null=True)
    type = models.CharField(max_length=255, blank=True, null=True)

    class Meta:
        managed = True
        db_table = 'HeadTable'


class Httpgettable(models.Model):
    httpgetid = models.IntegerField(db_column='HttpGetID', primary_key=True)  # Field name made lowercase.
    httpid = models.ForeignKey('Httptable', models.DO_NOTHING, db_column='HttpID', blank=True, null=True)  # Field name made lowercase.
    headid = models.IntegerField(db_column='HeadID', blank=True, null=True)  # Field name made lowercase.
    httpget = models.CharField(db_column='HttpGet', max_length=255, blank=True, null=True)  # Field name made lowercase.
    httphost = models.CharField(db_column='HttpHost', max_length=255, blank=True, null=True)  # Field name made lowercase.
    httpurl = models.CharField(db_column='HttpUrl', max_length=1000, blank=True, null=True)  # Field name made lowercase.
    httpdata = models.CharField(db_column='HttpData', max_length=255, blank=True, null=True)  # Field name made lowercase.
    urltype = models.CharField(db_column='UrlType', max_length=255, blank=True, null=True)  # Field name made lowercase.
    localpath = models.CharField(db_column='LocalPath', max_length=255, blank=True, null=True)  # Field name made lowercase.
    httpfullurl = models.CharField(db_column='HttpFullUrl', max_length=1265, blank=True, null=True)  # Field name made lowercase.
    ifrequest = models.CharField(db_column='IfRequest', max_length=255, blank=True, null=True)  # Field name made lowercase.
    outfile = models.CharField(db_column='OutFile', max_length=255, blank=True, null=True)  # Field name made lowercase.

    class Meta:
        managed = True
        db_table = 'HttpGetTable'


class Httpheader(models.Model):
    httpheaderid = models.IntegerField(db_column='HttpHeaderID', blank=True, null=True)  # Field name made lowercase.
    headid = models.IntegerField(db_column='HeadID', blank=True, null=True)  # Field name made lowercase.
    httpheadername = models.CharField(db_column='HttpHeaderName', max_length=255, blank=True, null=True)  # Field name made lowercase.
    httpvalue = models.CharField(db_column='HttpValue', max_length=9999, blank=True, null=True)  # Field name made lowercase.

    class Meta:
        managed = True
        db_table = 'HttpHeader'


class Httprequest(models.Model):
    httprequestid = models.IntegerField(db_column='HttpRequestID', primary_key=True)  # Field name made lowercase.
    httpid = models.IntegerField(db_column='HttpID', blank=True, null=True)  # Field name made lowercase.
    headid = models.IntegerField(db_column='HeadID', blank=True, null=True)  # Field name made lowercase.
    httprequestway = models.CharField(db_column='HttpRequestWay', max_length=255, blank=True, null=True)  # Field name made lowercase.

    class Meta:
        managed = True
        db_table = 'HttpRequest'


class Httpresponse(models.Model):
    httpresponseid = models.IntegerField(db_column='HttpResponseID', primary_key=True)  # Field name made lowercase.
    httpid = models.IntegerField(db_column='HttpID', blank=True, null=True)  # Field name made lowercase.
    headid = models.IntegerField(db_column='HeadID', blank=True, null=True)  # Field name made lowercase.
    contenttype = models.CharField(db_column='ContentType', max_length=255, blank=True, null=True)  # Field name made lowercase.
    filename = models.CharField(db_column='FileName', max_length=255, blank=True, null=True)  # Field name made lowercase.
    bodyname = models.CharField(db_column='BodyName', max_length=255, blank=True, null=True)  # Field name made lowercase.

    class Meta:
        managed = True
        db_table = 'HttpResponse'


class Httptable(models.Model):
    httpid = models.IntegerField(db_column='HttpID', primary_key=True)  # Field name made lowercase.
    tcpid = models.IntegerField(db_column='TcpID', blank=True, null=True)  # Field name made lowercase.
    headid = models.IntegerField(db_column='HeadID', blank=True, null=True)  # Field name made lowercase.
    httptype = models.CharField(db_column='HttpType', max_length=255, blank=True, null=True)  # Field name made lowercase.

    class Meta:
        managed = True
        db_table = 'HttpTable'


class Tcptable(models.Model):
    tcpid = models.IntegerField(db_column='TcpID', primary_key=True)  # Field name made lowercase.
    headid = models.IntegerField(db_column='HeadID', blank=True, null=True)  # Field name made lowercase.
    tcpsport = models.IntegerField(db_column='TcpsPort', blank=True, null=True)  # Field name made lowercase.
    tcpdport = models.IntegerField(db_column='TcpdPort', blank=True, null=True)  # Field name made lowercase.

    class Meta:
        managed = True
        db_table = 'TcpTable'


class Udptable(models.Model):
    udpid = models.IntegerField(db_column='UdpID', primary_key=True)  # Field name made lowercase.
    headid = models.IntegerField(db_column='HeadID', blank=True, null=True)  # Field name made lowercase.
    udpsport = models.IntegerField(db_column='UdpsPort', blank=True, null=True)  # Field name made lowercase.
    udpdport = models.IntegerField(db_column='UdpdPort', blank=True, null=True)  # Field name made lowercase.

    class Meta:
        managed = True
        db_table = 'UdpTable'


class Whoistable(models.Model):
    id = models.AutoField(db_column='ID', primary_key=True)  # Field name made lowercase.
    headid = models.IntegerField(db_column='HeadID', blank=True, null=True)  # Field name made lowercase.
    host = models.CharField(db_column='Host', max_length=255, blank=True, null=True)  # Field name made lowercase.
    registrar = models.CharField(db_column='Registrar', max_length=255, blank=True, null=True)  # Field name made lowercase.
    name = models.CharField(max_length=255, blank=True, null=True)
    emails = models.CharField(max_length=255, blank=True, null=True)
    type = models.CharField(max_length=255, blank=True, null=True)

    class Meta:
        managed = True
        db_table = 'WhoisTable'
