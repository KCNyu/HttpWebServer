/*================================================================
* Filename:main.cpp
* Author: KCN_yu
* Createtime:Sat 19 Jun 2021 02:57:51 PM CST
================================================================*/

#include <iostream>
#include "web_server.h"
using namespace std;

int main()
{
    WebServer serv(9999);
    serv.RunServer();
    return 0;
}