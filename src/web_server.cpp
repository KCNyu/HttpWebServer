/*================================================================
* Filename:web_server.cpp
* Author: KCN_yu
* Createtime:Sat 19 Jun 2021 02:18:04 PM CST
================================================================*/

#include "web_server.h"
using namespace std;

WebServer::WebServer(int pt):port(pt){
    epfd = epoll_create(MAXSIZE);
    if (epfd == -1) {
        cerr << "epoll_create error" << endl;
        exit(1);
    }
    InitListenfd();
}
void WebServer::RunServer(){
    // ask the fd in epoll
    while(true) {
        int ret = epoll_wait(epfd, events, MAXSIZE, 0);
        if(ret == -1) {
            cerr << "epoll_wait error" << endl;
        }

        // Traverse the fd which has changes
        for(int i = 0; i < ret; i++) {
            struct epoll_event *pev = &events[i];
            // only solve the read events
            if(!(pev->events & EPOLLIN)) {
                continue;
            }
            if(pev->data.fd == lfd) {
                AcceptFd();
            } else {
                printf("====== BEFORE DO READ RET %d ======\n", ret);
                ReadFd(pev->data.fd);
                printf("======= AFTER DO READ RET %d ======\n", ret);
            }
        }
    }
}
// get new connect
void WebServer::AcceptFd(){
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int cfd = accept(lfd , reinterpret_cast<struct sockaddr*>(&client), &len);
    if(cfd == -1) {
        cerr << "accept error" << endl;
        exit(1);
    }

    // print info of client
    char ip[64] = {0};
    printf("New Client IP: %s, Port: %d, cfd = %d\n",
           inet_ntop(AF_INET, &client.sin_addr.s_addr, ip, sizeof(ip)),
           ntohs(client.sin_port), cfd);

    // set cfd nonblock
    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);

    // add cfd in epoll
    struct epoll_event ev;
    ev.data.fd = cfd;
    // set ET mode
    ev.events = EPOLLIN | EPOLLET;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if(ret == -1) {
        cerr << "epoll_ctl add cfd error" << endl;
        exit(1);
    }
}
void WebServer::InitListenfd(){

    // Init listenfd socket
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1){
        cerr << "socket error" << endl;
        exit(1);
    }

    // set listenfd
    struct sockaddr_in serv;
    bzero(&serv,sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    // reuse the port
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // listenfd bind to local IP and port
    int ret = bind(lfd, reinterpret_cast<struct sockaddr*>(&serv), sizeof(serv));
    if(ret == -1) {
        cerr << "bind error" << endl;
        exit(1);
    }

    // set listen
    ret = listen(lfd, 64);
    if(ret == -1) {
        cerr << "listen error" << endl;
        exit(1);
    }

    // add listenfd to epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if(ret == -1) {
        cerr << "epoll_ctl add lfd error" << endl;
        exit(1);
    }
}
int WebServer::GetLine(int sock, char* buf, int size){
    int i = 0;
    char c = '\0';
    int n;
    while ((i < size - 1) && (c != '\n')) {
        n = recv(sock, &c, 1, 0);
        if (n > 0) {
            if (c == '\r') {
                n = recv(sock, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n')) {
                    recv(sock, &c, 1, 0);
                } else {
                    c = '\n';
                }
            }
            buf[i] = c;
            i++;
        } else {
            c = '\n';
        }
    }
    buf[i] = '\0';

    return i;
}
void WebServer::ReadFd(int cfd){
    char line[1024] = {0};

    int len = GetLine(cfd, line, sizeof(line));
    if(len == 0) {
        cout << "Client Disconnect..." << endl;
        Disconnect(cfd);
    } else {
        cout << "====== HEAD ======" << endl;
        cout << "GET DATA: " << line << endl;

        while (1) {
            char buf[1024] = {0};
            len = GetLine(cfd, buf, sizeof(buf));
			if (buf[0] == '\n') {
				break;
			} else if (len == -1)
				break;
        }
        cout << "====== END ======" << endl;
    }

    if(strncasecmp("GET", line, 3) == 0) {
        HttpRequest(cfd, line);
        Disconnect(cfd);
    }
}
void WebServer::Disconnect(int cfd){
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
    if(ret == -1) {
        cerr << "epoll_ctl del cfd error" << endl;
        exit(1);
    }
    close(cfd);
}
void WebServer::HttpRequest(int cfd, const char* request){

}
