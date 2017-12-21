
#include <YXC_AVKernel/Utils/DynamicFFWrap.hpp>
#include <stdio.h>

namespace _YXV_FFWrap
{
	func_YXV_FFObjConfigV g_fn_YXV_FFObjConfigV;
	func_YXV_FFObjConfigA g_fn_YXV_FFObjConfigA;
	func_YXV_FFObjClose g_fn_YXV_FFObjClose;
	func_YXV_FFDecCreate g_fn_YXV_FFDecCreate;
	func_YXV_FFDecProcessV g_fn_YXV_FFDecProcessV;
	func_YXV_FFDecProcessA g_fn_YXV_FFDecProcessA;
	func_YXV_FFEncCreate g_fn_YXV_FFEncCreate;
	func_YXV_FFEncProcessV g_fn_YXV_FFEncProcessV;
	func_YXV_FFEncProcessA g_fn_YXV_FFEncProcessA;
	func_YXV_FFEncReadExtV g_fn_YXV_FFEncReadExtV;
	func_YXV_FFEncReadExtA g_fn_YXV_FFEncReadExtA;
	func_YXV_FFFormatCreate g_fn_YXV_FFFormatCreate;
	func_YXV_FFFormatStartWrite g_fn_YXV_FFFormatStartWrite;
	func_YXV_FFFormatWriteV g_fn_YXV_FFFormatWriteV;
	func_YXV_FFFormatWriteA g_fn_YXV_FFFormatWriteA;
	func_YXV_FFFormatEndWrite g_fn_YXV_FFFormatEndWrite;
	func_YXV_FFVSpecParamDefault g_fn_YXV_FFVSpecParamDefault;
	func_DecodeH264SPS g_fn_DecodeH264SPS;
	func_av_frame_unref g_fn_av_frame_unref;
	func_av_frame_free g_fn_av_frame_free;
	func_av_buffer_unref g_fn_av_buffer_unref;

	func_sws_getCachedContext g_fn_sws_getCachedContext;
	func_sws_scale g_fn_sws_scale;
	func_sws_freeContext g_fn_sws_freeContext;
	func_swr_alloc_set_opts g_fn_swr_alloc_set_opts;
	func_swr_init g_fn_swr_init;
	func_swr_convert g_fn_swr_convert;
	func_swr_free g_fn_swr_free;
	HMODULE g_ffwrap_module;
}

static void PromptFuncLoadError(const char* name)
{
	char szErr[400];
	sprintf(szErr, "未能定位入口点%s在ffwrap.dll上!", name);
	MessageBoxA(NULL, szErr, "MediaWrap", MB_OK);
}

#define _LOAD_FFWRAP_LIB(name)	\
	_YXV_FFWrap::g_fn_##name = (_YXV_FFWrap::func_##name)GetProcAddress(_YXV_FFWrap::g_ffwrap_module, #name);	\
	if (_YXV_FFWrap::g_fn_##name == NULL)	PromptFuncLoadError(#name)

static int _InitDynamicFFWrap()
{
	_YXV_FFWrap::g_ffwrap_module = LoadLibraryW(L"ffwrap.dll");
	if (_YXV_FFWrap::g_ffwrap_module == NULL)
	{
		return 0;
	}

	_LOAD_FFWRAP_LIB(YXV_FFObjConfigV);
	_LOAD_FFWRAP_LIB(YXV_FFObjConfigA);
	_LOAD_FFWRAP_LIB(YXV_FFObjClose);
	_LOAD_FFWRAP_LIB(YXV_FFDecCreate);
	_LOAD_FFWRAP_LIB(YXV_FFDecProcessV);
	_LOAD_FFWRAP_LIB(YXV_FFDecProcessA);
	_LOAD_FFWRAP_LIB(YXV_FFEncCreate);
	_LOAD_FFWRAP_LIB(YXV_FFEncProcessV);
	_LOAD_FFWRAP_LIB(YXV_FFEncProcessA);
	_LOAD_FFWRAP_LIB(YXV_FFEncReadExtV);
	_LOAD_FFWRAP_LIB(YXV_FFEncReadExtA);
	_LOAD_FFWRAP_LIB(YXV_FFFormatCreate);
	_LOAD_FFWRAP_LIB(YXV_FFFormatStartWrite);
	_LOAD_FFWRAP_LIB(YXV_FFFormatWriteV);
	_LOAD_FFWRAP_LIB(YXV_FFFormatWriteA);
	_LOAD_FFWRAP_LIB(YXV_FFFormatEndWrite);
	_LOAD_FFWRAP_LIB(YXV_FFVSpecParamDefault);
	_LOAD_FFWRAP_LIB(DecodeH264SPS);
	_LOAD_FFWRAP_LIB(av_buffer_unref);
	_LOAD_FFWRAP_LIB(av_frame_unref);
	_LOAD_FFWRAP_LIB(av_frame_free);
	_LOAD_FFWRAP_LIB(sws_getCachedContext);
	_LOAD_FFWRAP_LIB(sws_scale);
	_LOAD_FFWRAP_LIB(sws_freeContext);
	_LOAD_FFWRAP_LIB(swr_alloc_set_opts);
	_LOAD_FFWRAP_LIB(swr_init);
	_LOAD_FFWRAP_LIB(swr_convert);
	_LOAD_FFWRAP_LIB(swr_free);

	return 0;
}

int g_wrap_init = _InitDynamicFFWrap();
