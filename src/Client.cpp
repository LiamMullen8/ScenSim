#include "Client.hpp"

#include "simulation.pb.h"

Client::Client(boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints)
    : io_context_(io_context), socket_(io_context)
{
    do_connect(endpoints);
}

Client::~Client()
{
}

void Client::send_command(const std::string &command)
{
    simulation::Packet packet;
    packet.set_source(simulation::Packet::CLIENT);

    simulation::Entity *entity = packet.mutable_entity();
    entity->set_action(simulation::Entity_Action_CREATE);
    entity->set_type(simulation::Entity_Type_FIGHTER);
    entity->set_name(command);
    entity->set_id(1);

    std::string serialized_msg;
    packet.SerializeToString(&serialized_msg);

    boost::asio::post(
        socket_.get_executor(),
        [this, serialized_msg]()
        {
            write_msgs_.push_back(serialized_msg);
            if (write_msgs_.size() > 1)
            {
                return;
            }

            do_write();
        });
}

void Client::do_connect(const tcp::resolver::results_type &endpoints)
{
    boost::asio::async_connect(
        socket_, endpoints,
        [this](boost::system::error_code ec, tcp::endpoint)
        {
            if (!ec)
            {
                std::cout << "Successfully connected to server." << std::endl;
                do_read_header();
            }
            else
            {
                std::cerr << "Connection error: " << ec.message() << std::endl;
            }
        });
}

void Client::do_read_header()
{
    // Read in HEADER_SIZE ( 4 bytes ) to get packet body size
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(incoming_header_, HEADER_SIZE),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                uint32_t body_size_int;
                std::memcpy(&body_size_int, incoming_header_, HEADER_SIZE);
                uint32_t body_size = boost::asio::detail::socket_ops::network_to_host_long(body_size_int);

                do_read_body(body_size);
            }
            else
            {
                if(ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
                {
                    std::cerr << "Server connection closed" << std::endl;
                }
                else
                {
                    std::cerr << "Read body error: " << ec.message() << std::endl;
                }

                io_context_.post([this]()
                                 { socket_.close(); });
            }
        });
}

void Client::do_read_body(std::size_t body_size)
{
    if (body_size == 0)
    {
        do_read_header();
        return;
    }

    incoming_body_.resize(body_size);

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(incoming_body_),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                simulation::Packet packet;
                if (packet.ParseFromString(incoming_body_))
                {
                    if (packet.source() == simulation::Packet::SERVER)
                    {
                        std::cout << "Message received from Server: " << std::endl;
                        if(packet.has_world_state())
                        {
                            for(const auto& e : packet.world_state().entities())
                            {
                                std::cout << e.name() << std::endl;
                                std::cout << e.id() << std::endl;
                                std::cout << simulation::Entity::Type_Name(e.type()) << std::endl;
                            }
                        }
                    }
                }
                else
                {
                    std::cerr << "Failed to parse incoming protobuf message.\n";
                }

                do_read_header();
            }
            else
            {
                if(ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
                {
                    std::cerr << "Server connection closed" << std::endl;
                }
                else
                {
                    std::cerr << "Read body error: " << ec.message() << std::endl;
                }

                io_context_.post([this]()
                                 { socket_.close(); });
            }
        });
}

void Client::do_write()
{
    const std::string &serialized_data = write_msgs_.front();

    uint32_t body_size = static_cast<uint32_t>(serialized_data.size());
    uint32_t network_size = boost::asio::detail::socket_ops::host_to_network_long(body_size);

    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&network_size, HEADER_SIZE));
    buffers.push_back(boost::asio::buffer(serialized_data));

    boost::asio::async_write(
        socket_,
        buffers,
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
                io_context_.post([this]()
                                 { socket_.close(); });
            }
        });
}
