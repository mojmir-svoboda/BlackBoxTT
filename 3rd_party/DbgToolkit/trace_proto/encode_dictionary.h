#pragma once
#include "encoder.h"
#include <Command.h>
#include <asn_SET_OF.h>

namespace asn1 {

	//@NOTE: interface is a bit imposed by asn.1 (i.e. non-const values)
	inline size_t encode_dictionary (char * buff, size_t buff_ln, int type, int64_t * values, char const * names[], size_t dict_sz)
	{
		assert(dict_sz <= 64);

		Command_t command;
		command.present = Command_PR_dict;	
		command.choice.dict.type = type;

		//@NOTE: data in asn.1 required layout (not sure why)
 		int64_t * asn1_values = const_cast<int64_t *>(values);
		int64_t ** asn1_value_ptrs = reinterpret_cast<int64_t **>(alloca(dict_sz * sizeof(int64_t *)));
		for(size_t i = 0; i < dict_sz; ++i)
			asn1_value_ptrs[i] = &values[i];
		command.choice.dict.value.list.array = asn1_value_ptrs;
		command.choice.dict.value.list.count = static_cast<int>(dict_sz);
		command.choice.dict.value.list.size = 0;
		command.choice.dict.value.list.free = nullptr;

		//@NOTE: data in asn.1 required layout (not sure why)
		OCTET_STRING_t * asn1_names = reinterpret_cast<OCTET_STRING_t *>(alloca(dict_sz * sizeof(OCTET_STRING_t)));
		OCTET_STRING_t ** asn1_name_ptrs = reinterpret_cast<OCTET_STRING_t **>(alloca(dict_sz * sizeof(OCTET_STRING_t *)));
		for (size_t i = 0; i < dict_sz; ++i)
			asn1_name_ptrs[i] = &asn1_names[i];

		for (size_t i = 0; i < dict_sz; ++i)
		{
			OCTET_STRING_t name = mkOctetString(names[i]);
			asn1_names[i] = name;
		}

		command.choice.dict.name.list.array = asn1_name_ptrs;
		command.choice.dict.name.list.count = static_cast<int>(dict_sz);
		command.choice.dict.name.list.size = 0;
		command.choice.dict.name.list.free = nullptr;

		asn_enc_rval_t const ec = der_encode_to_buffer(nullptr, &asn_DEF_Command, &command, buff, buff_ln);
		if (ec.encoded == -1)
			return 0;
		return ec.encoded;
	}
}

