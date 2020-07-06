/**
 * Simplest Librtmp Receive
 * 最简单的使用libRTMP接收流媒体的例子
 *
 * 雷霄骅，张晖
 * leixiaohua1020@126.com
 * zhanghuicuc@gmail.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序用于接收RTMP流媒体并在本地保存成FLV格式的文件。
 * This program can receive rtmp live stream and save it as local flv file.
 */
#include <stdio.h>
#include "rtmp_sys.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>
 
int InitSockets()
{
#ifdef WIN32
    WORD version;
    WSADATA wsaData;
    version = MAKEWORD(1, 1);
    return (WSAStartup(version, &wsaData) == 0);
#endif
}
 
void CleanupSockets()
{
#ifdef WIN32
    WSACleanup();
#endif
}
 
int main(int argc, char* argv[])
{
    InitSockets();
     
    double duration=-1;
    int nRead;
    //is live stream ?
    int bLiveStream=1;              
     
     
    int bufsize=1024*1024*10;           
    char *buf=(char*)malloc(bufsize);
    memset(buf,0,bufsize);
    long countbufsize=0;
     
    FILE *fp=fopen("receive.H265","wb");
    if (!fp){
        printf("Open File Error.\n");
        CleanupSockets();
        return -1;
    }
    
    char* rtmpUrl; 
    if(argc >= 2) {
    	rtmpUrl = "rtmp://192.168.3.60:1935/live/live_10080_01_00";
    }
    else {

    	rtmpUrl = "rtmp://192.168.3.60:1935/live/live_888888_01_00";
    }
    //const char* rtmpUrl = "rtmp://192.168.3.210:1935/live/live_10080_03_00";
    /* set log level */
    //RTMP_LogLevel loglvl=RTMP_LOGDEBUG;
    //RTMP_LogSetLevel(loglvl);
 
    RTMP *rtmp=RTMP_Alloc();
    RTMP_Init(rtmp);
    //set connection timeout,default 30s
    rtmp->Link.timeout=10;   
    // HKS's live URL
    //const char* rtmpUrl = "rtmp://192.168.3.210:1935/live/live_10080_03_00";
    //const char* rtmpUrl = "rtmp://192.168.3.210:1935/live/live_888888_01_00";
    printf("%s\n", rtmpUrl);
    if(!RTMP_SetupURL(rtmp,(char*)rtmpUrl))
    {
        printf("SetupURL Err\n");
        RTMP_Free(rtmp);
        CleanupSockets();
        return -1;
    }  
    if (bLiveStream){
        rtmp->Link.lFlags|=RTMP_LF_LIVE;
    }
     
    //1hour
    RTMP_SetBufferMS(rtmp, 3600*1000);      
     
    if(!RTMP_Connect(rtmp,NULL)){
        printf("Connect Err\n");
        RTMP_Free(rtmp);
        CleanupSockets();
        return -1;
    }
 
    if(!RTMP_ConnectStream(rtmp,0)){
        printf("ConnectStream Err\n");
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        CleanupSockets();
        return -1;
    }
    while(nRead=RTMP_Read(rtmp,buf,bufsize)){
        fwrite(buf,1,nRead,fp);
 
        countbufsize+=nRead;
        printf("Receive: %5dByte, Total: %5.2fkB\n",nRead,countbufsize*1.0/1024);
    }
 
    if(fp)
        fclose(fp);
 
    if(buf){
        free(buf);
    }
 
    if(rtmp){
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        CleanupSockets();
        rtmp=NULL;
    }   
    return 0;
}
