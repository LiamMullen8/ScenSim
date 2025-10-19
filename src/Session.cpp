#include "Session.hpp"
#include "Server.hpp"

Session::Session(tcp::socket socket, Server &server)
    : socket_(std::move(socket)), server_(server)
{
    std::cout << "Session created for new client." << std::endl;
}

Session::~Session()
{
}

void Session::start()
{
    do_read();
}

void Session::send_message(const std::string &message)
{
    auto self(shared_from_this());
    boost::asio::post(socket_.get_executor(), [this, self, message]()
    {
        write_msgs_.push_back(message + "\n");

        if(write_msgs_.size() > 1)
        {
            return;
        }

        do_write(); 
    });
}

void Session::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::string message(data_, length);

                server_.handle_client_update(message);

                do_read();
            }
            else if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
            {
                std::cout << "Client disconnected," << std::endl;
                server_.remove_session(self);
            }
            else
            {
                std::cerr << "Session read error: " << ec.message() << std::endl;
                server_.remove_session(self);
            }
        }
    );
}

void Session::do_write()
{
    auto self(shared_from_this());
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(write_msgs_.front()),
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if(!ec)
            {
                write_msgs_.pop_front();
                if(!write_msgs_.empty())
                {
                    do_write();
                }
            }
            else
            {
                std::cerr << "Write error: " << ec.message() << std::endl;
                server_.remove_session(self);
            }
        }
    );
}