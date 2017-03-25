#pragma once
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <fstream>

inline bool mk_mmap_file (std::string const & fname, size_t sz, boost::interprocess::file_mapping & fmap)
{
  boost::interprocess::file_mapping::remove(fname.c_str());
  std::filebuf fbuf;
  fbuf.open(fname, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
  fbuf.pubseekoff(sz - 1, std::ios_base::beg);
  fbuf.sputc(0);
  fmap = boost::interprocess::file_mapping(fname.c_str(), boost::interprocess::read_write);
	return true;
}

inline bool mmap_region (boost::interprocess::file_mapping & fmap, boost::interprocess::mapped_region & freg)
{
  freg = boost::interprocess::mapped_region(fmap, boost::interprocess::read_write);
	return freg.get_size() > 0;
}
