#include"include/ws_server.h"
int main()
{
    ws_server WS;
    WS.InitTcpClient_ConnectServer("127.0.0.1",22222);
    WS.init_ws_server();
    //Listen on port 44444
    WS.ws_run(44444);

    return 0;
}