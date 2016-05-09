#pragma once
#include "encoder.h"
#include <Command.h>
#include "dictionary.h"
#include <asn_SET_OF.h>

namespace asn1 {

	inline size_t encode_dictionary (char * buff, size_t buff_ln, int type, DictPair const * dict_ptr, size_t dict_sz)
	{
		assert(dict_sz <= 64);

		Command_t command;
		command.present = Command_PR_dict;
		command.choice.dict.type = type;

		DictPair * asn1_dict_ptr = const_cast<DictPair *>(dict_ptr);
		::DictPair * proto_dict_ptr = reinterpret_cast<::DictPair * >(asn1_dict_ptr);
		::DictPair pairs[64];
		::DictPair * tmp[64];
		for (size_t i = 0; i < dict_sz; ++i)
		{
			::DictPair & dp = pairs[i];
			dp.value = dict_ptr[i].first;
			dp.name = mkOctetString(dict_ptr[i].second);
			tmp[i] = &pairs[i];
		}

		//	struct SET_OF(type) {
		//		type **array;
		//		int count;	/* Meaningful size */
		//		int size;	/* Allocated size */
		//		void (*free)(type *);
		command.choice.dict.dict.list.array = &tmp[0];
		command.choice.dict.dict.list.count = dict_sz;
		command.choice.dict.dict.list.size = 0;
		command.choice.dict.dict.list.free = nullptr;

		asn_enc_rval_t const ec = der_encode_to_buffer(nullptr, &asn_DEF_Command, &command, buff, buff_ln);
		if (ec.encoded == -1)
			return 0;
		return ec.encoded;
	}
}

