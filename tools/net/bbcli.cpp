#include <platform_win.h>
#include <cstdlib>
#include <thread>
#include <asio.hpp>
#include <iostream>
#include <bbproto/Header.h>
#include <bbproto/encoder.h>
#include "CommandLine.h"

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

//		 asio::post(m_io,
//				 [this, buff, ln]()
//				 {
//					 bool write_in_progress = !write_msgs_.empty();
//					 write_msgs_.push_back(msg);
//					 if (!write_in_progress)
//					 {
//						 do_write();
//					 }
//				 });
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

//int main (int argc, char * argv[])
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	try
	{
		bb::CommandLine cli;
		cli.Init();

		if (cli.m_quit)
			return 0;

		asio::io_context io;
		asio::ip::tcp::resolver resolver(io);
		std::string p = std::to_string(cli.Port());
		auto endpoints = resolver.resolve(cli.Host().c_str(), p.c_str());
		Client c(io, endpoints);

		std::thread t([&io](){ io.run(); });

		std::string const cmd = cli.Cmd();
		if (!cmd.empty())
		{
			char msg[16384];
			size_t const cmd_ln = cli.Cmd().size();
			if (cmd_ln > 0)
			{
				size_t const n = bb::encode_bbcmd(msg, 16384, cli.Cmd().c_str(), cmd_ln);
				c.Write(msg, n);
			}
		}
		else
		{
			wchar_t line[4096];
			// interactive shell
			while (std::wcin.getline(line, 4096))
			{
				char msg[16384];
				size_t const line_ln = wcslen(line);
				if (line_ln > 0)
				{
					size_t const n = bb::encode_bbcmd(msg, 16384, line, line_ln);
					c.Write(msg, n);
				}
			}
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
