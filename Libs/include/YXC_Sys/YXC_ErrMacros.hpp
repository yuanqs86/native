/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_ERR_MACROS_HPP__
#define __INC_YXC_SYS_BASE_ERR_MACROS_HPP__

#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_SysBase.h>
#include <YXC_Sys/YXC_Characters.h>
#include <YXC_Sys/YXC_ErrorCode.h>

#define __STR2WSTR(str) L##str
#define _STR2WSTR(str) __STR2WSTR(str)

#if YCHAR_WCHAR_T
#if YXC_PLATFORM_MINGW
#define __EFUNCTION__ __efunction(__FUNCTION__)
#else
#define __EFUNCTION__ _STR2WSTR(__FUNCTION__)
#endif /* YXC_PLATFORM_MINGW */
#else
#define __EFUNCTION__ __FUNCTION__
#endif /* YXC_PLATFORM_WIN */

#ifdef __WFILE__
#undef __WFILE__
#endif /* __WFILE__ */

#if YCHAR_WCHAR_T
#define __EFILE__ _STR2WSTR(__FILE__)
#else
#define __EFILE__ __FILE__
#endif /* YXC_PLATFORM_WIN */

#ifndef __MODULE__
#define __EMODULE__ __EFILE__
#else
#if YCHAR_WCHAR_T
#define __EMODULE__ _STR2WSTR(__MODULE__)
#else
#define __EMODULE__ __MODULE__
#endif /* YCHAR_WCHAR_T */
#endif /* __MODULE__ */

#ifndef __LABEL__
#define __LABEL__ err_ret
#endif /* __LABEL__ */

#ifndef __VARIABLE_RC__
#define __VARIABLE_RC__ rcRet
#endif /* __VARIABLE_RC__ */

#if !YXC_GNU_C

#define _YXC_REPORT_IMPL(bClear, code, wszMsg, ...) YXC_ReportErrorFormat(bClear, code, __EMODULE__, __EFILE__, __LINE__, __EFUNCTION__, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_RET(cond, rc)												\
	do {																	\
		if (!(cond)) {														\
			return rc;														\
		}																	\
	} while (0)

#define _YXC_CHECK_REPORT_RET_IMPL(cond, bClear, code, rc, wszMsg, ...)		\
	do {																	\
		if (!(cond)) {														\
			_YXC_REPORT_IMPL(bClear, code, wszMsg, ##__VA_ARGS__);				\
			return rc;														\
		}																	\
	} while (0)

#define _YXC_CHECK_GOTO(cond, rc)											\
	do {																	\
		if (!(cond)) {														\
			__VARIABLE_RC__ = rc;											\
			goto __LABEL__; 												\
		}																	\
	} while (0)

#define _YXC_CHECK_REPORT_GOTO_IMPL(cond, bClear, code, rc, wszMsg, ...)		\
	do {																	\
		if (!(cond)) {														\
			_YXC_REPORT_IMPL(bClear, code, wszMsg, ##__VA_ARGS__);				\
			__VARIABLE_RC__ = rc;											\
			goto __LABEL__; 												\
		}																	\
	} while (0)

#define _YXC_REPORT_ERR(code, wszMsg, ...) _YXC_REPORT_IMPL(FALSE, code, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_NEW_ERR(code, wszMsg, ...) _YXC_REPORT_IMPL(TRUE, code, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_RET(cond, code, wszMsg, ...) _YXC_CHECK_REPORT_RET_IMPL(cond, FALSE, code, code, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_NEW_RET(cond, code, wszMsg, ...) _YXC_CHECK_REPORT_RET_IMPL(cond, TRUE, code, code, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_NEW_RET(code, wszMsg, ...) _YXC_CHECK_REPORT_NEW_RET(FALSE, code, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_RET(code, wszMsg, ...) _YXC_CHECK_REPORT_RET(FALSE, code, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_RET2(cond, code, rc, wszMsg, ...) _YXC_CHECK_REPORT_RET_IMPL(cond, FALSE, code, rc, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_NEW_RET2(cond, code, rc, wszMsg, ...) _YXC_CHECK_REPORT_RET_IMPL(cond, TRUE, code, rc, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_GOTO(cond, code, wszMsg, ...) _YXC_CHECK_REPORT_GOTO_IMPL(cond, FALSE, code, code,  wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_GOTO2(cond, code, rc, wszMsg, ...) _YXC_CHECK_REPORT_GOTO_IMPL(cond, FALSE, code, rc,  wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_NEW_GOTO(cond, code, wszMsg, ...) _YXC_CHECK_REPORT_GOTO_IMPL(cond, TRUE, code, code, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_NEW_GOTO2(cond, code, rc, wszMsg, ...) _YXC_CHECK_REPORT_GOTO_IMPL(cond, TRUE, code, rc, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_NEW_GOTO(code, wszMsg, ...) _YXC_CHECK_REPORT_NEW_GOTO(FALSE, code, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_GOTO(code, wszMsg, ...) _YXC_CHECK_REPORT_GOTO(FALSE, code, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_IMPL_EX(bClear, errType, cat, code, wszMsg, stExtend, pvExtend, ...)	\
	YXC_ReportErrorExFormat(bClear, errType, cat, code, __EMODULE__, __EFILE__, __LINE__, __EFUNCTION__, stExtend, pvExtend, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_RET_IMPL_EX(cond, bClear, errSrc, cat, code, rc, stExtend, pvExtend, wszMsg, ...)	\
	do {																									\
		if (!(cond)) {																						\
			_YXC_REPORT_IMPL_EX(bClear, errSrc, cat, code, wszMsg, stExtend, pvExtend, ##__VA_ARGS__);			\
			return rc;																						\
		}																									\
	} while (0)

#define _YXC_CHECK_REPORT_GOTO_IMPL_EX(cond, bClear, errSrc, cat, code, rc, stExtend, pvExtend, wszMsg, ...)	\
	do {																									\
		if (!(cond)) {																						\
			_YXC_REPORT_IMPL_EX(bClear, errSrc, cat, code, wszMsg, stExtend, pvExtend, ##__VA_ARGS__);			\
			__VARIABLE_RC__ = rc;																			\
			goto __LABEL__; 																				\
		}																									\
	} while (0)


#define _YXC_REPORT_ERR_EX(errSrc, cat, code, wszMsg, ...) _YXC_REPORT_IMPL_EX(FALSE, errSrc, cat, code, wszMsg, 0, NULL, ##__VA_ARGS__)

#define _YXC_REPORT_ERR_EX2(errSrc, cat, code, wszMsg, stExtend, pvExtend, ...) _YXC_REPORT_IMPL_EX(FALSE, errSrc, cat, code, wszMsg, stExtend, pvExtend, ##__VA_ARGS__)

#define _YXC_REPORT_NEW_ERR_EX(errSrc, cat, code, wszMsg, ...) _YXC_REPORT_IMPL_EX(TRUE, errSrc, cat, code, wszMsg, 0, NULL, ##__VA_ARGS__)

#define _YXC_REPORT_NEW_ERR_EX2(errSrc, cat, code, wszMsg, stExtend, pvExtend, ...) _YXC_REPORT_IMPL_EX(TRUE, errSrc, cat, code, wszMsg, stExtend, pvExtend, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_RET_EX(cond, errSrc, cat, code, rc, wszMsg, ...)		\
	_YXC_CHECK_REPORT_RET_IMPL_EX(cond, TRUE, errSrc, cat, code, rc, 0, NULL, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_GOTO_EX(cond, errSrc, cat, code, rc, wszMsg, ...)		\
	_YXC_CHECK_REPORT_GOTO_IMPL_EX(cond, TRUE, errSrc, cat, code, rc, 0, NULL, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_RET_EX2(cond, errSrc, cat, code, rc, wszMsg, stExtend, pvExtend, ...)		\
	_YXC_CHECK_REPORT_RET_IMPL_EX(cond, TRUE, errSrc, cat, code, rc, stExtend, pvExtend, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_REPORT_GOTO_EX2(cond, errSrc, cat, code, rc, wszMsg, stExtend, pvExtend, ...)		\
	_YXC_CHECK_REPORT_GOTO_IMPL_EX(cond, TRUE, errSrc, cat, code, rc, stExtend, pvExtend, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_OS_ERR(wszMsg, ...) _YXC_REPORT_IMPL_EX(TRUE, YXC_ERROR_SRC_OS, YXC_ERR_CAT_OS_LRESULT, YXCLib::OSGetLastError(), wszMsg, 0, NULL, ##__VA_ARGS__)

#define _YXC_CHECK_OS_RET(cond, wszMsg, ...)		\
	_YXC_CHECK_REPORT_RET_IMPL_EX(cond, TRUE, YXC_ERROR_SRC_OS, YXC_ERR_CAT_OS_LRESULT, YXCLib::OSGetLastError(), YXC_ERC_OS, 0, NULL, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_OS_RET(wszMsg, ...) _YXC_CHECK_OS_RET(FALSE, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_OS_GOTO(cond, wszMsg, ...)		\
	_YXC_CHECK_REPORT_GOTO_IMPL_EX(cond, TRUE, YXC_ERROR_SRC_OS, YXC_ERR_CAT_OS_LRESULT, YXCLib::OSGetLastError(), YXC_ERC_OS, 0, NULL, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_OS_GOTO(wszMsg, ...) _YXC_CHECK_OS_GOTO(FALSE, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_HR_RET(hr, wszMsg, ...)													\
	do {																					\
		HRESULT __e_hrx = (hr);																\
		_YXC_CHECK_REPORT_RET_EX(__e_hrx == S_OK, YXC_ERROR_SRC_OS, YXC_ERR_CAT_OS_HRESULT,	\
			__e_hrx, YXC_ERC_OS, wszMsg, ##__VA_ARGS__);										\
	} while (0)

#define _YXC_CHECK_HR_GOTO(hr, wszMsg, ...)													\
	do {																					\
		HRESULT __e_hrx = (hr);																\
		_YXC_CHECK_REPORT_GOTO_EX(__e_hrx == S_OK, YXC_ERROR_SRC_OS, YXC_ERR_CAT_OS_HRESULT,	\
			__e_hrx, YXC_ERC_OS, 0, ##__VA_ARGS__);											\
	} while (0)

#define _YXC_CHECK_MMS_RET(mr, wszMsg, ...)											\
	do {																			\
		MMRESULT __e_mrx = (mr);													\
		_YXC_CHECK_REPORT_RET_EX(__e_mrx == MMSYSERR_NOERROR, YXC_ERROR_SRC_OS,		\
			YXC_ERR_CAT_OS_MMSYSTEM,	__e_mrx, YXC_ERC_OS, wszMsg, ##__VA_ARGS__);		\
	} while (0)

#define _YXC_CHECK_MMS_GOTO(mr, wszMsg, ...)											\
	do {																			\
		MMRESULT __e_mrx = (mr);													\
		_YXC_CHECK_REPORT_GOTO_EX(__e_mrx == MMSYSERR_NOERROR, YXC_ERROR_SRC_OS,		\
			YXC_ERR_CAT_OS_MMSYSTEM,	__e_mrx, YXC_ERC_OS, wszMsg, ##__VA_ARGS__);		\
	} while (0)

#define _YXC_REPORT_CRT_ERR(wszMsg, ...) _YXC_REPORT_IMPL_EX(TRUE, YXC_ERROR_SRC_C_RUNTIME, 0, errno, wszMsg, 0, NULL, ##__VA_ARGS__)

#define _YXC_CHECK_CRT_RET(cond, wszMsg, ...)		\
	_YXC_CHECK_REPORT_RET_IMPL_EX(cond, TRUE, YXC_ERROR_SRC_C_RUNTIME, 0, errno, YXC_ERC_C_RUNTIME, 0, NULL, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_CRT_RET(wszMsg, ...) _YXC_CHECK_CRT_RET(FALSE, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_CRT_GOTO(cond, wszMsg, ...)		\
	_YXC_CHECK_REPORT_GOTO_IMPL_EX(cond, TRUE, YXC_ERROR_SRC_C_RUNTIME, 0, errno, YXC_ERC_C_RUNTIME, 0, NULL, wszMsg, ##__VA_ARGS__)

#define _YXC_REPORT_CRT_GOTO(wszMsg, ...) _YXC_CHECK_CRT_GOTO(FALSE, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_STATUS_RET(rc, wszMsg, ...)									\
	do {																		\
		YXC_Status rx = (rc);													\
		_YXC_CHECK_REPORT_RET(rx == YXC_ERC_SUCCESS, rx, wszMsg, ##__VA_ARGS__);	\
	} while (0)

#define _YXC_CHECK_STATUS_GOTO(rc, wszMsg, ...)									\
	do {																		\
		YXC_Status rx = (rc);													\
		_YXC_CHECK_REPORT_GOTO(rx == YXC_ERC_SUCCESS, rx, wszMsg, ##__VA_ARGS__);	\
	} while (0)

#define _YCHK_REPLACEMENT_NEW_RET(_Ptr, _Type, ...)                      	 \
    try {                                                                    \
        new (_Ptr) _Type(##__VA_ARGS__);                                       \
    } catch (std::exception& ex) {                                           \
        _YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("ctor of [%@] caused '%@'"), \
            YC(#_Type), typeid(_Type).name());                               \
    }

#define _YCHK_REPLACEMENT_NEW_RET1(_Ptr, _Type, ...)                      	 \
    try {                                                                    \
        new (_Ptr) _Type(__VA_ARGS__);                                       \
    } catch (std::exception& ex) {                                           \
        _YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("ctor of [%@] caused '%@'"), \
            YC(#_Type), typeid(_Type).name());                               \
    }

#define _YCHK_REPLACEMENT_NEW_GOTO(_Ptr, _Type, ...)                          \
    try {                                                                     \
        new (_Ptr) _Type(##__VA_ARGS__);                                        \
    } catch (std::exception& ex) {                                            \
        _YXC_REPORT_NEW_GOTO(YXC_ERC_C_RUNTIME, YC("ctor of [%@] caused '%@'"), \
            YC(#_Type), typeid(_Type).name());                                \
    }

#else /* YXC_GNU_C */

#define _YXC_REPORT_IMPL(bClear, code, wszMsg, args...) YXC_ReportErrorFormat(bClear, code, __EMODULE__, __EFILE__, __LINE__, __EFUNCTION__, wszMsg, ##args)

#define _YXC_CHECK_RET(cond, rc)												\
	do {																	\
		if (!(cond)) {														\
			return rc;														\
		}																	\
	} while (0)

#define _YXC_CHECK_REPORT_RET_IMPL(cond, bClear, code, rc, wszMsg, args...)	\
	do {																	\
		if (!(cond)) {														\
			_YXC_REPORT_IMPL(bClear, code, wszMsg, ##args);					\
			return rc;														\
		}																	\
	} while (0)

#define _YXC_CHECK_GOTO(cond, rc)											\
	do {																	\
		if (!(cond)) {														\
			__VARIABLE_RC__ = rc;											\
			goto __LABEL__; 												\
		}																	\
	} while (0)

#define _YXC_CHECK_REPORT_GOTO_IMPL(cond, bClear, code, rc, wszMsg, args...)	\
	do {																	\
		if (!(cond)) {														\
			_YXC_REPORT_IMPL(bClear, code, wszMsg, ##args);					\
			__VARIABLE_RC__ = rc;											\
			goto __LABEL__; 												\
		}																	\
	} while (0)

#define _YXC_REPORT_ERR(code, wszMsg, args...) _YXC_REPORT_IMPL(FALSE, code, wszMsg, ##args)

#define _YXC_REPORT_NEW_ERR(code, wszMsg, args...) _YXC_REPORT_IMPL(TRUE, code, wszMsg, ##args)

#define _YXC_CHECK_REPORT_RET(cond, code, wszMsg, args...) _YXC_CHECK_REPORT_RET_IMPL(cond, FALSE, code, code, wszMsg, ##args)

#define _YXC_CHECK_REPORT_NEW_RET(cond, code, wszMsg, args...) _YXC_CHECK_REPORT_RET_IMPL(cond, TRUE, code, code, wszMsg, ##args)

#define _YXC_REPORT_NEW_RET(code, wszMsg, args...) _YXC_CHECK_REPORT_NEW_RET(FALSE, code, wszMsg, ##args)

#define _YXC_REPORT_RET(code, wszMsg, args...) _YXC_CHECK_REPORT_RET(FALSE, code, wszMsg, ##args)

#define _YXC_CHECK_REPORT_RET2(cond, code, rc, wszMsg, args...) _YXC_CHECK_REPORT_RET_IMPL(cond, FALSE, code, rc, wszMsg, ##args)

#define _YXC_CHECK_REPORT_NEW_RET2(cond, code, rc, wszMsg, args...) _YXC_CHECK_REPORT_RET_IMPL(cond, TRUE, code, rc, wszMsg, ##args)

#define _YXC_CHECK_REPORT_GOTO(cond, code, wszMsg, args...) _YXC_CHECK_REPORT_GOTO_IMPL(cond, FALSE, code, code,  wszMsg, ##args)

#define _YXC_CHECK_REPORT_GOTO2(cond, code, rc, wszMsg, args...) _YXC_CHECK_REPORT_GOTO_IMPL(cond, FALSE, code, rc,  wszMsg, ##args)

#define _YXC_CHECK_REPORT_NEW_GOTO(cond, code, wszMsg, args...) _YXC_CHECK_REPORT_GOTO_IMPL(cond, TRUE, code, code, wszMsg, ##args)

#define _YXC_CHECK_REPORT_NEW_GOTO2(cond, code, rc, wszMsg, args...) _YXC_CHECK_REPORT_GOTO_IMPL(cond, TRUE, code, rc, wszMsg, ##args)

#define _YXC_REPORT_NEW_GOTO(code, wszMsg, args...) _YXC_CHECK_REPORT_NEW_GOTO(FALSE, code, wszMsg, ##args)

#define _YXC_REPORT_GOTO(code, wszMsg, args...) _YXC_CHECK_REPORT_GOTO(FALSE, code, wszMsg, ##args)

#define _YXC_REPORT_IMPL_EX(bClear, errType, cat, code, wszMsg, stExtend, pvExtend, args...)	\
	YXC_ReportErrorExFormat(bClear, errType, cat, code, __EMODULE__, __EFILE__, __LINE__, __EFUNCTION__, stExtend, pvExtend, wszMsg, ##args)

#define _YXC_CHECK_REPORT_RET_IMPL_EX(cond, bClear, errSrc, cat, code, rc, stExtend, pvExtend, wszMsg, args...)	\
	do {																										\
		if (!(cond)) {																							\
			_YXC_REPORT_IMPL_EX(bClear, errSrc, cat, code, wszMsg, stExtend, pvExtend, ##args);              	\
			return rc;																							\
		}																										\
	} while (0)

#define _YXC_CHECK_REPORT_GOTO_IMPL_EX(cond, bClear, errSrc, cat, code, rc, stExtend, pvExtend, wszMsg, args...)	\
	do {																										\
		if (!(cond)) {																							\
			_YXC_REPORT_IMPL_EX(bClear, errSrc, cat, code, wszMsg, stExtend, pvExtend, ##args);              	\
			__VARIABLE_RC__ = rc;																				\
			goto __LABEL__; 																					\
		}																										\
	} while (0)

#define _YXC_REPORT_ERR_EX(errSrc, cat, code, wszMsg, args...) _YXC_REPORT_IMPL_EX(FALSE, errSrc, cat, code, wszMsg, 0, NULL, ##args)

#define _YXC_REPORT_ERR_EX2(errSrc, cat, code, wszMsg, stExtend, pvExtend, args...) _YXC_REPORT_IMPL_EX(FALSE, errSrc, cat, code, wszMsg, stExtend, pvExtend, ##args)

#define _YXC_REPORT_NEW_ERR_EX(errSrc, cat, code, wszMsg, args...) _YXC_REPORT_IMPL_EX(TRUE, errSrc, cat, code, wszMsg, 0, NULL, ##args)

#define _YXC_REPORT_NEW_ERR_EX2(errSrc, cat, code, wszMsg, stExtend, pvExtend, args...) _YXC_REPORT_IMPL_EX(TRUE, errSrc, cat, code, wszMsg, stExtend, pvExtend, ##args)

#define _YXC_CHECK_REPORT_RET_EX(cond, errSrc, cat, code, rc, wszMsg, args...)		\
	_YXC_CHECK_REPORT_RET_IMPL_EX(cond, TRUE, errSrc, cat, code, rc, 0, NULL, wszMsg, ##args)

#define _YXC_CHECK_REPORT_GOTO_EX(cond, errSrc, cat, code, rc, wszMsg, args...)		\
	_YXC_CHECK_REPORT_GOTO_IMPL_EX(cond, TRUE, errSrc, cat, code, rc, 0, NULL, wszMsg, ##args)

#define _YXC_CHECK_REPORT_RET_EX2(cond, errSrc, cat, code, rc, wszMsg, stExtend, pvExtend, args...)		\
	_YXC_CHECK_REPORT_RET_IMPL_EX(cond, TRUE, errSrc, cat, code, rc, stExtend, pvExtend, wszMsg, ##args)

#define _YXC_CHECK_REPORT_GOTO_EX2(cond, errSrc, cat, code, rc, wszMsg, stExtend, pvExtend, args...)		\
	_YXC_CHECK_REPORT_GOTO_IMPL_EX(cond, TRUE, errSrc, cat, code, rc, stExtend, pvExtend, wszMsg, ##args)

#define _YXC_REPORT_OS_ERR(wszMsg, args...) _YXC_REPORT_CRT_ERR(wszMsg, ##args)

#define _YXC_CHECK_OS_RET(cond, wszMsg, args...) _YXC_CHECK_CRT_RET(cond, wszMsg, ##args)

#define _YXC_REPORT_OS_RET(wszMsg, args...) _YXC_REPORT_CRT_RET(FALSE, wszMsg, ##args)

#define _YXC_CHECK_OS_GOTO(cond, wszMsg, args...) _YXC_CHECK_CRT_GOTO(cond, wszMsg, ##args)

#define _YXC_REPORT_OS_GOTO(wszMsg, args...) _YXC_REPORT_CRT_GOTO(FALSE, wszMsg, ##args)

#if YXC_PLATFORM_WIN

#define _YXC_CHECK_HR_RET(hr, wszMsg, args...)											\
	do {																				\
		HRESULT hrx = (hr);																\
		_YXC_CHECK_REPORT_RET_EX(hrx == S_OK, YXC_ERROR_SRC_OS, YXC_ERR_CAT_OS_HRESULT,	\
			hrx, YXC_ERC_OS, wszMsg, ##args);											\
	} while (0)

#define _YXC_CHECK_HR_GOTO(hr, wszMsg, args...)											\
	do {																				\
		HRESULT hrx = (hr);																\
		_YXC_CHECK_REPORT_GOTO_EX(hrx == S_OK, YXC_ERROR_SRC_OS, YXC_ERR_CAT_OS_HRESULT,	\
			hrx, YXC_ERC_OS, 0, ##args);													\
	} while (0)

#endif /* YXC_PLATFORM_WIN */

#define _YXC_REPORT_CRT_ERR(wszMsg, args...) _YXC_REPORT_IMPL_EX(TRUE, YXC_ERROR_SRC_C_RUNTIME, 0, errno, wszMsg, 0, NULL, ##args)

#define _YXC_CHECK_CRT_RET(cond, wszMsg, args...)		\
	_YXC_CHECK_REPORT_RET_IMPL_EX(cond, TRUE, YXC_ERROR_SRC_C_RUNTIME, 0, errno, YXC_ERC_C_RUNTIME, 0, NULL, wszMsg, ##args)

#define _YXC_REPORT_CRT_RET(wszMsg, args...) _YXC_CHECK_CRT_RET(FALSE, wszMsg, ##args)

#define _YXC_CHECK_CRT_GOTO(cond, wszMsg, args...)		\
	_YXC_CHECK_REPORT_GOTO_IMPL_EX(cond, TRUE, YXC_ERROR_SRC_C_RUNTIME, 0, errno, YXC_ERC_C_RUNTIME, 0, NULL, wszMsg, ##args)

#define _YXC_REPORT_CRT_GOTO(wszMsg, args...) _YXC_CHECK_CRT_GOTO(FALSE, wszMsg, ##args)

#define _YXC_CHECK_STATUS_RET(rc, wszMsg, args...)						\
	do {																\
		YXC_Status rx = (rc);											\
		_YXC_CHECK_REPORT_RET(rx == YXC_ERC_SUCCESS, rx, wszMsg, ##args);	\
	} while (0)

#define _YXC_CHECK_STATUS_GOTO(rc, wszMsg, args...)							\
	do {																	\
		YXC_Status rx = (rc);												\
		_YXC_CHECK_REPORT_GOTO(rx == YXC_ERC_SUCCESS, rx, wszMsg, ##args);	\
	} while (0)

#define _YCHK_REPLACEMENT_NEW_RET(_Ptr, _Type, args...)                      \
    try {                                                                    \
        new (_Ptr) _Type(##args);                                            \
    } catch (std::exception& ex) {                                           \
        _YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("ctor of [%@] caused '%@'"), \
            YC(#_Type), typeid(_Type).name());                               \
    }

#define _YCHK_REPLACEMENT_NEW_RET1(_Ptr, _Type, _arg1, args...)                      \
    try {                                                                    \
		new (_Ptr) _Type(_arg1, ##args);                                            \
    } catch (std::exception& ex) {                                           \
        _YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("ctor of [%@] caused '%@'"), \
            YC(#_Type), typeid(_Type).name());                               \
    }

#define _YCHK_REPLACEMENT_NEW_GOTO(_Ptr, _Type, args...)                      \
    try {                                                                     \
        new (_Ptr) _Type(##args);                                             \
    } catch (std::exception& ex) {                                            \
        _YXC_REPORT_NEW_GOTO(YXC_ERC_C_RUNTIME, YC("ctor of [%@] caused '%@'"), \
            YC(#_Type), typeid(_Type).name());                                \
    }

#define _YCHK_REPLACEMENT_NEW_GOTO1(_Ptr, _Type, _arg1, args...)                      \
    try {                                                                     \
        new (_Ptr) _Type(_arg1, ##args);                                             \
    } catch (std::exception& ex) {                                            \
        _YXC_REPORT_NEW_GOTO(YXC_ERC_C_RUNTIME, YC("ctor of [%@] caused '%@'"), \
            YC(#_Type), typeid(_Type).name());                                \
    }

#endif /* YXC_GNU_C */

#define _YXC_FATAL_ASSERT(expr)						\
	do {											\
		ybool_t expr_bRet = (expr);					\
		if (!expr_bRet) {							\
			YXC_FatalAssert(YC("%s"), YC(#expr));	\
		}											\
	} while (0)

#if YXC_GNU_C

#define _YXC_FATAL_ASSERT_MSG(expr, msg, args...)	\
	do {											\
		ybool_t expr_bRet = (expr);					\
		if (!expr_bRet) {							\
			YXC_FatalAssert(_msg, ##args);			\
		}											\
	} while (0)

#define _YXC_FATAL_ASSERT_SYSMSG(expr, msg, args...)	\
	do {											\
		YXC_Status expr_rx = (expr);					\
		if (expr_rx != YXC_ERC_SUCCESS) {			\
			YXC_FatalAssertSys(_msg, ##args);		\
		}											\
	} while (0)

#else

#define _YXC_FATAL_ASSERT_MSG(_expr, _msg, ...)		\
	do {											\
		ybool_t expr_bRet = (_expr);				\
		if (!expr_bRet) {							\
			YXC_FatalAssert(_msg, ##__VA_ARGS__);		\
		}											\
	} while (0)

#define _YXC_FATAL_ASSERT_SYSMSG(_rc, _msg, ...)		\
	do {											\
		YXC_Status expr_rx = (_rc);					\
		if (expr_rx != YXC_ERC_SUCCESS) {			\
			YXC_FatalAssertSys(_msg, ##__VA_ARGS__);	\
		}											\
	} while (0)

#endif /* YXC_GNU_C */

#define _YXC_CHECK_RC_RET(rc)						\
	do {											\
		YXC_Status expr_rx = (rc);					\
		if (expr_rx != YXC_ERC_SUCCESS) {			\
			return expr_rx;							\
		}											\
	} while (0)

#define _YXC_CHECK_RC_GOTO(rc)						\
	do {											\
		YXC_Status expr_rx = (rc);					\
		if (expr_rx != YXC_ERC_SUCCESS) {			\
			__VARIABLE_RC__ = expr_rx;				\
			goto __LABEL__;							\
		}											\
	} while (0)

#define _YXC_CHECK_RC_RETP(rc) _YXC_CHECK_STATUS_RET(rc, YC("%s"), YC("Forwarding ") YC(#rc))
#define _YXC_CHECK_RC_GOTOP(rc) _YXC_CHECK_STATUS_GOTO(rc, YC("%s"), YC("Forwarding ") YC(#rc))

#define _YXC_FATAL_ASSERT_EXPR(expr)					\
	do {											\
		ybool_t expr_bRet = (expr);					\
		if (!expr_bRet) {							\
			YXC_FatalAssert(YC("%s"), YC(#expr));	\
		}											\
	} while (0)

#define _YXC_FATAL_ASSERT_SYSEXPR(expr)				\
	do {											\
		YXC_Status expr_bRet = (expr);				\
		if (expr_bRet != YXC_ERC_SUCCESS) {			\
			YXC_FatalAssertSys(YC("%s"), YC(#expr));	\
		}											\
	} while (0)

#if YXC_PLATFORM_WIN

#define _YXC_SURE_GETPROC(_hDll, _proc_name, _ret)								\
	_YXC_GET_PROC(_hDll, _proc_name, _ret);										\
	_YXC_FATAL_ASSERT_MSG(_ret != NULL, YC("Load '%@' failed"), #_proc_name);

#define _YXC_GET_PROC(_hDll, _proc_name, _ret)								\
	do {																	\
		FARPROC proc_load = (FARPROC)GetProcAddress(_hDll, #_proc_name);	\
		memcpy(&_ret, &proc_load, sizeof(FARPROC));							\
	} while (0)

#endif /* YXC_PLATFORM_WIN */

#define _YXC_MAL_PTR1(_Ptr, _Type, _Count) _Type* _Ptr = (_Type*)malloc(sizeof(_Type) * (_Count))
#define _YXC_MAL_PTR2(_Ptr, _Type, _Count) _Ptr = (_Type*)malloc(sizeof(_Type) * (_Count))
#define _YXC_EAL_PTR1(_Ptr, _Al, _Type, _Count) _Type* _Ptr = (_Type*)(_Al).Alloc(sizeof(_Type) * (_Count))
#define _YXC_EAL_PTR2(_Ptr, _Al, _Type, _Count) _Ptr = (_Type*)(_Al).Alloc(sizeof(_Type) * (_Count))

#define _YCHK_APTR_PROC(_Proc, _Ptr, _Count) _Proc(_Ptr != NULL, YXC_ERC_OUT_OF_MEMORY, \
	YC("Alloc for member %s(%d) failed"), YC(#_Ptr), _Count)

#define _YCHK_APTR_NR(_Ptr, _Count) _YCHK_APTR_PROC(_YXC_CHECK_REPORT_NEW_RET, _Ptr, _Count)
#define _YCHK_APTR_NG(_Ptr, _Count) _YCHK_APTR_PROC(_YXC_CHECK_REPORT_NEW_GOTO, _Ptr, _Count)
#define _YCHK_APTR_OR(_Ptr, _Count) _YCHK_APTR_PROC(_YXC_CHECK_REPORT_RET, _Ptr, _Count)
#define _YCHK_APTR_OG(_Ptr, _Count) _YCHK_APTR_PROC(_YXC_CHECK_REPORT_GOTO, _Ptr, _Count)

#define _YCHK_MAL_R1(_Ptr, _Type) _YXC_MAL_PTR1(_Ptr, _Type, 1); _YCHK_APTR_NR(_Ptr, 1);
#define _YCHK_MAL_R2(_Ptr, _Type) _YXC_MAL_PTR2(_Ptr, _Type, 1); _YCHK_APTR_NR(_Ptr, 1);
#define _YCHK_MAL_G1(_Ptr, _Type) _YXC_MAL_PTR1(_Ptr, _Type, 1); _YCHK_APTR_NG(_Ptr, 1);
#define _YCHK_MAL_G2(_Ptr, _Type) _YXC_MAL_PTR2(_Ptr, _Type, 1); _YCHK_APTR_NG(_Ptr, 1);

#define _YCHK_MAL_ARR_R1(_Ptr, _Type, _C) _Type* _Ptr; do { ysize_t _xc = (_C); _YXC_MAL_PTR2(_Ptr, _Type, _xc); _YCHK_APTR_NR(_Ptr, _xc); } while (0)
#define _YCHK_MAL_ARR_R2(_Ptr, _Type, _C) do { ysize_t _xc = (_C); _YXC_MAL_PTR2(_Ptr, _Type, _xc); _YCHK_APTR_NR(_Ptr, _xc); } while (0)
#define _YCHK_MAL_ARR_G1(_Ptr, _Type, _C) _Type* _Ptr; do { ysize_t _xc = (_C); _YXC_MAL_PTR1(_Ptr, _Type, _xc); _YCHK_APTR_NG(_Ptr, _xc); } while (0)
#define _YCHK_MAL_ARR_G2(_Ptr, _Type, _C) do { ysize_t _xc = (_C); _YXC_MAL_PTR2(_Ptr, _Type, _xc); _YCHK_APTR_NG(_Ptr, _xc); } while (0)

#define _YCHK_MAL_STRA_R1(_Ptr, _Len) _YCHK_MAL_ARR_R1(_Ptr, char, (_Len) + 1)
#define _YCHK_MAL_STRA_R2(_Ptr, _Len) _YCHK_MAL_ARR_R2(_Ptr, char, (_Len) + 1)
#define _YCHK_MAL_STRA_G1(_Ptr, _Len) _YCHK_MAL_ARR_G1(_Ptr, char, (_Len) + 1)
#define _YCHK_MAL_STRA_G2(_Ptr, _Len) _YCHK_MAL_ARR_G2(_Ptr, char, (_Len) + 1)
#define _YCHK_MAL_STRW_R1(_Ptr, _Len) _YCHK_MAL_ARR_R1(_Ptr, wchar_t, (_Len) + 1)
#define _YCHK_MAL_STRW_R2(_Ptr, _Len) _YCHK_MAL_ARR_R2(_Ptr, wchar_t, (_Len) + 1)
#define _YCHK_MAL_STRW_G1(_Ptr, _Len) _YCHK_MAL_ARR_G1(_Ptr, wchar_t, (_Len) + 1)
#define _YCHK_MAL_STRW_G2(_Ptr, _Len) _YCHK_MAL_ARR_G2(_Ptr, wchar_t, (_Len) + 1)
#define _YCHK_MAL_STR_R1(_Ptr, _Len) _YCHK_MAL_ARR_R1(_Ptr, ychar, (_Len) + 1)
#define _YCHK_MAL_STR_R2(_Ptr, _Len) _YCHK_MAL_ARR_R2(_Ptr, ychar, (_Len) + 1)
#define _YCHK_MAL_STR_G1(_Ptr, _Len) _YCHK_MAL_ARR_G1(_Ptr, ychar, (_Len) + 1)
#define _YCHK_MAL_STR_G2(_Ptr, _Len) _YCHK_MAL_ARR_G2(_Ptr, ychar, (_Len) + 1)

#define _YCHK_EAL_ARR_R1(_Ptr, _Al, _Type, _C) _Type* _Ptr; do { ysize_t _xc = (_C); _YXC_EAL_PTR2(_Ptr, _Al, _Type, _xc); _YCHK_APTR_NR(_Ptr, _xc); } while (0)
#define _YCHK_EAL_ARR_R2(_Ptr, _Al, _Type, _C) do { ysize_t _xc = (_C); _YXC_EAL_PTR2(_Ptr, _Al, _Type, _xc); _YCHK_APTR_NR(_Ptr, _xc); } while (0)
#define _YCHK_EAL_ARR_G1(_Ptr, _Al, _Type, _C) _Type* _Ptr; do { ysize_t _xc = (_C); _YXC_EAL_PTR2(_Ptr, _Al, _Type, _xc); _YCHK_APTR_NG(_Ptr, _xc); } while (0)
#define _YCHK_EAL_ARR_G2(_Ptr, _Al, _Type, _C) do { ysize_t _xc = (_C); _YXC_EAL_PTR2(_Ptr, _Al, _Type, _xc); _YCHK_APTR_NG(_Ptr, _xc); } while (0)

#define _YCHK_EAL_R1(_Ptr, _Al, _Type) _YCHK_EAL_ARR_R1(_Ptr, _Al, _Type, 1)
#define _YCHK_EAL_R2(_Ptr, _Al, _Type) _YCHK_EAL_ARR_R2(_Ptr, _Al, _Type, 1)
#define _YCHK_EAL_G1(_Ptr, _Al, _Type) _YCHK_EAL_ARR_G1(_Ptr, _Al, _Type, 1)
#define _YCHK_EAL_G2(_Ptr, _Al, _Type) _YCHK_EAL_ARR_G2(_Ptr, _Al, _Type, 1)

#define _YCHK_EAL_STRA_R1(_Ptr, _Al, _Len) _YCHK_EAL_ARR_R1(_Ptr, _Al, char, (_Len) + 1)
#define _YCHK_EAL_STRA_R2(_Ptr, _Al, _Len) _YCHK_EAL_ARR_R2(_Ptr, _Al, char, (_Len) + 1)
#define _YCHK_EAL_STRA_G1(_Ptr, _Al, _Len) _YCHK_EAL_ARR_G1(_Ptr, _Al, char, (_Len) + 1)
#define _YCHK_EAL_STRA_G2(_Ptr, _Al, _Len) _YCHK_EAL_ARR_G2(_Ptr, _Al, char, (_Len) + 1)
#define _YCHK_EAL_STRW_R1(_Ptr, _Al, _Len) _YCHK_EAL_ARR_R1(_Ptr, _Al, wchar_t, (_Len) + 1)
#define _YCHK_EAL_STRW_R2(_Ptr, _Al, _Len) _YCHK_EAL_ARR_R2(_Ptr, _Al, wchar_t, (_Len) + 1)
#define _YCHK_EAL_STRW_G1(_Ptr, _Al, _Len) _YCHK_EAL_ARR_G1(_Ptr, _Al, wchar_t, (_Len) + 1)
#define _YCHK_EAL_STRW_G2(_Ptr, _Al, _Len) _YCHK_EAL_ARR_G2(_Ptr, _Al, wchar_t, (_Len) + 1)
#define _YCHK_EAL_STR_R1(_Ptr, _Al, _Len) _YCHK_EAL_ARR_R1(_Ptr, _Al, ychar, (_Len) + 1)
#define _YCHK_EAL_STR_R2(_Ptr, _Al, _Len) _YCHK_EAL_ARR_R2(_Ptr, _Al, ychar, (_Len) + 1)
#define _YCHK_EAL_STR_G1(_Ptr, _Al, _Len) _YCHK_EAL_ARR_G1(_Ptr, _Al, ychar, (_Len) + 1)
#define _YCHK_EAL_STR_G2(_Ptr, _Al, _Len) _YCHK_EAL_ARR_G2(_Ptr, _Al, ychar, (_Len) + 1)

#define _YXC_TRY_START() try {

#define _YXC_CATCH_RET()															\
    } catch (const std::exception& ex) {                                    	\
        _YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("Exception '%@'"), ex.what()); 	\
	}
#define _YXC_CATCH_GOTO()														\
    } catch (const std::exception& ex) {                                    	\
        _YXC_REPORT_NEW_GOTO(YXC_ERC_C_RUNTIME, YC("Exception '%@'"), ex.what()); \
	}

#define _YCHK_EAL_STRA_COPY_R1(_Ptr, _Al, _Str)			\
	char* _Ptr;											\
	do {												\
		ysize_t _l1 = strlen(_Str);						\
		_YCHK_EAL_ARR_R2(_Ptr, _Al, char, _l1 + 1);		\
		memcpy(_Ptr, _Str, (_l1 + 1) * sizeof(char));	\
	} while (0)

#define _YCHK_EAL_STRA_COPY_G1(_Ptr, _Al, _Str)			\
	char* _Ptr;											\
	do {												\
		ysize_t _l1 = strlen(_Str);						\
		_YCHK_EAL_ARR_R2(_Ptr, _Al, char, _l1 + 1);		\
		memcpy(_Ptr, _Str, (_l1 + 1) * sizeof(char));	\
	} while (0)

#define _YCHK_EAL_STRA_COPY_R2(_Ptr, _Al, _Str)			\
	do {												\
		ysize_t _l1 = strlen(_Str);						\
		_YCHK_EAL_ARR_R2(_Ptr, _Al, char, _l1 + 1);		\
		memcpy(_Ptr, _Str, (_l1 + 1) * sizeof(char));	\
	} while (0)

#define _YCHK_EAL_STRA_COPY_G2(_Ptr, _Al, _Str)			\
	do {												\
		ysize_t _l1 = strlen(_Str);						\
		_YCHK_EAL_ARR_R2(_Ptr, _Al, char, _l1 + 1);		\
		memcpy(_Ptr, _Str, (_l1 + 1) * sizeof(char));	\
	} while (0)

#define _YCHK_EAL_STRW_COPY_R1(_Ptr, _Al, _Str)			\
	wchar_t* _Ptr;										\
	do {												\
		ysize_t _l1 = wcslen(_Str);						\
		_YCHK_EAL_ARR_R2(_Ptr, _Al, wchar_t, _l1 + 1);	\
		memcpy(_Ptr, _Str, (_l1 + 1) * sizeof(wchar_t));\
	} while (0)

#define _YCHK_EAL_STRW_COPY_G1(_Ptr, _Al, _Str)			\
	wchar_t* _Ptr;										\
	do {												\
		ysize_t _l1 = wcslen(_Str);						\
		_YCHK_EAL_ARR_R2(_Ptr, _Al, wchar_t, _l1 + 1);	\
		memcpy(_Ptr, _Str, (_l1 + 1) * sizeof(wchar_t));\
	} while (0)

#define _YCHK_EAL_STRW_COPY_R2(_Ptr, _Al, _Str)			\
	do {												\
		ysize_t _l1 = wcslen(_Str);						\
		_YCHK_EAL_ARR_R2(_Ptr, _Al, wchar_t, _l1 + 1);	\
		memcpy(_Ptr, _Str, (_l1 + 1) * sizeof(wchar_t));\
	} while (0)

#define _YCHK_EAL_STRW_COPY_G2(_Ptr, _Al, _Str)			\
	do {												\
		ysize_t _l1 = wcslen(_Str);						\
		_YCHK_EAL_ARR_R2(_Ptr, _Al, wchar_t, _l1 + 1);	\
		memcpy(_Ptr, _Str, (_l1 + 1) * sizeof(wchar_t));\
	} while (0)

#if YCHAR_WCHAR_T
#define _YCHK_EAL_STR_COPY_R1(_Ptr, _Al, _Str) _YCHK_EAL_STRW_COPY_R1(_Ptr, _Al, _Str)
#define _YCHK_EAL_STR_COPY_R2(_Ptr, _Al, _Str) _YCHK_EAL_STRW_COPY_R2(_Ptr, _Al, _Str)
#define _YCHK_EAL_STR_COPY_G1(_Ptr, _Al, _Str) _YCHK_EAL_STRW_COPY_G1(_Ptr, _Al, _Str)
#define _YCHK_EAL_STR_COPY_G2(_Ptr, _Al, _Str) _YCHK_EAL_STRW_COPY_G2(_Ptr, _Al, _Str)
#else
#define _YCHK_EAL_STR_COPY_R1(_Ptr, _Al, _Str) _YCHK_EAL_STRA_COPY_R1(_Ptr, _Al, _Str)
#define _YCHK_EAL_STR_COPY_R2(_Ptr, _Al, _Str) _YCHK_EAL_STRA_COPY_R2(_Ptr, _Al, _Str)
#define _YCHK_EAL_STR_COPY_G1(_Ptr, _Al, _Str) _YCHK_EAL_STRA_COPY_G1(_Ptr, _Al, _Str)
#define _YCHK_EAL_STR_COPY_G2(_Ptr, _Al, _Str) _YCHK_EAL_STRA_COPY_G2(_Ptr, _Al, _Str)
#endif /* YCHAR_WCHAR_T */

#endif /* __INC_YXC_SYS_BASE_ERR_MACROS_HPP__ */
