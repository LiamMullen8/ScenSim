#include "Session.hpp"
#include "Server.hpp"

Session::Session(tcp::socket socket, Server &server)
    : socket_(std::move(socket)), server_(server)
{
    std::cout << "Session created for new client." << std::endl;
}

void Session::start()
{
    do_read_header();
}

void Session::send_packet(const std::string &serialized_packet)
{
    auto self(shared_from_this());
    boost::asio::post(
        socket_.get_executor(), [this, self, serialized_packet]()
        {
            write_msgs_.push_back(serialized_packet);

            if(write_msgs_.size() > 1)
            {
                return;
            }

            do_write(); 
        });
}

void Session::do_read_header()
{
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(incoming_header_, HEADER_SIZE),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                uint32_t body_size_int;
                std::memcpy(&body_size_int, incoming_header_, HEADER_SIZE);
                uint32_t body_size = boost::asio::detail::socket_ops::network_to_host_long(body_size);

                do_read_body(body_size);
            }
            else
            {
                if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
                {
                    std::cerr << "Session read error: " << ec.message() << std::endl;
                }

                std::cerr << "Client disconnected." << std::endl;
                server_.remove_session(self);
            }
        });
}

void Session::do_read_body(std::size_t body_size)
{
    if (body_size == 0 || body_size > 1024 * 1024)
    {
        std::cerr << "Invalid message size detected: " << body_size << "\n";
        server_.remove_session(shared_from_this());
        return;
    }

    auto self(shared_from_this());
    incoming_body_.resize(body_size);

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(incoming_body_),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                // Deserialize the protobuf message
                simulation::NetPacket packet;
                if (packet.ParseFromString(incoming_body_))
                {
                    // Delegate the structured packet to the server logic
                    server_.handle_client_update(packet);
                }
                else
                {
                    std::cerr << "Failed to parse protobuf message from client.\n";
                }

                do_read_header(); // Continue the read loop
            }
            else
            {
                std::cerr << "Session read body error: " << ec.message() << std::endl;
                server_.remove_session(self);
            }
        });
}

void Session::do_write()
{
    auto self(shared_from_this());

    const std::string &serialized_data = write_msgs_.front();

    uint32_t body_size = static_cast<uint32_t>(serialized_data.size());
    uint32_t network_size = boost::asio::detail::socket_ops::host_to_network_long(body_size);

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(write_msgs_.front()),
        [this, self](boost::system::error_code ec, std::size_t)
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
                server_.remove_session(self);
            }
        });
}