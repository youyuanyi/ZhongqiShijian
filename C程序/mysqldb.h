#ifndef MYSQLDB_H
#define MYSQLDB_H
#include"head_file.h"
extern MYSQL mysql;
void mysql_connect(MYSQL *mysql);//连接数据库
void mysql_insert_HeadTable(MYSQL *mysql,int HeadID,char *Protocol,char *IP_version,char *srcIP,char *dstIP,int srcPort,int dstPort,int caplen,int len,char *type);
void mysql_insert_TcpTable(MYSQL *mysql,int TcpID,int HeadID,int TcpsPort,int TcpdPort);
void mysql_insert_UdpTable(MYSQL *mysql,int UdpID,int HeadID,int UdpsPort,int UdpdPort);
/*存放payload字段的值:例如:Host:baidu.com*/
void mysqldb_insert_HttpHeader(MYSQL *mysql, int HttpHeaderID, int HeadID, char *HttpHeaderName, char *HttpValue);
/*存放是Request还是Response*/
void mysqldb_insert_HttpTable(MYSQL *mysql,int HttpID,int TcpID,int HeadID,char *HttpType);//分HttpRequest还是HttpResponse
/*存放HttpRequestWay*/
void mysqldb_insert_HttpRequest(MYSQL *mysql, int HttpRequestID, int HttpID,int HeadID,char *HttpRequestWay);//这个表存HTTP请求方式
/*存放访问url和packet保存位置*/
void mysqldb_insert_HttpGetTable(MYSQL *mysql, int HttpGetID, int HttpID,int HeadID,char *HttpGet, char *HttpHost, char *HttpUrl, char *HttpData);//存请求方式为GET的payload
void mysqldb_insert_HttpResponse(MYSQL *mysql, int HttpResponseID, int HttpID,int HeadID,char *Content_Type,char *filename,char *bodyname);
#endif
