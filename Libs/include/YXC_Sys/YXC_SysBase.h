/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_BASE_H__
#define __INC_YXC_SYS_BASE_BASE_H__

#include <YXC_Sys/YXC_Types.h>
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//#include <malloc.h>
#endif /* _DEBUG */

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifdef YXC_MACROS_NO_MIN_MAX

#ifdef min
#undef min
#endif /* min */

#ifdef max
#undef max
#endif /* max */

#endif /* YXC_RESERVE_MIN_MAX */

#define YXC_DECLARE_STRUCTURE_HANDLE(Struct) typedef struct __YXC_DECLARE_HANDLE_##Struct { int unused; } *Struct

#ifdef __cplusplus
#define YXC_DECLARE_INHERIT_HANDLE(Struct, Parent)		\
	typedef struct __YXC_DECLARE_HANDLE_##Struct : public __YXC_DECLARE_HANDLE_##Parent {	} *Struct
#else
#define YXC_DECLARE_INHERIT_HANDLE(Struct, Parent) YXC_DECLARE_STRUCTURE_HANDLE(Struct)
#endif /* __cplusplus */

#define YXC_RESET_HANDLE(handle) do { handle = NULL; } while (0)

#define YXC_IS_VALID_HANDLE(handle) (handle != NULL)

#define YXC_INFINITE (yuint32_t)(-1)

#define YXC_ARR_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))
#define YXC_ARR_SIZE(type, len) ((len) * sizeof(type))

#define YXC_STRING_SZ_LENA(size) ((size - sizeof(char)) / sizeof(char))
#define YXC_STRING_SZ_LENW(size) ((size - sizeof(wchar_t)) / sizeof(wchar_t))
#define YXC_STRING_ARR_LEN(str) (sizeof(str) / sizeof(str[0]) - 1) // exclude terminate character
#define YXC_STR_CCH(str) (sizeof(str) / sizeof(str[0]) - 1) // exclude terminate character

#define YXC_STRING_SIZEA(len) (((len) + 1) * sizeof(char))
#define YXC_STRING_SIZEW(len) (((len) + 1) * sizeof(wchar_t))
#define YXC_STRING_SIZE(len) (((len) + 1) * sizeof(ychar))

#define YXC_STR_NTS (-1)

#define YXC_ALIGN_LEN(len, nAlign) (((len - 1) / (nAlign) + 1) * (nAlign))
#define YXC_ALIGN_PTR_LEN(len) YXC_ALIGN_LEN(len, sizeof(void*))

#define YXC_HI_32BITS(qw) ((yuint32_t)((yuint64_t)(qw) >> 32))
#define YXC_LO_32BITS(qw) ((yuint32_t)((yuint64_t)(qw) & 0xffffffff))

#define YXC_HI_16BITS(dw) ((yuint16_t)((yuint32_t)(dw) >> 16))
#define YXC_LO_16BITS(dw) ((yuint16_t)((yuint32_t)(dw) & 0xffff))

#define YXC_HI_8BITS(s) ((ybyte_t)((yuint16_t)(s) >> 8))
#define YXC_LO_8BITS(s) ((ybyte_t)((yuint16_t)(s) & 0xff))

#define YXC_MK64BITS_32(dwh, dwl) (((yuint64_t)((yuint32_t)(dwh)) << 32) | (yuint32_t)(dwl))
#define YXC_MK32BITS_16(wh, wl) (((yuint32_t)((yuint16_t)(wh)) << 16) | (yuint16_t)(wl))
#define YXC_MK16BITS_8(bh, bl) (((yuint16_t)((ybyte_t)(bh)) << 8) | (ybyte_t)(bl))

#define YXC_MAX_ERROR_STR (1024)
#define YXC_MAX_USERNAME (64)
#define YXC_MAX_CCH_PASSWORD (32 - 1)
#define YXC_MAX_FUNC_NAME (128)
#define YXC_MAX_NUM_VIDEOS (16)

#ifndef __cplusplus
	#define YXC_BYTES_IN_KB (ysize_t)(1 << 10)
	#define YXC_BYTES_IN_MB (ysize_t)(1 << 20)
	#define YXC_BYTES_IN_GB (ysize_t)(1 << 30)
	#define YXC_BYTES_IN_TB ((yuint64_t)1 << 40)
#else
	static const ysize_t YXC_BYTES_IN_KB = 1 << 10;
	static const ysize_t YXC_BYTES_IN_MB = 1 << 20;
	static const ysize_t YXC_BYTES_IN_GB = 1 << 30;
	static const yuint64_t YXC_BYTES_IN_TB = (yuint64_t)1 << 40;
#endif /* __cplusplus */

#define YXC_STRUCT_MEMBER_OFFSET(StructType, member) (yuint32_t)(&((StructType*)0)->member)

#define YXC_STD_IN_FILE 0
#define YXC_STD_OUT_FILE 1
#define YXC_STD_ERR_FILE 2

#define YXC_BAD_ALLOC_CODE 0x70000005

#if YXC_PLATFORM_WIN
#define YXC_MAX_CCH_PATH (MAX_PATH)
#define YXC_MAX_CCH_PATH_UTF8 (MAX_PATH * 3)
#else
#define YXC_MAX_CCH_PATH PATH_MAX
#define YXC_MAX_CCH_PATH_UTF8 PATH_MAX
#endif /* YXC_PLATFORM_WIN */

    typedef ychar YXC_FPath[YXC_MAX_CCH_PATH + 1];
    typedef char YXC_FPathA[YXC_MAX_CCH_PATH + 1];
    typedef wchar_t YXC_FPathW[YXC_MAX_CCH_PATH + 1];
    typedef yuint8_t YXC_FPathU[YXC_MAX_CCH_PATH * 3 + 1];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef __cplusplus
#define YXC_INLINE static inline
#define YXC_DEF_PARAM(x) = x

namespace YXCLib
{
	template <typename T>
    YXC_INLINE T TMin(T t1, T t2)
    {
        return t1 < t2 ? t1 : t2;
    }

    template <typename T>
    YXC_INLINE T TMax(T t1, T t2)
    {
        return t1 > t2 ? t1 : t2;
    }
}

#else
#define YXC_INLINE static
#define YXC_DEF_PARAM(x)
#endif /* __cplusplus */

YXC_INLINE int YXC_Round(double x)
{
	return (int)floor(x + 0.5);
}

#define YXC_DISALLOW_ASSIGN(_TypeName) _TypeName& operator=(const _TypeName&)
#define YXC_DISALLOW_COPY(_TypeName) _TypeName(const _TypeName&)

#endif /* __INC_YXC_SYS_BASE_BASE_H__ */
