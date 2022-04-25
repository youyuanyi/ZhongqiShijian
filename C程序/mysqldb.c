#include"mysqldb.h"
MYSQL mysql;
#define HOST_NAME "localhost"
#define USER_NAME "root"
#define PASSWORD  ""
#define DATABASE  "ZhongqiShijian"
#define HeadTable "HeadTable"
#define TcpTable  "TcpTable"
#define UdpTable  "UdpTable"
#define HttpTable "HttpTable"
#define HttpRequest "HttpRequest"
#define HttpGetTable "HttpGetTable"
#define HttpHeader "HttpHeader"
#define HttpResponse "HttpResponse"

void mysql_connect(MYSQL *mysql)
{
    if(!mysql_real_connect(mysql,HOST_NAME,USER_NAME,PASSWORD,DATABASE,0,NULL,0))
    {
        printf("\n连接失败:%s\n",mysql_error(mysql));
    }
    else{
        printf("\n连接成功!\n");
    }
}

/*向HeadTable表里插入数据
    mysql:句柄
    HeadID:ID号
    Protocol:TCP or UDP
    IP_version:IPv4 or IPv6
    srcIP：源IP
    dstIP：目的IP
    srcPort:源端口
    dstPort：目的端口
    caplen:实际捕获长度
    len:真正的长度
    type:类型 udp tcp response request
*/
void mysql_insert_HeadTable(MYSQL *mysql,int HeadID,char *Protocol,char *IP_version,char *srcIP,char *dstIP,int srcPort,int dstPort,int caplen,int len,char *type)
{
    char QuerySql[2002];
    char *QueryInsert="INSERT INTO ";
    char QueryField[200]="Head_ID,Protocol,IP_version,srcIP,dstIP,srcPort,dstPort,caplen,len,type";
    char *QueryLeft="(";
    char *QueryRight=")";
    char *QueryValue="VALUES";
    char QueryContent[1500];
    sprintf(QueryContent,"%d,'%s','%s','%s','%s','%d','%d','%d','%d','%s'",HeadID,Protocol,IP_version,srcIP,dstIP,srcPort,dstPort,caplen,len,type);
    //拼接Query
    sprintf(QuerySql,"%s%s%s%s%s%s %s%s%s",QueryInsert,HeadTable,QueryLeft,QueryField,QueryRight,QueryValue,QueryLeft,QueryContent,QueryRight);
    printf("Prepare to execute %s\n",QuerySql);
    if(mysql_real_query(mysql,QuerySql,strlen(QuerySql))){
        printf("HeadTable执行Insert语句失败:%s\n",mysql_error(mysql));
    }
    else
    {
        printf("\nHeadTable表插入数据成功!\n");
    }
}

/*
    向TcpTable表中插入数据
    TcpID
    HeadID
    TcpsPort
    TcpdPort
*/
void mysql_insert_TcpTable(MYSQL *mysql,int TcpID,int HeadID,int TcpsPort,int TcpdPort)
{
    char QuerySql[2002];
    char *QueryInsert="INSERT INTO ";
    char QueryField[200]="TcpID,HeadID,TcpsPort,TcpdPort";
    char *QueryLeft="(";
    char *QueryRight=")";
    char *QueryValue="VALUES";
    char QueryContent[1500];
    sprintf(QueryContent,"%d,'%d','%d','%d'",TcpID,HeadID,TcpsPort,TcpdPort);
    //拼接Query
    sprintf(QuerySql,"%s%s%s%s%s%s %s%s%s",QueryInsert,TcpTable,QueryLeft,QueryField,QueryRight,QueryValue,QueryLeft,QueryContent,QueryRight);
    printf("Prepare to execute %s\n",QuerySql);

    if(mysql_real_query(mysql,QuerySql,strlen(QuerySql))){
        printf("TcpTable执行插入语句失败:%s\n",mysql_error(mysql));
    }
    else
    {
        printf("\nTcpTable表插入数据成功!\n");
    }
}

/*
    向UdpTable表中插入数据
    UdpID
    HeadID
    UdpsPort
    UdpdPort
*/
void mysql_insert_UdpTable(MYSQL *mysql,int UdpID,int HeadID,int UdpsPort,int UdpdPort)
{
    char QuerySql[2002];
    char *QueryInsert="INSERT INTO ";
    char QueryField[200]="UdpID,HeadID,UdpsPort,UdpdPort";
    char *QueryLeft="(";
    char *QueryRight=")";
    char *QueryValue="VALUES";
    char QueryContent[1500];
    sprintf(QueryContent,"%d,'%d','%d','%d'",UdpID,HeadID,UdpsPort,UdpdPort);
    //拼接Query
    sprintf(QuerySql,"%s%s%s%s%s%s %s%s%s",QueryInsert,UdpTable,QueryLeft,QueryField,QueryRight,QueryValue,QueryLeft,QueryContent,QueryRight);
    printf("Prepare to execute %s\n",QuerySql);

    if(mysql_real_query(mysql,QuerySql,strlen(QuerySql))){
        printf("UdpTable执行插入语句失败:%s\n",mysql_error(mysql));
    }
    else
    {
        printf("\nUdpTable表插入数据成功!\n");
    }
}

/*
    向HttpGetTable表中插入数据
    HttpGetID
    HttpID
    HeadID
    HttpGet
    HttpHost
    HttpUrl
    LocalPath
*/
void mysqldb_insert_HttpGetTable(MYSQL *mysql, int HttpGetID, int HttpID,int HeadID,char *HttpGet, char *HttpHost, char *HttpUrl, char *HttpData)//存请求方式为GET的payload
{
    char Query[2000];
    char *Queryhead="INSERT INTO ";
    char Queryfield[200]="HttpGetID,HttpID,HeadID,HttpGet,HttpHost,HttpUrl,HttpData";
    char *Queryleft="(";
    char *Queryright=") ";
    char *Queryvalues="VALUES";
    char Querymessage[1500];
    sprintf(Querymessage,"%d,'%d','%d','%s','%s','%s','%s'",HttpGetID,HttpID,HeadID,HttpGet,HttpHost,HttpUrl,HttpData);
    //拼接Query
    sprintf(Query,"%s%s%s%s%s%s %s%s%s",Queryhead,HttpGetTable,Queryleft,Queryfield,Queryright,Queryvalues,Queryleft,Querymessage,Queryright);
    printf("%s\n",Query);
    int n=mysql_real_query(mysql,Query,strlen(Query));
    if(n)
    {
        printf("HttpGetTable执行插入语句失败:%s\n",mysql_error(mysql));
    }
    else
    {
        printf("\nHttpGetTable表插入数据成功!\n");
    }
}
/*
    向HttpHeader表中插入数据
    HttpHeaderId:一个key/value对应多个，不同的key/value可重复
    HeadID
    HttpHeaderName
    HttpValue
*/
void mysqldb_insert_HttpHeader(MYSQL *mysql, int HttpHeaderID,int HeadID, char *HttpHeaderName, char *HttpValue)//插入HttpHeader表的数据
{
    char QuerySql[2002];
    char *QueryInsert="INSERT INTO ";
    char QueryField[200]="HttpHeaderID,HeadID,HttpHeaderName,HttpValue";
    char *QueryLeft="(";
    char *QueryRight=")";
    char *QueryValue="VALUES";
    char QueryContent[1500];
    sprintf(QueryContent,"%d,'%d','%s','%s'",HttpHeaderID,HeadID,HttpHeaderName,HttpValue);
    //拼接Query
    sprintf(QuerySql,"%s%s%s%s%s%s %s%s%s",QueryInsert,HttpHeader,QueryLeft,QueryField,QueryRight,QueryValue,QueryLeft,QueryContent,QueryRight);
    printf("Prepare to execute %s\n",QuerySql);

    if(mysql_real_query(mysql,QuerySql,strlen(QuerySql))){
        printf("\nHttpHeader执行插入语句失败:%s\n",mysql_error(mysql));
    }
    else
    {
        printf("\nHttpHeader表插入数据成功!\n");
    }
}

/*
    向HttpTable表插入数据
    HttpID
    TcpID
    HeadID
    HttpType:Request Or Response
*/
void mysqldb_insert_HttpTable(MYSQL *mysql,int HttpID,int TcpID,int HeadID,char *HttpType)//分HttpRequest还是HttpResponse
{
    char QuerySql[2002];
    char *QueryInsert="INSERT INTO ";
    char QueryField[200]="HttpID,TcpID,HeadID,HttpType";
    char *QueryLeft="(";
    char *QueryRight=")";
    char *QueryValue="VALUES";
    char QueryContent[1500];
    sprintf(QueryContent,"%d,'%d','%d','%s'",HttpID,TcpID,HeadID,HttpType);
    //拼接Query
    sprintf(QuerySql,"%s%s%s%s%s%s %s%s%s",QueryInsert,HttpTable,QueryLeft,QueryField,QueryRight,QueryValue,QueryLeft,QueryContent,QueryRight);
    printf("Prepare to execute %s\n",QuerySql);

    if(mysql_real_query(mysql,QuerySql,strlen(QuerySql))){
        printf("\nHttpTable执行插入语句%s失败:%s\n",QuerySql,mysql_error(mysql));
    }
    else
    {
        printf("\nHttpTable表插入数据成功!\n");
    }
}
/*
	HttpRequest表
	HttpRequestID
	HttpID
	HeadID
	HttpRequestWay
*/
void mysqldb_insert_HttpRequest(MYSQL *mysql, int HttpRequestID, int HttpID,int HeadID,char *HttpRequestWay)//这个表存HTTP请求方式
{
    char QuerySql[2002];
    char *QueryInsert="INSERT INTO ";
    char QueryField[200]="HttpRequestID,HttpID,HeadID,HttpRequestWay";
    char *QueryLeft="(";
    char *QueryRight=")";
    char *QueryValue="VALUES";
    char QueryContent[1500];
    sprintf(QueryContent,"%d,'%d','%d','%s'",HttpRequestID,HttpID,HeadID,HttpRequestWay);
    //拼接Query
    sprintf(QuerySql,"%s%s%s%s%s%s %s%s%s",QueryInsert,HttpRequest,QueryLeft,QueryField,QueryRight,QueryValue,QueryLeft,QueryContent,QueryRight);
    printf("Prepare to execute %s\n",QuerySql);

    if(mysql_real_query(mysql,QuerySql,strlen(QuerySql))){
        printf("\nHttpRequest执行插入语句失败:%s\n",mysql_error(mysql));
    }
    else
    {
        printf("\nHttpRequest表插入数据成功!\n");
    }
}
//存 HttpResponseID,HttpID,HeadID ,类型,http载荷路径,正文路径
void mysqldb_insert_HttpResponse(MYSQL *mysql, int HttpResponseID, int HttpID,int HeadID,char *Content_Type,char *filename,char *bodyname)
{
	char QuerySql[2002];
    char *QueryInsert="INSERT INTO ";
    char QueryField[200]="HttpResponseID,HttpID,HeadID,ContentType,FileName,BodyName";
    char *QueryLeft="(";
    char *QueryRight=")";
    char *QueryValue="VALUES";
    char QueryContent[1500];
    sprintf(QueryContent,"%d,'%d','%d','%s','%s','%s'",HttpResponseID,HttpID,HeadID,Content_Type,filename,bodyname);
    //拼接Query
    sprintf(QuerySql,"%s%s%s%s%s%s %s%s%s",QueryInsert,HttpResponse,QueryLeft,QueryField,QueryRight,QueryValue,QueryLeft,QueryContent,QueryRight);
    printf("Prepare to execute %s\n",QuerySql);

    if(mysql_real_query(mysql,QuerySql,strlen(QuerySql))){
        printf("\nHttpRequest执行插入语句失败:%s\n",mysql_error(mysql));
    }
    else
    {
        printf("\nHttpRequest表插入数据成功!\n");
    }
}
