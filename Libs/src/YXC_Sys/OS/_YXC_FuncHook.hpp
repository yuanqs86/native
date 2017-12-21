#ifndef __INNER_INC_YXC_SYS_BASE_FUNC_HOOK_HPP__
#define __INNER_INC_YXC_SYS_BASE_FUNC_HOOK_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/OS/_YXC_PEAsmCode_x86.hpp>

#define _YXC_ASM_JMP_SIZE (sizeof(ybyte_t) + sizeof(yuintptr_t))
#define _YXC_ASM_HOOKED_PROC_SIZE (2 * _YXC_ASM_JMP_SIZE + (YXC_X86_MAX_INSTRUCION_LEN - 1))

namespace YXC_Inner
{
	YXC_Status _HookFunc(const ychar* cpszModule, const char* cpszProcName, FARPROC* pOldProc, ybyte_t* pCopiedProcBuffer, FARPROC pNewProc);

	void _UnhookFunc(FARPROC pOldProc, const ybyte_t* pNewBuffer);
}

#endif /* __INNER_INC_YXC_SYS_BASE_FUNC_HOOK_HPP__ */
