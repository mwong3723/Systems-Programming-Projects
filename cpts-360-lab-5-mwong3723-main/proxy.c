#include <stdio.h>
#include <string.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_def = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connectionHeader = "Connection: close\r\n";
static const char *proxyHeader = "Proxy-Connection: close\r\n";
static const char *userKey = "User-Agent: ";
static const char *connectionKey = "Connection: ";
static const char *proxyConnection = "Proxy-Connection: ";
static const char *hostKey = "Host: ";

int keyIdentifer(char *header, const char *key);
void parseLink(char *link, char *hostname, unsigned int *port, char *query);
int otherHeader(char *header);
void createHeader(rio_t rio, char *headers, char *hostname, char *query);
void forwardRequest(int clientfd, rio_t rio, char *link);
void handleRequest(int clientfd);

int main(int argc, char **argv) {
    int listenFD, conFD;
    char hostName[MAXLINE], port[MAXLINE];
    socklen_t clientLen;
    struct sockaddr_storage clientAddress;

    if (argc != 2) { // check for port input if not return error
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }

    if ((listenFD = open_listenfd(argv[1])) < 0) {
        printf("Unable to open port %s\n", argv[1]);
        return 1;
    }

    while (1) { //loops till user ctrl+c
        clientLen = sizeof(clientAddress);
        conFD = Accept(listenFD, (SA *)&clientAddress, &clientLen);
        Getnameinfo((SA *) &clientAddress, clientLen, hostName, MAXLINE, port, MAXLINE, 0);
        fprintf(stdout,"Accepted connection from (%s, %s)\n", hostName, port);
        handleRequest(conFD);
        Close(conFD);
    }

    return 0;
}

int keyIdentifer(char *header, const char *key) {
    int len = strlen(key);
    return strncmp(header, key, len) == 0;
}

void parseLink(char *link, char *hostname, unsigned int *port, char *query) {
    char *ptr;
    int strLength = strlen("http://");
    if(strncmp(link, "http://", strLength) == 0){
        ptr = link + 7; 
    }
    else {
        ptr = link;
    }

    while ((*ptr != ':') && (*ptr != '/')) {
        *hostname = *ptr;
        ptr++;
        hostname++;
    }
    //addded null charcter to make string and terminate
    *hostname = '\0'; 
    if (*ptr == ':') {
        sscanf(ptr + 1, "%d%s", port, query);
    } 
    else {
        *port = 80;  
        strcpy(query, ptr);
    }
}

int otherHeader(char *header) {
    return !(keyIdentifer(header, userKey) || keyIdentifer(header, connectionKey) || keyIdentifer(header, proxyConnection));
}

void createHeader (rio_t rio, char *headers, char *hostname, char *query) {
    char buffer[MAXLINE], hostHeader[MAXLINE] = "";
    int flag1 = 1, flag2 = 1;
    sprintf(headers, "GET %s HTTP/1.0\r\n", query);
    strcat(headers, connectionHeader);
    strcat(headers, proxyHeader);
    if (strcmp(rio.rio_bufptr, "") != 0) { // checks for headers
        while (Rio_readlineb(&rio, buffer, MAXLINE) != 0) {
            if (strcmp(buffer, "\r\n") == 0) {
                break;
            } else if (keyIdentifer(buffer, hostKey)) {
                strcat(headers, buffer);
                flag1 = 0;
            } else if (keyIdentifer(buffer, userKey)) {
                strcat(headers, buffer);
                flag2 = 0;
            } else if (otherHeader(buffer)) {
                strcat(headers, buffer);
            }
        }
    }


    if (flag1) { // host
        sprintf(hostHeader, "Host: %s\r\n", hostname);
        strcat(headers, hostHeader);
    }
    if (flag2) { // agent
        strcat(headers, user_agent_def);
    }
    strcat(headers, "\r\n");
}

void forwardRequest(int clientfd, rio_t rio, char *link) {
    int serverFD;
    char hostName[MAXLINE], portString[8], query[MAXLINE];
    char headers[MAXLINE] = "", response[MAXLINE] = "";
    unsigned int port;
    size_t length;
    parseLink(link, hostName, &port, query);
    sprintf(portString, "%d", port);
    if ((serverFD = open_clientfd(hostName, portString)) < 0) {
        return;
    }
    createHeader(rio, headers, hostName, query);
    Rio_readinitb(&rio, serverFD);
    Rio_writen(serverFD, headers, strlen(headers));
    while((length = Rio_readlineb(&rio, response, MAXLINE)) != 0){
        Rio_writen(clientfd, response, length); 
    }
    Close(serverFD);
}


void handleRequest(int clientfd) {
    char buffer[MAXLINE], method[8], link[MAXLINE], version[8];
    rio_t rio;
    Rio_readinitb(&rio, clientfd);
    memset(rio.rio_buf, 0, 8192); 
    if (!Rio_readlineb(&rio, buffer, MAXLINE)) {
        return;
    }
    printf("%s", buffer);
    sscanf(buffer, "%s %s %s", method, link, version);
    if (strcasecmp(method, "GET") != 0) {
        printf("Proxy can only handle HTTP GET requests\n");
        return;
    }
    forwardRequest(clientfd, rio, link);
}