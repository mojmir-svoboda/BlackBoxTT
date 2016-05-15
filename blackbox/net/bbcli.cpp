#include <platform_win.h>
#include <cstdlib>
#include <thread>
#include <asio.hpp>
#include <iostream>

struct Client
{
  asio::io_context & m_io;
  asio::ip::tcp::socket m_socket;

  Client (asio::io_context & io, asio::ip::tcp::resolver::results_type const & endpoints)
    : m_io(io)
    , m_socket(io)
  {
    DoConnect(endpoints);
  }

  /*void write (const chat_message & msg)
  {
    asio::post(io_context_,
        [this, msg]()
        {
          bool write_in_progress = !write_msgs_.empty();
          write_msgs_.push_back(msg);
          if (!write_in_progress)
          {
            do_write();
          }
        });
  }*/	

  void close ()
  {
    asio::post(m_io, [this]() { m_socket.close(); });
  }

  void DoConnect (asio::ip::tcp::resolver::results_type const & endpoints)
  {
    asio::async_connect(m_socket, endpoints,
        [this](std::error_code ec, asio::ip::tcp::endpoint)
        {
          if (!ec)
          {
            DoReadHeader();
          }
        });
  }

  void DoReadHeader ()
  {
#if 0
    asio::async_read(m_socket,
        asio::buffer(read_msg_.data(), chat_message::header_length),
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header())
          {
            do_read_body();
          }
          else
          {
            socket_.close();
          }
        });
#endif
  }

#if 0
  void do_read_body()
  {
    asio::async_read(socket_,
        asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            std::cout.write(read_msg_.body(), read_msg_.body_length());
            std::cout << "\n";
            do_read_header();
          }
          else
          {
            socket_.close();
          }
        });
  }

  void do_write()
  {
    asio::async_write(socket_,
        asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
        [this](std::error_code ec, std::size_t /*length*/)
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
            socket_.close();
          }
        });
  }
#endif
};

int main (int argc, char * argv[])
{
  try
  {
    if (argc != 3)
    {
      //std::cerr << "Usage: bbcli <host> <port>\n";
      return 1;
    }

    asio::io_context io;

		asio::ip::tcp::resolver resolver(io);
    auto endpoints = resolver.resolve(argv[1], argv[2]);
    Client c(io, endpoints);

    std::thread t([&io](){ io.run(); });

    //char line[chat_message::max_body_length + 1];
		char line[16384];
    while (std::cin.getline(line, 16384))
    {
      //chat_message msg;
      //msg.body_length(std::strlen(line));
      //std::memcpy(msg.body(), line, msg.body_length());
      //msg.encode_header();
      //c.write(msg);
    }

    c.close();
    t.join();
  }
  catch (std::exception & e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
