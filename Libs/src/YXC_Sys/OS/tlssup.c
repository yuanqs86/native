#include <windows.h>
#include <winnt.h>
#include <YXC_Sys/YXC_Platform.h>

int _tls_index=0;

#if YXC_IS_32BIT

#pragma data_seg(".tls")
int _tls_start=0;
#pragma data_seg(".tls$ZZZ")
int _tls_end=0;
#pragma data_seg(".CRT$XLA")
int __xl_a=0;
#pragma data_seg(".CRT$XLZ")
int __xl_z=0;

#pragma data_seg(".rdata$T")

extern PIMAGE_TLS_CALLBACK my_tls_callbacktbl[];

IMAGE_TLS_DIRECTORY32 _tls_used[]={(DWORD)&_tls_start,(DWORD)&_tls_end,(DWORD)&_tls_index,(DWORD)my_tls_callbacktbl,0,0};

#else

#pragma data_seg(".tls")
long long _tls_start=0;
#pragma data_seg(".tls$ZZZ")
long long _tls_end=0;
#pragma data_seg(".CRT$XLA")
long long __xl_a=0;
#pragma data_seg(".CRT$XLZ")
long long __xl_z=0;

#pragma data_seg(".rdata$T")

extern PIMAGE_TLS_CALLBACK my_tls_callbacktbl[];

IMAGE_TLS_DIRECTORY64 _tls_used[]={(DWORD)&_tls_start,(DWORD)&_tls_end,(DWORD)&_tls_index,(DWORD)my_tls_callbacktbl,0,0};

#endif /* YXC_IS_32BIT */
