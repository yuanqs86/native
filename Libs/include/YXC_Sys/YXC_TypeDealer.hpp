/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_TYPE_DEALER_HPP__
#define __INC_YXC_BASE_TYPE_DEALER_HPP__

namespace YXCLib
{
	template <typename T>
	struct TypeDealer
	{
		static const bool HAS_POINTER = false;
	};
}

#define DECLARE_TYPE_DEALER(_type, _hasPointer)				\
	namespace YXCLib											\
	{														\
		template <>											\
		struct TypeDealer<_type>							\
		{													\
			static const bool HAS_POINTER = _hasPointer;	\
		};													\
	}

DECLARE_TYPE_DEALER(bool, true);
DECLARE_TYPE_DEALER(char, true);
DECLARE_TYPE_DEALER(unsigned char, true);
DECLARE_TYPE_DEALER(short, true);
DECLARE_TYPE_DEALER(unsigned short, true);
DECLARE_TYPE_DEALER(int, true);
DECLARE_TYPE_DEALER(unsigned int, true);
DECLARE_TYPE_DEALER(long long, true);
DECLARE_TYPE_DEALER(unsigned long long, true);
DECLARE_TYPE_DEALER(float, true);
DECLARE_TYPE_DEALER(double, true);
DECLARE_TYPE_DEALER(unsigned long, true);
DECLARE_TYPE_DEALER(long, true);

namespace YXCLib
{
	template <bool IsOK>
	struct StaticCheckType
	{
	};

	template <>
	struct StaticCheckType<true>
	{
		enum { Ok = 1 };
	};

	template <>
	struct StaticCheckType<false>
	{
		enum { Error = 1 };
	};
}

#define STATIC_CHECK_EXPRESSION(_expression) YXCLib::StaticCheckType<(bool)_expression>::Ok
#define IS_ONLY_DATA_TYPE(_type) YXCLib::TypeDealer<_type>::HAS_POINTER

#endif /* __INC_YXC_BASE_TYPE_DEALER_HPP__ */
