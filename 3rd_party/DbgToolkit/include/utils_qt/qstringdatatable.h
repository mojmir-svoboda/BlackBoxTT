#pragma once
#include <QString>
#include "membuffer.h"
#include "virtualallocbuffer.h"
#include "utils_from_qt5.h"
#include <LogScopeType.h>
#include <OCTET_STRING.h>

struct stroffs_t {
	stroffs_t () : m_idx(0) { }
	stroffs_t (uint32_t i) : m_idx(i) { }
  uint32_t m_idx;
};

using strop_t = std::tuple<bool, stroffs_t>;

//QArrayData
//typedef QTypedArrayData<ushort> QStringData;
struct QStringDataTable : MemBuffer<VirtualAllocBuffer>
{
	void clear() { Reset(); }

	strop_t AddQStringDataFromUtf8 (char const * buff, size_t sz, size_t indent, LogScopeType scptype)
	{
		Q_ASSERT(indent >= 0);
		enum { qstring_sz = sizeof(QArrayData) };

		int const scope_tag_sz = scptype == LogScopeType_scopeNone ? 0 : 2;
		wchar_t * u16_tmp = static_cast<wchar_t *>(alloca((sz + indent + scope_tag_sz) * sizeof(wchar_t)));
		for (size_t i = 0; i < indent; ++i)
			u16_tmp[i] = L' ';
		if (scptype == LogScopeType_scopeEntry)
		{
			u16_tmp[indent] = L'{';
			u16_tmp[indent + 1] = L' ';
		}
		else if (scptype == LogScopeType_scopeExit)
		{
			u16_tmp[indent] = L'}';
			u16_tmp[indent + 1] = L' ';
		}

		size_t const u16_sz = convertToUnicodeFromUtf8(buff, sz, u16_tmp + indent + scope_tag_sz, sz);
		size_t const mem_need = qstring_sz + (u16_sz + indent + scope_tag_sz) * sizeof(wchar_t);
		if (char * mem = static_cast<char *>(Malloc(mem_need)))
		{
			QArrayData * data = new (mem) QArrayData Q_STATIC_STRING_DATA_HEADER_INITIALIZER(int(u16_sz + indent + scope_tag_sz));
			memcpy(mem + qstring_sz, u16_tmp, (u16_sz + indent + scope_tag_sz) * sizeof(char16_t));
			return std::make_tuple(true, stroffs_t(std::distance(begin(), mem)));
		}
		return std::make_tuple(false, stroffs_t(0));
	}

	strop_t AddQStringDataFromUtf8 (char const * buff, size_t sz)
	{
		return AddQStringDataFromUtf8(buff, sz, 0, LogScopeType_scopeNone);
	}

	strop_t AddQStringDataFromOCTET_STRING (OCTET_STRING_t const & octstr)
	{
		char const * b = (char const *)octstr.buf;
		int const sz = octstr.size;
		return AddQStringDataFromUtf8(b, sz);
	}

	strop_t AddQStringDataFromOCTET_STRING (OCTET_STRING_t const & octstr, size_t indent, LogScopeType scptype)
	{
		char const * b = (char const *)octstr.buf;
		int const sz = octstr.size;
		return AddQStringDataFromUtf8(b, sz, indent, scptype);
	}

	strop_t AddQStringData (char16_t const * buff, size_t sz)
	{
		enum { qstring_sz = sizeof(QArrayData) };

		if (char * mem = static_cast<char *>(Malloc(qstring_sz + sz)))
		{
			QArrayData * data = new (mem) QArrayData Q_STATIC_STRING_DATA_HEADER_INITIALIZER(int(sz));
			memcpy(mem + qstring_sz, buff, sz);
			return std::make_tuple(true, stroffs_t(std::distance(begin(), mem)));
		}
		return std::make_tuple(false, stroffs_t(0));
	}

	QString GetQString (uint32_t offs) const
	{
		//assert size
		char const * raw_data = begin() + offs;
		const QArrayData * data = reinterpret_cast<const QArrayData *>(raw_data);
		assert(data->ref.isStatic());
    QStringData * qdata = const_cast<QStringData *>(static_cast<const QStringData*>(data));
		QStringDataPtr holder = { qdata };
		const QString qstring_literal_temp(holder);
    return qstring_literal_temp;
	}
	QString GetQString (stroffs_t offs) const
	{
		return GetQString(offs.m_idx);
	}
};

