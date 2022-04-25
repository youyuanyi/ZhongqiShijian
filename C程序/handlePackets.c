#include"handlePackets.h"
#include"head_struct.h"
#include"mysqldb.h"
#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_IPV6 0x86DD
#define MAX_NUMBER_FOR_SPLIT 1500//记录分割字符串时，最大字符串数量
#define MAX_KEYVALUE 8000//在strstr2中使用，表示最大keyvalue长度
#define MAXCONTENT 5000//http Content 最大容量
/*算有多少个IPV6帧*/
int num=0; 
/*标记是第几个包*/
int HeadID=1;
/*TCP包编号*/
int TcpID=1;
/*UDP包编号*/
int UdpID=1;
/*Http包编号*/
int HttpID=1;
/*RequestID*/
int HttpRequestID=1;
/*ResponseID*/
int HttpResponseID=0;
int HttpGetID=1;
int HttpHeaderID=1;
FILE *fp;

//ip整型转换点分十进制
char *InttoIpv4str(u_int32_t num){
    char* ipstr = (char*)calloc(128, sizeof(char*));

    if (ipstr)
        sprintf(ipstr, "%d.%d.%d.%d", num >> 24 & 255, num >> 16 & 255, num >> 8 & 255, num & 255);
    else
        printf("failed to Allocate memory...");

    return ipstr;
}

int print_payload(const u_int8_t *payload,int len)
{
    int len_rem=len;
    int line_width=16;
    int line_len;
    int offset=0;
    const u_char *ch=payload;

    if(len<=0)
        return -1;
    if(len<=line_width)
    {
        print_hex_ascii_line(ch,len,offset);
        return 0;
    }
    for(;;)
    {
        line_len=line_width % len_rem;
        print_hex_ascii_line(ch,line_len,offset);
        len_rem=len_rem - line_len;
        ch=ch+line_len;
        offset=offset+line_width;
        if(len_rem<=line_width)
        {
            print_hex_ascii_line(ch,len_rem,offset);
            break;
        }
    }
    return 0;
}

void print_hex_ascii_line(const u_int8_t *payload,int len,int offset){
    int i;
    int gap;
    const u_char *ch;
    printf("%05d      ",offset);
    ch=payload;
    for(i=0;i<len;i++)
    {
        printf("%02x",*ch);
        ch++;
        if(i==7)
            printf(" ");
    }
    if(len<8)
        printf(" ");
    if(len<16)
    {
        gap=16-len;
        for(i=0;i<gap;i++)
        {
            printf(" ");
        }
    }
    printf("           ");

    ch=payload;
    for(i=0;i<len;i++)
    {   
        if(isprint(*ch))
            printf("%c",*ch);
        else
            printf(".");
        ch++;
    }   
    printf("\n");
    return;
}


char *strstr1(const char *s1,const char *delm){
    int len2;
    if(!(len2=strlen(delm)))
        return (char*)(s1+4);//偏移4，使得跳过\r\n
    for(;*s1;++s1){
        if(*s1==*delm && strncmp(s1,delm,len2)==0)
            return (char *)(s1+4);//偏移4，使得跳过\r\n
    }
    return NULL;
}
char *strstr2(const char *s1,const char *delm){
    //这两行方便s1为空时候使用
    const char *source;
    source=s1;

    int len2;
    if(!(len2=strlen(delm)))
        return (char*)s1;
    char before[MAX_KEYVALUE];
    char *p;
    p=before;
    for(int i=0;*s1;++s1,++i){
        if(*s1==*delm && strncmp(s1,delm,len2)==0)
        {
            before[i]=*s1;
            ++s1,++i;
            before[i]='\0';
            return (char *)p;
        }
        else
            before[i]=*s1;
    }
    return (char *)source;
}

/*HttpRequest分为
        请求行
        请求头部
        请求数据
*/
void HandleHttpRequestPayload(const char *Payload,const int TcpPayloadLen)
{
    int t;
    printf("start handle HttpRequest\n");
    t=print_payload(Payload,TcpPayloadLen);
    if(t==0)
    {
    	    //print_payload(Payload,TcpPayloadLen);
    }
    else return ;
    const u_int8_t *ch; //8bit:一个字节一个字节拆
    ch=Payload;         //起始位置
    printf("First character =%u\n",*ch);
    if(!isprint(*ch)){ //不能打印\r\n,\0以及其他不符合编码的数据
        printf("this Http payload is encrypted\n");
        return;
    }
    /*这里只处理Http 1.0的请求方法,不处理Http 1.1新增的六种请求方法*/
    if(Payload[0]!='G' && Payload[0]!='H' && Payload[0]!='P'){
        return;
    }
    if(!(
        strstr(Payload,"GET")!=NULL ||
        strstr(Payload,"HEAD")!=NULL ||
        strstr(Payload,"POST")!=NULL ||
        strstr(Payload,"PUT")!=NULL ||
        strstr(Payload,"DELETE")!=NULL ||
        strstr(Payload,"CONNECT")!=NULL ||
        strstr(Payload,"OPTIONS")!=NULL ||
        strstr(Payload,"TRACE")!=NULL ||
        //响应头
        strstr(Payload,"HTTP")!=NULL )){
        printf("this HTTP payload is encrypted 2 and *ch=%c\n",*ch);
        return;
    }
    int num_header=0;
    printf("before request malloc\n");
    //u_char *load=(u_char*)malloc(SNAP_LEN);
    u_char load[SNAP_LEN];
    printf("after request malloc\n");
    //if(load==NULL)
    //{
    //	printf(" HandleHttpRequestPayload malloc error!\n");
    //	return ;
    //}
    strcpy(load,Payload);
    const char *split_char="\r\n";  //以\r\n分割
    const char *blank=" ";
    const char *maohao=":";
    char *Line="0";

    /* 第一行请求行的三个字段以空格分割，然后以\r\n换行 */
    /*存放GET方法*/
    char firststr[10];
    /*存放Host地址*/
    char secondstr[9999];
    Line=strtok(load,blank);
    strcpy(firststr,Line); //获取请求方法
    firststr[9]=0;
    printf("firststr=%s\n",firststr);

    Line=strtok(NULL,blank); //获取URL
    strcpy(secondstr,Line); 
    int secondstrlen=strlen(secondstr);
    if(secondstrlen>200)
        return;
    printf("secondstr=%s\n",secondstr);

    int current_length=-1;
    current_length=strlen(firststr)+1+strlen(secondstr);
    printf("current_length=%d\tTcpPayloadLen=%d\n",current_length,TcpPayloadLen);

    if(current_length>=TcpPayloadLen)
    {
        //插入HTTP_request_data数据
        mysqldb_insert_HttpRequest(&mysql,HttpRequestID,HttpID,HeadID,firststr);
        HttpRequestID+=1; 
        return;
    }
    Line=strtok(NULL,split_char);

    Line=strtok(NULL,"");
    Line+=1;//之前获得的Line有\r\n，要跳过这两个字符

    /*开始第2步，拆key/value*/
    char *KeyValue="0";
    KeyValue=strstr2(Line,"\r\n\r\n");
    char filename[20];
    char filename2[20];
    sprintf(filename2,"%d",HeadID); //将HeadID写入filename2
    sprintf(filename,"./files/%d",HeadID);
    if((fp=fopen(filename,"w"))==NULL){
        printf("Open File Error!\n");
        return;
    }
    printf("before write\n");
    fwrite(Payload,sizeof(Payload),TcpPayloadLen,fp);
    printf("after write\n");
    fclose(fp);
    KeyValue[strlen(KeyValue)]=='\0';
    printf("KeyValue:\n%s\n",KeyValue);
    /*key_value真正的个数-1*/
    int count_real_num=0;
    for(int i=0;i<strlen(KeyValue);i++){
        if(KeyValue[i]=='\n') //如果是换行，即已经遍历了一行Key/value,要进入下一个Key/value
            count_real_num++;
    }
    printf("count_real_num:%d\n",count_real_num);
    char OneLine[MAX_NUMBER_FOR_SPLIT][MAX_LEN_FOR_SPLIT] = {0}; //存放分割后的子字符串 
    char aroundColon[MAX_NUMBER_FOR_SPLIT][MAX_LEN_FOR_SPLIT]={0};//存放冒号两边的值
    int num=0;
    int num_aroundColon=0;
   

    if(KeyValue!=NULL){
        SplitByMultiDelim(KeyValue,split_char,OneLine,&num);
        int i;
        printf("num:%d\n",num);
        for(i=0;i<count_real_num;i++){
            if(OneLine[i]=="")
                break;
            SplitByMultiDelim(OneLine[i],": ",aroundColon,&num_aroundColon);
            printf("Key=%s : Value=%s\n",aroundColon[0],aroundColon[1]);
	    if((!strcmp(firststr,"GET") && (!strcmp(aroundColon[0],"Host")))|| (!strcmp(firststr,"POST") && (!strcmp(aroundColon[0],"Host"))) || (!strcmp(firststr,"HEAD") && (!strcmp(aroundColon[0],"Host"))))
       	    {
           	mysqldb_insert_HttpGetTable(&mysql,HttpGetID,HttpID,HeadID, firststr, aroundColon[1], secondstr, filename2);
            	HttpGetID+=1;
            }
            mysqldb_insert_HttpHeader(&mysql, i+1, HeadID,aroundColon[0], aroundColon[1]); //存放该Request payload中的所有Key/value字段,一行存一个
        }
    }
    return; 
}

/*处理响应包，需要IP，端口号，帧长度,Content-type*/
/*
需求：1. HTTP 协议载荷文件，文件名为报获报文顺序编号
      2. 保存 response 载荷的外部文件 path(对应为变量filename) 
      3. 保存 response 中Content-Type 值
      4. 根据Content-Type 值对载荷处理，保存为对应文件，文件名为序号
*/
void HandleHttpResponsePayload(const char *Payload,const int TcpPayloadLen)
{
    int t;
    t=print_payload(Payload,TcpPayloadLen);
    if(t==0)
    {
    	    //print_payload(Payload,TcpPayloadLen);
    }
    else return ;

    printf("start handle HttpResponse\n");
    const u_int8_t *ch; //8bit:一个字节一个字节拆
    ch=Payload;         //起始位置
    printf("First character =%u\n",*ch);
    if(!isprint(*ch)){ //不能打印\r\n,\0以及其他不符合编码的数据
        printf("this Http payload is encrypted\n");
        return;
    }
    /*这里只处理响应报文*/
    if(Payload[0]!='H'){
        return;
    }
     if(!(
        strstr(Payload,"GET")!=NULL ||
        strstr(Payload,"HEAD")!=NULL ||
        strstr(Payload,"POST")!=NULL ||
        strstr(Payload,"PUT")!=NULL ||
        strstr(Payload,"DELETE")!=NULL ||
        strstr(Payload,"CONNECT")!=NULL ||
        strstr(Payload,"OPTIONS")!=NULL ||
        strstr(Payload,"TRACE")!=NULL ||
        //响应头
        strstr(Payload,"HTTP")!=NULL )){
        printf("this HTTP payload is encrypted and *ch=%c\n",*ch);
        return;
    }
    int num_header=0;
    //u_char *load=(u_char*)malloc(SNAP_LEN);
    u_char load[SNAP_LEN];
    if(load==NULL)
    {
	printf(" HandleHttpRequestPayload malloc error!\n");
	return ;
    }
    strcpy(load,Payload);
    const char *split_char="\r\n";  //以\r\n分割
    const char *blank=" ";
    const char *maohao=":";
    char *Line="0";

    /* 第一行请求行的三个字段以空格分割，然后以\r\n换行 */
	// 第一行格式 HTTP/1.1 200 OK
    char firststr[10];
    char secondstr[SNAP_LEN];
    Line=strtok(load,blank);
    strcpy(firststr,Line); //获取版本
    printf("firststr=%s\n",firststr);

    Line=strtok(NULL,blank); //获取状态码
    strcpy(secondstr,Line); 
    int secondstrlen=strlen(secondstr);
    if(secondstrlen>200)
        return;
    printf("secondstr=%s\n",secondstr);

    int current_length=-1;
    current_length=strlen(firststr)+1+strlen(secondstr);
    printf("current_length=%d\tTcpPayloadLen=%d\n",current_length,TcpPayloadLen);
   

    if(current_length>=TcpPayloadLen)
    {
        //插入HTTP_request_data数据
        //mysqldb_insert_HttpRequest(&mysql,HttpRequestID,HttpID,HeadID,firststr);
        return;
    }
    Line=strtok(NULL,split_char);   //第一个\r\n前的字符串就是协议版本

    Line=strtok(NULL,"");
    Line+=1;//之前获得的Line有\r\n，要跳过这两个字符

    /*开始第2步，拆key/value*/
    char *KeyValue="0";
    KeyValue=strstr2(Line,"\r\n\r\n");
    char filename[20];
    char filename2[20];
    sprintf(filename2,"%d",HeadID); //将HeadID写入filename2
    sprintf(filename,"./files/%d",HeadID);
    if((fp=fopen(filename,"w"))==NULL){
        printf("Open File Error!\n");
        return;
    }
    //写入HTTP载荷文件 path为filename变量值
    fwrite(Payload,sizeof(Payload),TcpPayloadLen,fp);
    fclose(fp);
    KeyValue[strlen(KeyValue)]=='\0';
    printf("KeyValue:\n%s\n",KeyValue);
    /*key_value真正的个数-1*/
    int count_real_num=0;
    for(int i=0;i<strlen(KeyValue);i++){
        if(KeyValue[i]=='\n') //如果是换行，即已经遍历了一行Key/value,要进入下一个Key/value
            count_real_num++;
    }
    printf("count_real_num:%d\n",count_real_num);
    char OneLine[MAX_NUMBER_FOR_SPLIT][MAX_LEN_FOR_SPLIT] = {0}; //存放分割后的子字符串 
    char aroundColon[MAX_NUMBER_FOR_SPLIT][MAX_LEN_FOR_SPLIT]={0};//存放冒号两边的值
    int num=0;
    int num_aroundColon=0;
    char Content_Type[MAX_LEN_FOR_SPLIT];
    char Content_Len[MAX_LEN_FOR_SPLIT];

    HttpResponseID+=1; //初始化为0 开头自增 方便一点
    if(KeyValue!=NULL){
        SplitByMultiDelim(KeyValue,split_char,OneLine,&num);
        int i;
        printf("num:%d\n",num);
		int flag=0;
        for(i=0;i<count_real_num+1;i++){
            if(OneLine[i]=="")
                break;
            SplitByMultiDelim(OneLine[i],": ",aroundColon,&num_aroundColon);
            printf("Key=%s : Value=%s123\n",aroundColon[0],aroundColon[1]);

            if(!strcmp(aroundColon[0],"Content-Type")){ //只关心这个字段
				strcpy(Content_Type,aroundColon[1]);
				flag=1;
            }
	    else if(!strcmp(aroundColon[0],"Content-Length")){ //只关心这个字段
				strcpy(Content_Len,aroundColon[1]);
            }

            mysqldb_insert_HttpHeader(&mysql, i+1, HeadID,aroundColon[0], aroundColon[1]); //存放该HTTP payload中的所有Key/value字段,一行存一个
        }
		//处理正文,上面应该是发现两个连续的\r\n就break,目前不清楚具体分割字符串完以后是什么样子,调试时补上
		
		char fl[100];
		int off=0;
		const u_int8_t *now=ch;
		if(flag!=0)
		{
			while((*now)!=0)
			{
				if(!strncmp(now,"\r\n\r\n",4))
				{
					char *las;
					int isB=0;
					if(!strncmp(Content_Type,"text/html",sizeof("text/html")-1)) las=".html";
					else if(!strncmp(Content_Type,"text/plain",sizeof("text/plain")-1)) las=".txt";
					else if(!strncmp(Content_Type,"image/jpeg",sizeof("image/jpeg")-1)) las=".jpg",isB=1;
					else if(!strncmp(Content_Type,"image/gif",sizeof("image/gif")-1)) las=".gif",isB=1;
					else if(!strncmp(Content_Type,"video/quicktime",sizeof("video/quicktime")-1)) las=".mov",isB=1;
					else if(!strncmp(Content_Type,"Applicaton/vnd.ms-powserpoint",sizeof("Applicaton/vnd.ms-powserpoint")-1)) las=".ppt";
					else
					{
						printf("unknow Conten-Type %s",Content_Type);
						las="";
					}
					int clen=atoi(Content_Len);
					//fprintf(stderr,"clen:%d Content_Len:%s len:%d type:%s las:%s strncmp:%d\n",clen,Content_Len,TcpPayloadLen-off-4,Content_Type,las,strncmp(Content_Type,"text/html",sizeof("text/html")));
					sprintf(fl,"./resfile/%d%s",HeadID,las);
					if(isB)
					{
						if((fp=fopen(fl,"wb"))==NULL)
						{
							printf("open file error!\n");
							return;
						}
					}
					else 
					{
						if((fp=fopen(fl,"w"))==NULL)
						{
							printf("open file error!\n");
							return;
						}	
					}
				
					fwrite(now+4,sizeof(*now),TcpPayloadLen-off-4,fp);
					//fprintf(fp,"%s",now+4);
					fclose(fp);
					break;
				}
				now++;
				off++;
				//if((*now)==0) break;
				if(off==TcpPayloadLen) break;
			}

		}
			//存 HttpResponseID,HttpID,HeadID ,类型,http载荷路径,正文路径
		if(flag==0)
		{
			mysqldb_insert_HttpResponse(&mysql,HttpResponseID,HttpID,HeadID,"",filename,fl);
		}
		else
		{
			mysqldb_insert_HttpResponse(&mysql,HttpResponseID,HttpID,HeadID,Content_Type,filename,fl);
		}
    }
    //if(HttpResponseID>=110) exit(0);
    return;    
}
void SplitByMultiDelim(const char *src,const char *separator,char result[][MAX_LEN_FOR_SPLIT],int *num){
    if (src == NULL || strlen(src) == 0)
        return;
    if (separator == NULL || strlen(separator) == 0)
        return;
    const char *p;
    p=src;
    int count=0;//记录有多少个被分割字符串
    int len_separator=strlen(separator);//记录分割条件separator的长度
    int CurrentPostion=0;//当前位置，前面字符串被分割出来后之后会被清零
    //char result[MAX_NUMBER_FOR_SPLIT][MAX_LEN_FOR_SPLIT];//最大个数和字符串最大长度在宏定义
    while(1){
        if(count>=MAX_NUMBER_FOR_SPLIT){//出现这种情况就是要越界了
            printf("warning:the MAX_NUMBER_FOR_SPLIT is too small and this will make data dropped\n");
            break;
        }
        if(strncmp(p,separator,len_separator)==0||*p=='\0'){
            result[count][CurrentPostion]='\0';
            //*dest++ = result[count];
            CurrentPostion=0;
            p+=len_separator;
            count++;
            //printf("count:%d\n",count);
            //printf("result [count]:  %s\n",result[count-1]);
            //printf("*p:%d\n",*p);
            if(*p=='\0' || (int)*p<0)
            {
                //printf("*p:%d\n",*p);
                break; 
            }
        }
        result[count][CurrentPostion]=*p;
       
        p++;
        CurrentPostion++;
       
    }
    printf("final result [count]:  %s\n",result[count]);
    *num = count;
}
void handlepackets(u_char *useless,const struct pcap_pkthdr*pkthdr,const u_char*packet)
{
    /*packet headers struct*/
    const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
	const struct sniff_ip *ip;              /* The IP header */
	const struct sniff_tcp *tcp;            /* The TCP header */
	const char *payload;                    /* Packet payload */
    
    int size_ip;
	int size_tcp;
	int size_payload;

    /*
    拆包过程：
    1.解析链路层帧：捕获的帧长度，真正原长度
    2.解析IP层：版本，源IP，目的IP，传输层所用的协议:6代表TCP，17代表UDP
    3.解析传输层：源端口，目的端口
    4.解析Http载荷：分Request和Response。
        4.1：先拆Http第一行:GET URL 版本
        4.2: 拆key/value User-Agent:xxxx
    */

   /*拆链路层帧:要获取帧长度*/
   ethernet=(struct sniff_ethernet*)(packet);
   int caplen=pkthdr->caplen;   //捕获帧的长度 
   int len=pkthdr->len;         //真正原长度

   /*解析IP头,要获取源IP和目的IP，传输层使用的协议*/
   if(ntohs(ethernet->ether_type)==ETHERTYPE_IPV4){
       ip=(struct sniff_ip*)(packet+SIZE_ETHERNET);
       size_ip=IP_HL(ip)*4;
       if(size_ip<20){
            printf("Invalid IP header length: %u bytes\n", size_ip);
		    return;
       }
       char srcIP[30];
       char dstIP[30];
       inet_ntop(AF_INET,(void *)&ip->ip_src,srcIP,16);
       inet_ntop(AF_INET,(void *)&ip->ip_dst,dstIP,16);
       printf("From: %s\n", srcIP);
       printf("To: %s\n", dstIP);
       printf("ip->protocol:%u\n", ip->ip_p);
       /*如果是TCP*/
       if(ip->ip_p==6){
           tcp=(struct sniff_tcp*)(packet+SIZE_ETHERNET+size_ip);
           size_tcp=TH_OFF(tcp)*4;
           if(size_tcp<20){
               printf("Invalid TCP header length: %u bytes\n", size_tcp);
		       return;
           }
            payload=(u_char*)(packet+SIZE_ETHERNET+size_ip+size_tcp);
            size_payload=ntohs(ip->ip_len)-(size_ip+size_tcp);
            printf("Src port: %d\n", ntohs(tcp->th_sport));
	    printf("Dst port: %d\n", ntohs(tcp->th_dport));
            char *TCP="TCP";char *IP_version="Ipv4";
	    char *HttpType="";
            if(ntohs(tcp->th_dport)==80) //HttpRequest
            {
                HttpType="HttpRequest";
		printf("HttpRequest\n");
                mysqldb_insert_HttpTable(&mysql,HttpID,TcpID,HeadID,HttpType);
                HandleHttpRequestPayload(payload,size_payload);
                HttpID=HttpID+1;
            }
            else if(ntohs(tcp->th_sport)==80){
               	HttpType="HttpResponse";
		printf("HttpResponse\n");
                mysqldb_insert_HttpTable(&mysql,HttpID,TcpID,HeadID,HttpType);
                HandleHttpResponsePayload(payload,size_payload);
                HttpID=HttpID+1;
            }
	    /*插入保存所有包的表和TCP包的表中*/
            mysql_insert_HeadTable(&mysql,HeadID,TCP,IP_version,srcIP,dstIP,ntohs(tcp->th_sport),ntohs(tcp->th_dport),caplen,len,HttpType);
            mysql_insert_TcpTable(&mysql,TcpID,HeadID,ntohs(tcp->th_sport),ntohs(tcp->th_dport));

            HeadID++;
            TcpID++;
       }
       else if(ip->ip_p==17){ //UDP
            udp = (struct _udp_hdr*)(packet +SIZE_ETHERNET+size_ip);
            printf("udp_sport = %d\n", ntohs(udp->sport));  //UDP源端口
            printf("udp_dport = %d\n", ntohs(udp->dport));  //UDP目的端口
            /**********(pcaket + eth_len + ip_len + udp_len)就是UDP协议传输的正文数据***********/
            char *UDP="UDP";char *IP_version="Ipv4";
            mysql_insert_HeadTable(&mysql,HeadID,UDP,IP_version,inet_ntoa(ip->ip_src),inet_ntoa(ip->ip_dst),ntohs(udp->sport),ntohs(udp->dport),caplen,len,"");
            mysql_insert_UdpTable(&mysql,UdpID,HeadID,udp->sport,udp->dport);
            UdpID=UdpID+1;
            HeadID=HeadID+1;
       }
       else
        {
            char *el="Other Transport Protocol";char *IP_version="Ipv4";
            mysql_insert_HeadTable(&mysql,HeadID,el,IP_version,inet_ntoa(ip->ip_src),inet_ntoa(ip->ip_dst),-1,-1,caplen,len,"");
            HeadID=HeadID+1;
        }
   }
   else if(ntohs(ethernet->ether_type)==ETHERTYPE_IPV6){
        printf("I am IPv6\n");
        char *el="Unkown";char *IP_version="Ipv6";char *srcip="unknown";char *dstip="unknown";
        mysql_insert_HeadTable(&mysql,HeadID,el,IP_version,inet_ntoa(ip->ip_src),inet_ntoa(ip->ip_dst),-1,-1,caplen,len,"");
        HeadID=HeadID+1;
        num+=1;
   }
   

}
