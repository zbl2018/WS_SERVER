//#include "stdafx.h"
#include"./include/ws_server.h"

//=============静态变量初始化=================
int ws_server::clientSocket_fd =0 ;
sockaddr_in ws_server::serverAddr = {AF_INET,22222,inet_addr("127.0.0.1")};
////////////////////////////////////////////

void ws_server::OnOpen(WebsocketServer *server, websocketpp::connection_hdl hdl)
{
    cout << "have client connected" << endl;
}

void ws_server::OnClose(WebsocketServer *server, websocketpp::connection_hdl hdl)
{
    cout << "have client disconnected" << endl;
}

void ws_server::OnMessage(WebsocketServer *server, websocketpp::connection_hdl hdl, message_ptr msg)
{
    byte head[6];
    char send_info[1024]={0};
    string strMsg = msg->get_payload();
    int send_status=-1;
    //connection_ptr con = server->get_con_from_hdl(hdl);
    byte temp[6];
    // deal infomatiom of head 
    //默认小车与服务器的tcp连接 id 为 7
    memcpy(head,intToBytes(7,2),2);
    memcpy(head+2,intToBytes(strMsg.length()+1,4),4);
    cout<<"id:"<<bytesToInt(head,2)<<endl;    
    cout<<"headlen:"<<bytesToInt(head+2,4)<<endl;
    //merge info
    memcpy(send_info,head,6);
    memcpy(send_info+6,strMsg.c_str(),strMsg.length()+1);

    // memcpy(temp,send_info,6);
    // cout<<"temp:"<<bytesToInt(temp+2,4)<<endl;
    cout<<"ssss:"<<send_info+6<<endl;
    send_status=send(clientSocket_fd, send_info,strMsg.length()+7, MSG_NOSIGNAL);

    if(send_status>0){
        printf("send information to tcp server successfully!\n");
    }
    else{
        printf("fail to send information to tcp server!\n");
    }
    cout << strMsg << endl;
    string strRespon = "receive: ";
    strRespon.append(strMsg);
    server->send(hdl, strRespon, websocketpp::frame::opcode::text);
}

void ws_server::init_ws_server(){
    // Set logging settings
    server.set_access_channels(websocketpp::log::alevel::all);
    server.clear_access_channels(websocketpp::log::alevel::frame_payload);
    // Initialize ASIO
    server.init_asio();

    // Register our open handler
    server.set_open_handler(bind(&OnOpen, &server, ::_1));

    // Register our close handler
    server.set_close_handler(bind(&OnClose, &server, _1));

    // Register our message handler
    server.set_message_handler(bind(&OnMessage, &server, _1, _2));
}
void ws_server::ws_run(int port){

    server.listen(port);
    //Start the server accept loop
    server.start_accept();
    printf("start listen port: %d in websocket\n",port);
    //Start the ASIO io_service run loop
    server.run();
}
void ws_server::InitTcpClient_ConnectServer(string ip_addr,int tcp_server_port){

    serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(tcp_server_port);
	serverAddr.sin_addr.s_addr = inet_addr(ip_addr.c_str());
	//链接远程主机
	reconnect_tcp_server(clientSocket_fd,serverAddr);
    //与tcp_server连接成功，创建接收线程
    int res = pthread_create(&recv_th_id,NULL,recvInfo_from_TcpServer,NULL);
    if(res==0){
        printf("create recv pthread successfully!\n");
    }
    else {
         printf("fail to create recv pthread!\n");
    }

}
int ws_server::reconnect_tcp_server(int &clientSocket_fd, struct sockaddr_in serverAddr){
	int is_connect=-1;
	//创建句柄	
	if((clientSocket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return 1;
	}
	//链接	
	is_connect=connect(clientSocket_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(is_connect<0)
		{
			perror("connect");
			printf("wait for connect server...\n");
		}	
	while(is_connect<0){
		is_connect=connect(clientSocket_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	}
	printf("connect with destination host...\n");
	return 1;
}
void * ws_server::recvInfo_from_TcpServer(void* args){
		char recvbuf[1024];
		unsigned char head[6];
		int ret_len;
	while(1){
		ret_len=recv(clientSocket_fd,head,6,MSG_WAITALL);
		int data_len = bytesToInt(head+2,4);
		ret_len = recv(clientSocket_fd,recvbuf,data_len ,MSG_WAITALL);
		printf("length:%d\n",data_len);
		printf("iDataNum:%s\n",recvbuf);
		if(ret_len==-1||ret_len==0)
		{
			printf("oooo2\n");
			close(clientSocket_fd);
			//sleep(5);
			reconnect_tcp_server(clientSocket_fd,serverAddr);
			continue;
		}		
		//printf("链接断开3\n");
		//recvbuf[iDataNum] = '\0';
	}
	
}



byte* ws_server::intToBytes(int value,int byte_len){
    if(byte_len>4||byte_len<1)
    {
       cout<<"byte_len overflow!"<<endl;
        return 0;           
    }
        byte *des = new byte[byte_len];  
        des[0] = (byte) (value & 0xff);  // 低位(右边)的8个bit位    
        if(byte_len==1)
            return des;  
        des[1] = (byte) ((value >> 8) & 0xff); //第二个8 bit位 
        if(byte_len==2)
            return des;   
        des[2] = (byte) ((value >> 16) & 0xff); //第三个 8 bit位
        if(byte_len==3)
            return des;    
        /** 
         * (byte)((value >> 24) & 0xFF); 
         * value向右移动24位, 然后和0xFF也就是(11111111)进行与运算 
         * 在内存中生成一个与 value 同类型的值 
         * 然后把这个值强制转换成byte类型, 再赋值给一个byte类型的变量 des[3] 
         */  
        des[3] = (byte) ((value >> 24) & 0xff); //第4个 8 bit位  
        return des;  
    }  
  
    /** 
     * 将上面转成的byte数组转换成int原始值  
     * @param des 
     * @param offset 
     * @return 
     */  
int ws_server::bytesToInt(byte* des, int byte_len){
        if(byte_len>4||byte_len<1)
        {
        cout<<"byte_len overflow!"<<endl;
        return 0;           
        }  
        int value;    
        switch(byte_len){
            case 4:{ 
                //四位
                value = (int)((des[0] & 0xff)  
                | ((des[1] & 0xff) << 8)  
                | ((des[2] & 0xff) << 16)  
                | (des[3] & 0xff) << 24);
                break;
            }    
            case 3:{
                //三位
                value = (int) ((des[0] & 0xff)  
                | ((des[1] & 0xff) << 8)  
                | ((des[2] & 0xff) << 16));
                break;
            }
        case 2:{
                //二位
                value = (int) ((des[0] & 0xff)  
                | ((des[1] & 0xff) << 8));
                break;   
        }
        case 1:{
                //一位
                value = (int) ((des[0] & 0xff));
                break; 
        }      
        }       
        return value;  
    }  
string ws_server::GetTime(){
        time_t t = time( 0 );   
        char tmpBuf[255];   
        strftime(tmpBuf, 255, "%Y-%m-%d %H:%M:%S", localtime(&t)); //format date and time.
        return tmpBuf; 
}
