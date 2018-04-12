#include <iostream>
#include "./websocketpp/config/asio_no_tls.hpp"
#include "./websocketpp/server.hpp"
#define byte unsigned char
typedef websocketpp::server<websocketpp::config::asio> WebsocketServer;
typedef WebsocketServer::message_ptr message_ptr;
typedef WebsocketServer::connection_ptr connection_ptr;
//using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using namespace std;
class ws_server{
    public:
            //websocket
            static void OnOpen(WebsocketServer *server, websocketpp::connection_hdl hdl);
            static void OnClose(WebsocketServer *server, websocketpp::connection_hdl hdl);
            static void OnMessage(WebsocketServer *server, websocketpp::connection_hdl hdl, message_ptr msg);

            void init_ws_server();
            void ws_run(int port);
            //tcp_client
            void InitTcpClient_ConnectServer(string ip_addr,int tcp_server_port);
            static int reconnect_tcp_server(int &clientSocket, struct sockaddr_in serverAddr);
            static void *recvInfo_from_TcpServer(void* args);
            //TOOL                 
            static byte* intToBytes(int value,int byte_len);
            static int bytesToInt(byte* des, int byte_len);
            string GetTime();      
    private:
             WebsocketServer  server;
             static sockaddr_in serverAddr;
             static int clientSocket_fd;
             //tcp接收线程的id
             pthread_t recv_th_id;
};
        