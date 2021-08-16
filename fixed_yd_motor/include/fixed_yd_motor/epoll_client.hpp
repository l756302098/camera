/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-05-06 09:55:08
 * @LastEditors: li
 * @LastEditTime: 2021-05-08 13:48:27
 */
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cassert>

#include <string>
#include <iostream>
#include <memory>
#include <functional>
#include <thread>


// actually no need to implement a tcp client using epoll
namespace transport{

static const uint32_t kEpollWaitTime = 10; // epoll wait timeout 10 ms
static const uint32_t kMaxEvents = 100;    // epoll wait return max size

typedef struct Packet {
public:
    Packet()
        : msg { "" } {}
    Packet(const std::string& msg)
        : msg { msg } {}
    Packet(int fd, const std::string& msg)
        : fd(fd),
          msg(msg) {}

    int fd { -1 };     // meaning socket
    std::string msg;   // real binary content
} Packet;

typedef std::shared_ptr<Packet> PacketPtr;

// callback when packet received
using callback_recv_t = std::function<void(const PacketPtr& data)>;

// base class of EpollTcpServer, focus on Start(), Stop(), SendData(), RegisterOnRecvCallback()...
class EpollTcpBase {
public:
    EpollTcpBase()                                     = default;
    EpollTcpBase(const EpollTcpBase& other)            = delete;
    EpollTcpBase& operator=(const EpollTcpBase& other) = delete;
    EpollTcpBase(EpollTcpBase&& other)                 = delete;
    EpollTcpBase& operator=(EpollTcpBase&& other)      = delete;
    virtual ~EpollTcpBase()                            = default; 

public:
    virtual bool Start() = 0;
    virtual bool Stop()  = 0;
    virtual int32_t SendData(const PacketPtr& data) = 0;
    virtual void RegisterOnRecvCallback(callback_recv_t callback) = 0;
    virtual void UnRegisterOnRecvCallback() = 0;
};

using ETBase = EpollTcpBase;

typedef std::shared_ptr<ETBase> ETBasePtr;

// the implementation of Epoll Tcp client
class EpollTcpClient : public ETBase {
public:
    EpollTcpClient()                                       = default;
    EpollTcpClient(const EpollTcpClient& other)            = delete;
    EpollTcpClient& operator=(const EpollTcpClient& other) = delete;
    EpollTcpClient(EpollTcpClient&& other)                 = delete;
    EpollTcpClient& operator=(EpollTcpClient&& other)      = delete;
    ~EpollTcpClient() override;

    // the server ip and port
    EpollTcpClient(const std::string& server_ip, uint16_t server_port);

public:
    // start tcp client
    bool Start() override;
    // stop tcp client
    bool Stop() override;
    // send packet
    int32_t SendData(const PacketPtr& data) override;
    // register a callback when packet received
    void RegisterOnRecvCallback(callback_recv_t callback) override;
    void UnRegisterOnRecvCallback() override;

protected:
    // create epoll instance using epoll_create and return a fd of epoll
    int32_t CreateEpoll();
    // create a socket fd using api socket()
    int32_t CreateSocket();
    // connect to server
    int32_t Connect(int32_t listenfd);
    // add/modify/remove a item(socket/fd) in epoll instance(rbtree), for this example, just add a socket to epoll rbtree
    int32_t UpdateEpollEvents(int efd, int op, int fd, int events);
    // handle tcp socket readable event(read())
    void OnSocketRead(int32_t fd);
    // handle tcp socket writeable event(write())
    void OnSocketWrite(int32_t fd);
    // one loop per thread, call epoll_wait and return ready socket(readable,writeable,error...)
    void EpollLoop();


private:
    std::string server_ip_; // tcp server ip
    uint16_t server_port_ { 0 }; // tcp server port
    int32_t handle_ { -1 }; // client fd
    int32_t efd_ { -1 }; // epoll fd
    std::shared_ptr<std::thread> th_loop_ { nullptr }; // one loop per thread(call epoll_wait in loop)
    bool loop_flag_ { true }; // if loop_flag_ is false, then exit the epoll loop
    callback_recv_t recv_callback_ { nullptr }; // callback when received
};

using ETClient = EpollTcpClient;

typedef std::shared_ptr<ETClient> ETClientPtr;
}