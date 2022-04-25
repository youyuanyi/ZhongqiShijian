#ifndef HANDLEPACKETS_H
#define HANDLEPACKETS_H
#include"head_file.h"
/*记录分割字符串时，每个字符串最大长度*/
#define MAX_LEN_FOR_SPLIT 9999

/*ip整型转换为点分十进制*/
char *InttoIpv4str(u_int32_t num);
/*处理Request包的Payload*/
void HandleHttpRequestPayload(const char *Payload,const int PayloadLen);
/*处理Response包的Payload*/
void HandleHttpResponsePayload(const char *Payload,const int PayloadLen);
/*打印payload信息*/
int print_payload(const u_int8_t *payload,int len);
/*打印成16进制*/
void print_hex_ascii_line(const u_int8_t *payload,int len,int offset);
/*按照/r/n或者空格分割字符串*/
void SplitByMultiDelim(const char *src,const char *separator,char result[][MAX_LEN_FOR_SPLIT],int *num);
char *strstr1(const char *s1,const char *delm);
char *strstr2(const char *s1,const char *delm);
void Regularize_HttpContent(const u_int8_t *Content,char WhatYouWant,char *Regularized_HttpContent,int length_Regularized_HttpContent);
/*pcap_loop的callback,负责拆包*/
void handlepackets(u_char *useless,const struct pcap_pkthdr*pkthdr,const u_char*packet);
#endif
