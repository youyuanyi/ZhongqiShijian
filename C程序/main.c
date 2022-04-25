#include"handlePackets.h"
#include"mysqldb.h"
int main()
{
    int pid;
    // 2）在后台运行  
    if( pid=fork() ){ // 父进程  
        exit(0); //结束父进程，子进程继续  
    }else if(pid< 0){ // 出错  
        perror("fork");  
        exit(EXIT_FAILURE);  
    }  
      
    // 3）脱离控制终端、登录会话和进程组  
    setsid();    
      
    // 4）禁止进程重新打开控制终端  
    if( pid=fork() ){ // 父进程  
        exit(0);      // 结束第一子进程，第二子进程继续（第二子进程不再是会话组长）   
    }else if(pid< 0){ // 出错  
        perror("fork");  
        exit(EXIT_FAILURE);  
    }  

    freopen("tmp","w",stdout);
    mysql_init(&mysql);
    mysql_connect(&mysql);
    char ebuf[PCAP_ERRBUF_SIZE];
    char *pcap_file="traffic.pcap";
    pcap_t *p=pcap_open_offline(pcap_file,ebuf);
    printf("%s\n",ebuf);
    struct pcap_pkthdr pkthdr;
    int num=0;
    while(1)
    {
        const u_char *pktstr=pcap_next(p,&pkthdr);
	printf("%d\n",num);
        if(!pktstr)
        {
            printf("pcap file parse over! pkstr is %s \n",pktstr);
            printf("%d\n",num);
            pcap_close(p);
            exit(1);
        }
        pcap_loop(p,-1,handlepackets, NULL); 
        num=num+1;
    }
    return 0;
}
