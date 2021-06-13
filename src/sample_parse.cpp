#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>
#include <cstring>
#include <iostream>
#include <time.h>
extern "C"{
    #include "parse.h"
    #include "pcsa_net.h"
}
#define size 8192
#define MAXBUF 2048

typedef struct sockaddr SA;
using namespace std;

static const char* DAY_NAMES[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char* MONTH_NAMES[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

char *RFC1123_DateTimeNow(){
    const int RFC1123_TIME_LEN = 29;
    time_t t;
    struct tm tm;
    char* buf = (char *)malloc(RFC1123_TIME_LEN+1);
    time(&t);
    gmtime_r(&t, &tm);
    strftime(buf, RFC1123_TIME_LEN+1, "%a, %d %b %Y %H:%M:%S GMT", &tm);
    memcpy(buf, DAY_NAMES[tm.tm_wday], 3);
    memcpy(buf, MONTH_NAMES[tm.tm_mon], 3);
    return buf;
}

/* to return the MIME type */
char* check_MIME(char* string){
    if (strcmp(string, "html")==0) return "text/html";
    if (strcmp(string, "plain")==0) return "text/plain";
    if (strcmp(string, "css")==0) return "text/css";
    if (strcmp(string, "javascript")==0||strcmp(string, "js")==0) return "text/javascript";
    if (strcmp(string, "mp4")==0) return "video/mp4";
    if (strcmp(string, "png")==0) return "image/png";
    if (strcmp(string, "jpg")==0||strcmp(string, "jpeg")==0) return "image/jpg";
    if (strcmp(string, "gif")==0) return "image/gif";
    if (strcmp(string, "mpeg")==0) return "audio/mpeg";
    return "";
}

/* create a correct response with returned data type and size */
char* create_reponse_request(char* buf, int number_status, char* status, unsigned long size_of_packet, char* type){
    sprintf(buf,
        "HTTP/1.1 %d %s\r\n"
        "Date: %s\r\n"
        "Server: ICWS\r\n"
        "Connection: close\r\n"
        "Content-type: %s\r\n"
        "Content-length: %lu\r\n\r\n", number_status, status,  RFC1123_DateTimeNow(), type, size_of_packet
    );
    return buf;
}

/* create a incorrect response with no data type and no size */
char* create_error_request(char *buf, int number_status, char* status){
    sprintf(buf,
        "HTTP/1.1 %d %s\r\n"
        "Server: ICWS\r\n"
        "Connection: close\r\n",
        number_status, status
    );
    return buf;
}

/* do the internet thing. Send http over. */
void serve_http(int connFd, char* url_folder) { //char* dest
    //Read from the file the sample
    int index;
    char buf[size];
    int readRet = read(connFd,buf,size);
    //Parse the buffer to the parse function. You will need to pass the socket fd and the buffer would need to
    //be read from that fd
    Request *request = parse(buf,readRet,connFd);
    //Just printing everything
    /*printf("Http Method %s\n",request->http_method);
    printf("Http Version %s\n",request->http_version);
    printf("Http Uri %s\n",request->http_uri);*/
    struct stat stats;
    char *type;
    char *after_fullstop;
    char url[255];
    strcpy(url, url_folder);
    strcat(url,request->http_uri);
    //printf("url %s\n", url);
    /* open connection */
    int inputFd = open(url, O_RDONLY);
    if (inputFd < 0){
        printf("input failed\n");
        //write_all(connFd, create_error_request(500, "Internal Server Error"), strlen(buf));
    }
    after_fullstop = strrchr(url, '.');
    after_fullstop++;
    /* compare HTTP Methods */
    if (strcasecmp(request->http_method, "GET") == 0){
        if(stat(url, &stats) >= 0){
            type = "";
            type = check_MIME(after_fullstop);
            create_reponse_request(buf, 200, "OK", stats.st_size, type);
            printf("buf = %s\n",buf);
            write_all(connFd, buf, strlen(buf));
            ssize_t numRead;
            while ((numRead = read(inputFd, buf, MAXBUF)) > 0) {
                /* What about short counts? */
                write_all(connFd, buf, numRead);
            }
        }
        close(inputFd);
    }
    else if(strcasecmp(request->http_method, "HEAD") == 0){
        if(stat(url, &stats) >= 0){
            type = "";
            type = check_MIME(after_fullstop);
            create_reponse_request(buf, 200, "OK", stats.st_size, type);
            write_all(connFd, buf, strlen(buf));
        }
        close(inputFd);
    }
    else {
        create_error_request(buf, 501, "Method Unimplemented");
        write_all(connFd, buf, strlen(buf));
        close(inputFd);
    }
    free(request->headers);
    free(request);
}

/* main, input of port and root handled here. */
int main(int argc, char **argv){
    char* port;
    char* root_folder;
    /* Basic Error Checking */
    if(argc != 5){
        printf("not enough arguments\n");
        return 0;
    }
    if (!strcmp(argv[1],"--port")){
        for(int i = 0; i < strlen(argv[2]); i++){
            if (!isdigit(argv[2][i])){
                printf("not a valid port: please enter a valid number port\n");
                return 0;
            }
        }
        port = argv[2];
    }
    else{
        if (!strcmp(argv[1],"port") || !strcmp(argv[1],"-port")){
            printf("invalid port command: did you mean --port?\n");
            return 0;
        }
        printf("invalid port command: try to use port\n");
        return 0;
    }

    if (!strcmp(argv[3],"--root")){
        root_folder = argv[4];
    }
    else{
        if (!strcmp(argv[1],"root") || !strcmp(argv[1],"-root")){
            printf("invalid root command: did you mean --root?\n");
            return 0;
        }
        printf("invalid root command: try to use --root\n");
        return 0;
    }
    /* Socket connecting and interacting starts here */
    int listen_fd = open_listenfd(port);
    while (1) {
        struct sockaddr_storage clientAddr;
        socklen_t clientLen = sizeof(struct sockaddr_storage);

        int connFd = accept(listen_fd, (SA *) &clientAddr, &clientLen);
        if (connFd < 0) { fprintf(stderr, "Failed to accept\n"); continue; }

        char hostBuf[MAXBUF], svcBuf[MAXBUF];
        if (getnameinfo((SA *) &clientAddr, clientLen, 
                        hostBuf, MAXBUF, svcBuf, MAXBUF, 0)==0) 
            printf("Connection from %s:%s\n", hostBuf, svcBuf);
        else
            printf("Connection from ?UNKNOWN?\n");
                
        serve_http(connFd, root_folder);
        close(connFd);
    }
}