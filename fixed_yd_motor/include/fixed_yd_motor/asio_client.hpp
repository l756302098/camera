/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-05-06 09:55:08
 * @LastEditors: li
 * @LastEditTime: 2021-05-08 13:48:27
 */
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/system/error_code.hpp>
#include <boost/bind/bind.hpp>
#include <deque>
#include <mutex>

using namespace boost::asio;
using namespace std;

class client
{
    typedef client this_type;
    typedef ip::tcp::acceptor acceptor_type;
    typedef ip::tcp::endpoint endpoint_type;
    typedef ip::tcp::socket socket_type;
    typedef ip::address address_type;
    typedef boost::shared_ptr<socket_type> sock_ptr;
    typedef vector<char> buffer_type;

private:
    io_service m_io;
    buffer_type m_buf;
    endpoint_type m_ep;
    sock_ptr sock;
    std::mutex mtx;
    std::deque<vector<unsigned char>> receive_msg;
    bool socket_status;
    
public:
    bool is_open(){
        //return sock->is_open();
        return socket_status;
    }
    vector<unsigned char> queue_pop(){
        if(!receive_msg.empty() && mtx.try_lock()){
            auto msg = receive_msg.front();
            receive_msg.pop_front();
            mtx.unlock();
            return msg;
        }
        return {};
    }
    client(): m_buf(64, '/n'), m_ep(address_type::from_string("192.168.1.7"), 8234)
    {    start();    }

    client(string ip,int port): m_buf(64, '/n'), m_ep(address_type::from_string(ip), port),socket_status(false)
    {    
        std::cout << "client start " << std::endl;
        printf("%x == %x \n",'/n',m_buf[0]);
        std::cout << "ip:" << ip << " port:" << port << std::endl; 
        start();    
    }

    void run()
    {   
        try
        {
            boost::asio::io_service::work work(m_io);
            m_io.run();   
        }
        catch(const std::exception& e)
        {
            std::cout << "run error:"<< e.what() << '\n';
            m_io.stop();
        }
    }

    void start()
    {
        try
        {
            sock_ptr sock_t(new socket_type(m_io));
            sock = std::move(sock_t);
            sock->async_connect(m_ep, boost::bind(&this_type::conn_handler, this, boost::asio::placeholders::error, sock));
        }
        catch(const std::exception& e)
        {
            std::cout << "start error:" << e.what() << '\n';
            sock->close();
        }
    }

    void conn_handler(const boost::system::error_code&ec, sock_ptr sock)
    {
        if (ec)
        {
            std::cout << "connect error:" << ec << std::endl;
            socket_status = false;
            return;
        }
        socket_status = true;
        cout<<"Receive from "<<sock->remote_endpoint().address()<<": "<<endl;
        sock->async_read_some(buffer(m_buf), boost::bind(&client::read_handler, this, boost::asio::placeholders::error, sock));
    }

    void read_handler(const boost::system::error_code&ec, sock_ptr sock)
    { 
        if (ec)
        {
            std::cout << "read error:" << ec << std::endl;
            return;
        }    
        auto mutable_buffer = buffer(m_buf);
        sock->async_read_some(mutable_buffer, boost::bind(&client::read_handler, this, boost::asio::placeholders::error, sock));
        std::size_t s = boost::asio::buffer_size(mutable_buffer);
        unsigned char* p = boost::asio::buffer_cast<unsigned char*>(mutable_buffer);
        std::vector<unsigned char> response;
        for (size_t i = 0; i < s; i++)
        {
            unsigned char d = *p;
            if(d == '/n') break;
            response.push_back(d);
            p++;
        }
        //printf("\n");
        mtx.lock();
        while (receive_msg.size()>10)
        {
            receive_msg.pop_front();
        }
        receive_msg.push_back(response);
        mtx.unlock();
        //int index = m_buf.size() - 1;
        //cout<< "read data start:" <<&m_buf[0] << " end:" << m_buf[index] << endl;
        /*
        auto mutable_buffer = buffer(m_buf);
        sock->async_read_some(mutable_buffer,[&](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            std::cout << "length:" << length << std::endl; 
          }
        });
        */
    }

    void send(std::string &message){
        if(is_open())
            sock->write_some(boost::asio::buffer(message));
    }

    void send_bytes(std::vector<unsigned char> data){
        // for (size_t i = 0; i < data.size(); i++)
        // {
        //     printf(" %x ",data[i]);
        // }
        // printf("\n");
        // std::cout << "isopen:" << is_open() << std::endl;
        if(is_open())
        {
            boost::system::error_code ec;
            sock->write_some(boost::asio::buffer(data),ec);
            if(ec)
                std::cout << "ignored_error:" << ec.message() << std::endl;
        }
    }
};