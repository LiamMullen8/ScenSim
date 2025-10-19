#include "Client.hpp"

Client::Client(boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints)
    : io_context_(io_context), socket_(io_context)
{
    do_connect(endpoints);
}

Client::~Client()
{
}

void Client::send(const std::string &msg)
{
    boost::asio::post(socket_.get_executor(),
                      [this, msg]()
                      {
                          write_msgs_.push_back(msg);
                          if (write_msgs_.size() > 1)
                          {
                              return;
                          }

                          do_write();
                      });
}

void Client::do_connect(const tcp::resolver::results_type &endpoints)
{
    boost::asio::async_connect(socket_, endpoints,
                               [this](boost::system::error_code ec, tcp::endpoint)
                               {
                                   if (!ec)
                                   {
                                       std::cout << "Successfully connected to server." << std::endl;
                                       do_read();
                                   }
                                   else
                                   {
                                       std::cerr << "Connection error: " << ec.message() << std::endl;
                                   }
                               });
}

void Client::do_read()
{
    boost::asio::async_read_until(socket_,
                                  boost::asio::dynamic_buffer(read_buffer_), '\n',
                                  [this](boost::system::error_code ec, std::size_t length)
                                  {
                                      if (!ec)
                                      {
                                          std::string message(read_buffer_.begin(), read_buffer_.begin() + length);
                                          read_buffer_.erase(read_buffer_.begin(), read_buffer_.begin() + length);

                                          std::cout << "Received: " << message << std::endl;

                                          do_read();
                                      }
                                      else if (ec == boost::asio::error::eof)
                                      {
                                          std::cout << "Server closed connection." << std::endl;
                                      }
                                      else
                                      {
                                          std::cerr << "Read error: " << ec.message() << std::endl;
                                      }
                                  });
}

void Client::do_write()
{
    boost::asio::async_write(socket_,
                             boost::asio::buffer(write_msgs_.front()),
                             [this](boost::system::error_code ec, std::size_t)
                             {
                                 if (!ec)
                                 {
                                     write_msgs_.pop_front();
                                     if (!write_msgs_.empty())
                                     {
                                         do_write();
                                     }
                                 }
                                 else
                                 {
                                     std::cerr << "Write error: " << ec.message() << std::endl;
                                     socket_.close();
                                 }
                             });
}
