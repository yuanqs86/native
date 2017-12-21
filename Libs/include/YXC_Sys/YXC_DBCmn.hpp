/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_DB_COMMON_HPP__
#define __INC_YXC_SYS_BASE_DB_COMMON_HPP__

#include <YXC_Sys/YXC_DBResult.hpp>
#include <YXC_Sys/YXC_Database.h>

#define _EU_DB_RESULT_ALLOC(_Type, _Result, _Member)											\
	_Type* p##_Member = _Result.AllocT<_Type>();												\
	_YXC_CHECK_REPORT_RET(p##_Member != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for %s failed."),		\
		YC(#_Member))

#define _EU_DB_RESULT_ALLOC_ARRAY(_Type, _Result, _Member, _Len)									\
	_Type* pArr##_Member = (_Type*)_Result.Alloc(_Len * sizeof(_Type));								\
	_YXC_CHECK_REPORT_RET(pArr##_Member != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for '%s' failed."),	\
		YC(#_Member))

#define _EU_DB_RESULT_ALLOC_STRING(_Type, _Result, _Member, _Len)									\
	_Type* psz##_Member = (_Type*)_Result.Alloc(sizeof(_Type) * (_Len + 1));						\
	_YXC_CHECK_REPORT_RET(psz##_Member != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for '%s' failed."),	\
		YC(#_Member))

#define _EU_DB_RESULT_COPY_BSTRINGA(_Result, _Member, _Str, _Len)									\
	YXC_BStringA bs##_Member;																		\
	bs##_Member.pStr = (char*)_Result.Alloc(YXC_STRING_SIZEA(_Len));									\
	_YXC_CHECK_REPORT_RET(bs##_Member.pStr != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for '%s' failed."),	\
		YC(#_Member));																					\
	memcpy(bs##_Member.pStr, _Str, YXC_STRING_SIZEA(_Len));											\
	bs##_Member.stCchStr = _Len;

#define _EU_DB_RESULT_COPY_BSTRINGW(_Result, _Member, _Str, _Len)									\
	YXC_BStringW bs##_Member;																		\
	bs##_Member.pStr = (wchar_t*)_Result.Alloc(YXC_STRING_SIZEW(_Len));								\
	_YXC_CHECK_REPORT_RET(bs##_Member.pStr != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for '%s' failed."),	\
		YC(#_Member));																					\
	memcpy(bs##_Member.pStr, _Str, YXC_STRING_SIZEW(_Len));											\
	bs##_Member.stCchStr = _Len;

#define _EU_DB_RESULT_ALLOC2(_Type, _Result, _ExistedMember)										\
	_ExistedMember = _Result.AllocT<_Type>();														\
	_YXC_CHECK_REPORT_RET(_ExistedMember != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for '%s' failed."),	\
		YC(#_ExistedMember))

#define _EU_DB_RESULT_ALLOC_ARRAY2(_Type, _Result, _ExistedMember, _Len)							\
	_ExistedMember = (_Type*)_Result.Alloc(_Len * sizeof(_Type));									\
	_YXC_CHECK_REPORT_RET(_ExistedMember != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for '%s' failed."),	\
		YC(#_ExistedMember))

#define _EU_DB_RESULT_ALLOC_STRING2(_Type, _Result, _ExistedMember, _Len)							\
	_ExistedMember = (_Type*)_Result.Alloc(sizeof(_Type) * (_Len + 1));								\
	_YXC_CHECK_REPORT_RET(_ExistedMember != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for '%s' failed."),	\
		YC(#_ExistedMember))

#define _EU_DB_RESULT_COPY_BSTRINGA2(_Result, _EMember, _Str, _Len)									\
	_EMember.pStr = (char*)_Result.Alloc(YXC_STRING_SIZEA(_Len));									\
	_YXC_CHECK_REPORT_RET(_EMember.pStr != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for '%s' failed."),	\
		YC(#_EMember);																				\
	memcpy(_EMember.pStr, _Str, YXC_STRING_SIZEA(_Len));												\
	_EMember.stCchStr = _Len;

#define _EU_DB_RESULT_COPY_BSTRINGW2(_Result, _EMember, _Str, _Len)									\
	_EMember.pStr = (wchar_t*)_Result.Alloc(YXC_STRING_SIZEW(_Len));									\
	_YXC_CHECK_REPORT_RET(_EMember.pStr != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for %s failed."),		\
		YC(#_EMember));																				\
	memcpy(_EMember.pStr, _Str, YXC_STRING_SIZEW(_Len));												\
	_EMember.stCchStr = _Len;

#define _EU_DB_RESULT_COPY_BSTRINGA_NULLABLE(_Result, _EMember, _Str, _Len, _NotNull)		\
	do {																					\
		if (_NotNull) {																		\
			_EU_DB_RESULT_COPY_BSTRINGA2(_Result, _EMember, _Str, _Len);					\
		} else {																			\
			_EMember.pStr = NULL, _EMember.stCchStr = 0;									\
		}																					\
	} while (0)

#define _EU_DB_RESULT_COPY_BSTRINGA_N(_Result, _EMember, _NValue)								\
	_EU_DB_RESULT_COPY_BSTRINGA_NULLABLE(_Result, _EMember, _NValue.tVal.pStr, _NValue.tVal.stCchStr, (!_NValue.bIsNull))

#define _EU_DB_RESULT_COPY_BSTRINGW_NULLABLE(_Result, _EMember, _Str, _Len, _NotNull)		\
	do {																					\
		if (_NotNull) {																		\
			_EU_DB_RESULT_COPY_BSTRINGW2(_Result, _EMember, _Str, _Len);					\
		} else {																			\
			_EMember.pStr = NULL, _EMember.stCchStr = 0;									\
		}																					\
	} while (0)

#define _EU_DB_RESULT_COPY_BSTRINGW_N(_Result, _EMember, _NValue)								\
	_EU_DB_RESULT_COPY_BSTRINGW_NULLABLE(_Result, _EMember, _NValue.tVal.pStr, _NValue.tVal.stCchStr, (!_NValue.bIsNull))

#define _EU_DB_CHECK_BIND_GOTO(rc, _Member)	_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Bind param '" YC(#_Member) YC("' failed"))

namespace YXCLib
{
	YXC_API(YXC_Status) _CreateDBResult(ysize_t stBlockBuf, _DBResult** ppResult);

	YXC_API(void) _FreeDBResult(_DBResult* result);

	inline YXC_DBRTColBindInfo _CreateDBBindInfo(ysize_t stDataLen, void* pData, ybyte_t byDataType)
	{
		YXC_DBRTColBindInfo bindInfo = { stDataLen, pData, byDataType };
		return bindInfo;
	}

	template <typename T>
	struct _Nullable
	{
		ybool_t bIsNull;
		T tVal;
	};

	template <typename T>
	inline T _DBFetchCellValue(const YXC_DBRTCell*& pCells)
	{
		T retVal = *(T*)pCells->pCellData;
		++pCells;
		return retVal;
	}

	template <typename T>
	inline _Nullable<T> _DBFetchCellNValue(const YXC_DBRTCell*& pCells)
	{
		_Nullable<T> retVal = { pCells->bIsNullVal };
		if (!retVal.bIsNull)
		{
			retVal.tVal = *(T*)pCells->pCellData;
		}
		++pCells;
		return retVal;
	}

	inline YXC_BStringA _DBFetchCellStringA(const YXC_DBRTCell*& pCells)
	{
		YXC_BStringA ret = { (char*)pCells->pCellData, pCells->uDataLen / sizeof(char) };
		++pCells;
		return ret;
	}

	inline YXC_BStringW _DBFetchCellStringW(const YXC_DBRTCell*& pCells)
	{
		YXC_BStringW ret = { (wchar_t*)pCells->pCellData, pCells->uDataLen / sizeof(wchar_t) };
		++pCells;
		return ret;
	}

	inline _Nullable<YXC_BStringA> _DBFetchCellNStringA(const YXC_DBRTCell*& pCells)
	{
		_Nullable<YXC_BStringA> retVal = { pCells->bIsNullVal };
		if (!retVal.bIsNull)
		{
			retVal.tVal.pStr = (char*)pCells->pCellData;
			retVal.tVal.stCchStr = pCells->uDataLen / sizeof(char);
		}
		++pCells;
		return retVal;
	}

	inline _Nullable<YXC_BStringW> _DBFetchCellNStringW(const YXC_DBRTCell*& pCells)
	{
		_Nullable<YXC_BStringW> retVal = { pCells->bIsNullVal };
		if (!retVal.bIsNull)
		{
			retVal.tVal.pStr = (wchar_t*)pCells->pCellData;
			retVal.tVal.stCchStr = pCells->uDataLen / sizeof(wchar_t);
		}
		++pCells;
		return retVal;
	}

	template <typename T>
	class CCResultAlloc
	{
	public:
		typedef ysize_t size_type;
		typedef yintptr_t difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;

		template <typename U>
		class rebind
		{
		public:
			typedef CCResultAlloc<U> other;
		};

		CCResultAlloc(_DBResult* pResult, size_type stMaxSize)
		{
			this->pResult = pResult;
			this->stMaxSize = stMaxSize;
		}

		CCResultAlloc(const CCResultAlloc& rhs)
		{
			this->pResult = rhs.pResult;
			this->stMaxSize = rhs.stMaxSize;
		}

		CCResultAlloc& operator =(const CCResultAlloc& rhs)
		{
			this->pResult = rhs.pResult;
			this->stMaxSize = rhs.stMaxSize;
		}

		template <typename U>
		CCResultAlloc(const CCResultAlloc<U>& rhs) // for stl vector / deque
		{
			this->pResult = rhs.pResult;
			this->stMaxSize = rhs.stMaxSize;
		}

		T* allocate(size_type count)
		{
			void* pMemory = pResult->Alloc(count * sizeof(T));
			if (pMemory == NULL)
			{
				throw std::bad_alloc("No memory can be allocated");
				return NULL;
			}

			return (T*)pMemory;
		}

		void deallocate(void* ptr, size_type count) // Never dealloc here
		{
		}

		size_type max_size() const
		{
			return this->stMaxSize;
		}

		void construct(pointer ptr, const_reference val)
		{
			new (ptr) T(val);
		}

		void destroy(pointer ptr)
		{
			ptr->~T();
		}

	private:
		_DBResult* pResult;

		ysize_t stMaxSize;

		friend class CCResultAlloc; // <T>;
	};

	struct _BindInfoX
	{
		ybyte_t byDataType;
		yssize_t stStrLenOrInd;
		void* pPtr;
	};

	inline _BindInfoX _CreateBindInfo(ybyte_t byDataType, ysize_t stSize, const void* pData)
	{
		_BindInfoX bindInfo = { byDataType, stSize, (void*)pData };
		return bindInfo;
	}

	inline _BindInfoX _CreateBindInfoInt(const yint32_t* pData)
	{
		_BindInfoX bindInfo = { YXC_DB_DATA_TYPE_I32, sizeof(yint32_t), (void*)pData };
		return bindInfo;
	}

	inline _BindInfoX _CreateBindInfoStringA(YXC_BStringA bsStr)
	{
		_BindInfoX bindInfo;
		bindInfo.pPtr = bsStr.pStr;
		bindInfo.byDataType = YXC_DB_DATA_TYPE_VASTRING;
		if (bsStr.pStr == NULL)
		{
			bindInfo.stStrLenOrInd = YXC_DB_NULL_DATA;
		}
		else
		{
			bindInfo.stStrLenOrInd = YXC_BStringLenA(bsStr) * sizeof(char);
		}
		return bindInfo;
	}

	inline _BindInfoX _CreateBindInfoStringW(YXC_BStringW bsStr)
	{
		_BindInfoX bindInfo;
		bindInfo.pPtr = bsStr.pStr;
		bindInfo.byDataType = YXC_DB_DATA_TYPE_VWSTRING;
		if (bsStr.pStr == NULL)
		{
			bindInfo.stStrLenOrInd = YXC_DB_NULL_DATA;
		}
		else
		{
			bindInfo.stStrLenOrInd = YXC_BStringLenW(bsStr) * sizeof(wchar_t);
		}
		return bindInfo;
	}

	inline _BindInfoX _CreateBindInfoString(YXC_BString bsStr)
	{
		_BindInfoX bindInfo;
		bindInfo.pPtr = bsStr.pStr;
		bindInfo.byDataType = YXC_DB_DATA_TYPE_VSTRING;
		if (bsStr.pStr == NULL)
		{
			bindInfo.stStrLenOrInd = YXC_DB_NULL_DATA;
		}
		else
		{
			bindInfo.stStrLenOrInd = YXC_BStringLen(bsStr) * sizeof(ychar);
		}
		return bindInfo;
	}
}

#endif /* __INC_YXC_UTILITY_DB_COMMON_HPP__ */
