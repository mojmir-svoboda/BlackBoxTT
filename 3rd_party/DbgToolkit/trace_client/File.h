#pragma once
#include <cstdio>

namespace trace {

	struct File
	{
		FILE * m_file { nullptr };

		bool Open (char const * fname)
		{
			m_file = std::fopen(fname, "w");
			return m_file != nullptr;
		}

		void Close ()
		{
			if (m_file)
			{
				std::fclose(m_file);
				m_file = nullptr;
			}
		}

		bool Write (char const * buff, size_t ln)
		{
			if (m_file)
				return ln == std::fwrite(buff, sizeof(char), ln, m_file);
			return false;
		}

		void WriteEndl ()
		{
			std::fputc('\n', m_file);
		}

		void SetBuffered (bool on)
		{
			if (on)
				;//setvbuf(m_file, nullptr, _IOFBF, 0);
			else
				setvbuf(m_file, nullptr, _IONBF, 0);
		}

		void Flush ()
		{
			if (m_file)
				std::fflush(m_file);
		}
	};
}

