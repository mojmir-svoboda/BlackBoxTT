#pragma once
#include <cassert>

// expansion macro for enum value definition
#define FACT_ENUM_VALUE(name,assign) e_##name assign,

// expansion macro for enum to string conversion
#define FACT_ENUM_CASE(name,assign) #name,

// expansion macro for string to enum conversion
#define FACT_ENUM_STRCMP(name,assign) if (!strcmp(str,#name)) return e_##name;

/// generators for simple enum and enum_to_string
#define FACT_DECLARE_ENUM(EnumType,ENUM_DEF) \
	enum EnumType { \
		ENUM_DEF(FACT_ENUM_VALUE) \
	}; \

#define FACT_DECLARE_ENUM_STR(EnumType)			\
	extern char const * enum_to_string_##EnumType [];	\
	size_t enum_to_string_ln_##EnumType ()

#define FACT_DECLARE_ENUM_FROM_STR(EnumType,ENUM_DEF)			\
	EnumType enum_from_string_##EnumType (char const * str);

#define FACT_DEFINE_ENUM_FROM_STR(EnumType,ENUM_DEF)			\
	EnumType enum_from_string_##EnumType (char const * str)				\
	{															\
		ENUM_DEF(FACT_ENUM_STRCMP)								\
		assert("Undefined string to enum conversion!");			\
		return (EnumType)0;										\
	}

#define FACT_DEFINE_ENUM_STR(EnumType,ENUM_DEF)					\
	char const * enum_to_string_##EnumType [] =					\
	{															\
		ENUM_DEF(FACT_ENUM_CASE)								\
	}															\

#define FACT_DEFINE_ENUM_STR_LN(EnumType,ENUM_DEF)				\
	size_t enum_to_string_ln_##EnumType()						\
	{															\
		return sizeof(enum_to_string_##EnumType) / sizeof(*enum_to_string_##EnumType);	\
	}


#define FACT_DECLARE_ENUM_TO_STRING(EnumType,ENUM_DEF)			\
	char const * enumToString (EnumType e);						\

#define FACT_DEFINE_ENUM_TO_STRING(EnumType,ENUM_DEF)			\
	char const * enumToString (EnumType e)						\
	{															\
		return enum_to_string_##EnumType[e];					\
	}


// use this only if you dont need from string conversion
#define FACT_DEFINE_ENUM_TO_STRING_METHOD(EnumType, ENUM_DEF)	\
	static char const * enumToString(EnumType e)			\
	{														\
		FACT_DEFINE_ENUM_STR(EnumType, ENUM_DEF);			\
		return enum_to_string_##EnumType[e];				\
	}

