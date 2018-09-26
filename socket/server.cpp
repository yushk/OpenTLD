#include<iostream>
using namespace std;

//head files of Linux
#include<netinet/in.h>
#include<unistd.h>   //for fork and read
#include<sys/types.h>   //for socket
#include<sys/socket.h>  //for socket
#include<string.h> // for bzero
#include<arpa/inet.h>

int server()
{
    const unsigned short SERVERPORT = 53556;
    const int BACKLOG = 10; //10 个最大的连接数
    const int MAXSIZE = 1024;
    int sock, client_fd;
    sockaddr_in myAddr;
    sockaddr_in remoteAddr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    //create socket
    if( sock == -1)
    {
        cerr<<"socket create fail!"<<endl;
        return 1;
    }

    cout<<"sock :"<<sock<<endl;

    //bind
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(SERVERPORT);
    myAddr.sin_addr.s_addr = INADDR_ANY;
    bzero( &(myAddr.sin_zero), 8);
    if(bind(sock, (sockaddr*)(&myAddr), sizeof(sockaddr)) ==-1 )
    {
        cerr<<"bind error!"<<endl;
        return (1);
    }

    //listen
    if(listen(sock, BACKLOG) == -1)
    {
        cerr<<"listen error"<<endl;
        return(1);
    }

    while(true)
    {
        unsigned int sin_size = sizeof(sockaddr_in);
        if( (client_fd = accept(sock, (sockaddr*)(&remoteAddr), &sin_size)) ==-1 )
        {
            cerr<<"accept error!"<<endl;
            continue;
        }
        cout<<"Received a connection from "<<static_cast<char*>(inet_ntoa(remoteAddr.sin_addr) )<<endl;

        //子线程
        if(!fork() )
        {
            int rval;
            char buf[MAXSIZE];
            if( (rval = read(client_fd, buf, MAXSIZE) ) <0)
            {
                cout<<"Reading stream error!\n";
                continue;
            }
            cout<<buf<<endl;

            //向客户端发送信息
            const char* msg = "Hello, I am xiaojian. You are connected !";
            if( send(client_fd, const_cast<char*>(msg), strlen(msg), 0) == -1)
                cerr<<"send error!"<<endl;
            close(client_fd);
            return(0);
        }
    }
}       


int main()
{
    server();
}
