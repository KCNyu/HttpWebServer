#ifndef WEB_SERVER_H

#define WEB_SERVER_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define MAXSIZE 1024

class WebServer
{
private:
    int port;
    int epfd;
    int lfd;
    struct epoll_event events[MAXSIZE];

public:
    WebServer(int port);
    void RunServer();
    void InitListenfd();
    void AcceptFd();
    void ReadFd(int cfd);
    void Disconnect(int cfd);
    void HttpRequest(int cfd, const char* request);
    int GetLine(int sock, char *buf, int size);
    virtual ~WebServer() = default;
};

#endif /* WEB_SERVER_H */