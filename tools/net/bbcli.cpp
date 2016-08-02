#include <platform_win.h>
#include <cstdlib>
#include <thread>
#include <asio.hpp>
#include <iostream>
#include <bbproto/Header.h>
#include <bbproto/encoder.h>

// test
#include <bbproto/encode_bb32wm.h>

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

  void Write (char const * buff, size_t ln)
  {
		asio::async_write(m_socket,
		asio::buffer(buff, ln),
			[this](std::error_code ec, std::size_t /*length*/)
			{
				if (!ec)
				{
				}
				else
				{
					m_socket.close();
				}
			});

//     asio::post(m_io,
//         [this, buff, ln]()
//         {
//           bool write_in_progress = !write_msgs_.empty();
//           write_msgs_.push_back(msg);
//           if (!write_in_progress)
//           {
//             do_write();
//           }
//         });
  }

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
		char const * argv1 = argc == 3 ? argv[1] : "127.0.0.1";
		char const * argv2 = argc == 3 ? argv[2] : "13199";
    asio::io_context io;

		asio::ip::tcp::resolver resolver(io);
    auto endpoints = resolver.resolve(argv1, argv2);
    Client c(io, endpoints);

    std::thread t([&io](){ io.run(); });


    //char line[chat_message::max_body_length + 1];
		wchar_t line[1024];
    while (std::wcin.getline(line, 1024))
    {
			char msg[1024];
			size_t const n = bb::encode_bbcmd(msg, 1024, line);
			c.Write(msg, n);
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
