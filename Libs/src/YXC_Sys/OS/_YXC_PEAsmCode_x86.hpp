#ifndef __INNER_INC_YXC_PE_ASSEMBLY_CODE_X86_HPP__
#define __INNER_INC_YXC_PE_ASSEMBLY_CODE_X86_HPP__

#include <YXC_Sys/YXC_Types.h>

#define YXC_X86_REG_EAX 0
#define YXC_X86_REG_ECX 1
#define YXC_X86_REG_EDX 2
#define YXC_X86_REG_EBX 3
#define YXC_X86_REG_ESP 4
#define YXC_X86_REG_EBP 5
#define YXC_X86_REG_ESI 6
#define YXC_X86_REG_EDI 7

#define YXC_X86_ADD_BYTE_SEG_REG 0x00
#define YXC_X86_ADD_SEG_REG 0x01
#define YXC_X86_ADD_BYTE_REG_SEG 0x02
#define YXC_X86_ADD_REG_SEG 0x03
#define YXC_X86_ADD_AL 0x04
#define YXC_X86_ADD_EAX 0x05

#define YXC_X86_PUSH_ES 0x06
#define YXC_X86_POP_ES 0x07

#define YXC_X86_OR_BYTE_SEG_REG 0x08
#define YXC_X86_OR_SEG_REG 0x09
#define YXC_X86_OR_BYTE_REG_SEG 0x0A
#define YXC_X86_OR_REG_SEG 0x0B
#define YXC_X86_OR_AL 0x0C
#define YXC_X86_OR_EAX 0x0D

#define YXC_X86_PUSH_CS 0x0E
#define YXC_X86_EXTENDED 0x0F // document needed.

#define YXC_X86_ADC_BYTE_SEG_REG 0x10
#define YXC_X86_ADC_SEG_REG 0x11
#define YXC_X86_ADC_BYTE_REG_SEG 0x12
#define YXC_X86_ADC_REG_SEG 0x13
#define YXC_X86_ADC_AL 0x14
#define YXC_X86_ADC_EAX 0x15

#define YXC_X86_PUSH_SS 0x16
#define YXC_X86_POP_SS 0x17

#define YXC_X86_SBB_BYTE_SEG_REG 0x18
#define YXC_X86_SBB_SEG_REG 0x19
#define YXC_X86_SBB_BYTE_REG_SEG 0x1A
#define YXC_X86_SBB_REG_SEG 0x1B
#define YXC_X86_SBB_AL 0x1C
#define YXC_X86_SBB_EAX 0x1D

#define YXC_X86_PUSH_DS 0x1E
#define YXC_X86_POP_DS  0x1F

#define YXC_X86_AND_BYTE_SEG_REG 0x20
#define YXC_X86_AND_SEG_REG 0x21
#define YXC_X86_AND_BYTE_REG_SEG 0x22
#define YXC_X86_AND_REG_SEG 0x23
#define YXC_X86_AND_AL 0x24
#define YXC_X86_AND_EAX 0x25

#define YXC_X86_PREFIX_ES 0x26 // need investigate.

// BCD Convert on add
#define YXC_X86_DAA     0x27

#define YXC_X86_SUB_BYTE_SEG_REG 0x28
#define YXC_X86_SUB_SEG_REG 0x29
#define YXC_X86_SUB_BYTE_REG_SEG 0x2A
#define YXC_X86_SUB_REG_SEG 0x2B
#define YXC_X86_SUB_AL 0x2C
#define YXC_X86_SUB_EAX 0x2D

#define YXC_X86_PREFIX_CS 0x2E

// BCD Convert on subtract
#define YXC_X86_DAS 0x2F

#define YXC_X86_XOR_BYTE_SEG_REG 0x30
#define YXC_X86_XOR_SEG_REG 0x31
#define YXC_X86_XOR_BYTE_REG_SEG 0x32
#define YXC_X86_XOR_REG_SEG 0x33
#define YXC_X86_XOR_AL 0x34
#define YXC_X86_XOR_EAX 0x35

#define YXC_X86_PREFIX_SS 0x36

// ASCII Convert on add
#define YXC_X86_AAA 0x37

#define YXC_X86_CMP_BYTE_SEG_REG 0x38
#define YXC_X86_CMP_SEG_REG 0x39
#define YXC_X86_CMP_BYTE_REG_SEG 0x3A
#define YXC_X86_CMP_REG_SEG 0x3B
#define YXC_X86_CMP_AL 0x3C
#define YXC_X86_CMP_EAX 0x3D

#define YXC_X86_PREFIX_DS 0x3E

// ASCII Convert on subtract
#define YXC_X86_AAS 0x3F

#define YXC_X86_INC_EAX 0x40
#define YXC_X86_INC_ECX 0x41
#define YXC_X86_INC_EDX 0x42
#define YXC_X86_INC_EBX 0x43
#define YXC_X86_INC_ESP 0x44
#define YXC_X86_INC_EBP 0x45
#define YXC_X86_INC_ESI 0x46
#define YXC_X86_INC_EDI 0x47
#define YXC_X86_DEC_EAX 0x48
#define YXC_X86_DEC_ECX 0x49
#define YXC_X86_DEC_EDX 0x4A
#define YXC_X86_DEC_EBX 0x4B
#define YXC_X86_DEC_ESP 0x4C
#define YXC_X86_DEC_EBP 0x4D
#define YXC_X86_DEC_ESI 0x4E
#define YXC_X86_DEC_EDI 0x4F
#define YXC_X86_PUSH_EAX 0x50
#define YXC_X86_PUSH_ECX 0x51
#define YXC_X86_PUSH_EDX 0x52
#define YXC_X86_PUSH_EBX 0x53
#define YXC_X86_PUSH_ESP 0x54
#define YXC_X86_PUSH_EBP 0x55
#define YXC_X86_PUSH_ESI 0x56
#define YXC_X86_PUSH_EDI 0x57
#define YXC_X86_POP_EAX 0x58
#define YXC_X86_POP_ECX 0x59
#define YXC_X86_POP_EDX 0x5A
#define YXC_X86_POP_EBX 0x5B
#define YXC_X86_POP_ESP 0x5C
#define YXC_X86_POP_EBP 0x5D
#define YXC_X86_POP_ESI 0x5E
#define YXC_X86_POP_EDI 0x5F

#define YXC_X86_PUSHAD 0x60
#define YXC_X86_POPAD  0x61

// BOUND [REG], [REG / SEG]
#define YXC_X86_BOUND 0x62

// ARPL [REG], [REG / REG]
#define YXC_X86_ARPL_WORD_SEG_REG 0x63

#define YXC_X86_PREFIX_FS 0x64
#define YXC_X86_PREFIX_GS 0x65

#define YXC_X86_PREFIX_WORD 0x66
#define YXC_X86_PREFIX_WORD_REG 0x67

// 0x68 000D2277 : PUSH 77220D00
#define YXC_X86_PUSH 0x68

// need document
#define YXC_X86_IMUL_DWORD_NUM 0x69
#define YXC_X86_PUSH_BYTE 0x6A
#define YXC_X86_IMUL_BYTE_NUM 0x6B
#define YXC_X86_INS_BYTE 0x6C
#define YXC_X86_INS_DWORD 0x6D
#define YXC_X86_OUTS_BYTE 0x6E
#define YXC_X86_OUTS_DWORD 0x6F

#define YXC_X86_JO_SHORT 0x70
#define YXC_X86_JNO_SHORT 0x71
#define YXC_X86_JB_SHORT 0x72
#define YXC_X86_JNB_SHORT 0x73
#define YXC_X86_JE_SHORT 0x74
#define YXC_X86_JNE_SHORT 0x75
#define YXC_X86_JBE_SHORT 0x76
#define YXC_X86_JA_SHORT 0x77
#define YXC_X86_JS_SHORT 0x78
#define YXC_X86_JNS_SHORT 0x79

#define YXC_X86_JPE_SHORT 0x7A
#define YXC_X86_JPO_SHORT 0x7B
#define YXC_X86_JL_SHORT 0x7C
#define YXC_X86_JGE_SHORT 0x7D
#define YXC_X86_JLE_SHORT 0x7E
#define YXC_X86_JG_SHORT 0x7F

#define YXC_X86_ADD_BYTE_SEG_NUM1 0x80
#define YXC_X86_ADD_DWORD_SEG_NUM1 0x81
#define YXC_X86_ADD_BYTE_SEG_NUM2 0x82
#define YXC_X86_ADD_DWORD_SEG_NUM2 0x83

#define YXC_X86_TEST_BYTE_SEG_REG 0x84
#define YXC_X86_TEST_DWORD_SEG_REG 0x85

#define YXC_X86_XCHG_BYTE_SEG_REG 0x86
#define YXC_X86_XCHG_DWORD_SEG_REG 0x87

#define YXC_X86_MOV_BYTE_SEG_REG 0x88
#define YXC_X86_MOV_DWORD_SEG_REG 0x89
#define YXC_X86_MOV_BYTE_REG_SEG 0x8A
// 0x8BFF : MOV EDI, EDI
// 0x8BEC : MOV EBP, ESP
#define YXC_X86_MOV_DWORD_REG_SEG 0x8B

#define YXC_X86_MOV_WORD 0x8C

// 0x8D7D B4 : LEA EDI,[EBP-4C]
// 0x8D4D AC : LEA ECX,[EBP-54]
#define YXC_X86_LEA 0x8D

#define YXC_X86_MOV_SEG_POS 0x8E
#define YXC_X86_POP_SEG 0x8F
#define YXC_X86_NO_OP 0x90
#define YXC_X86_XCHG_EAX_ECX 0x91
#define YXC_X86_XCHG_EAX_EDX 0x92
#define YXC_X86_XCHG_EAX_EBX 0x93
#define YXC_X86_XCHG_EAX_ESP 0x94
#define YXC_X86_XCHG_EAX_EBP 0x95
#define YXC_X86_XCHG_EAX_ESI 0x96
#define YXC_X86_XCHG_EAX_EDI 0x97

// quint16_t expand
#define YXC_X86_CWDE 0x98
#define YXC_X86_CDQ 0x99

#define YXC_X86_FAR_CALL 0x9A
#define YXC_X86_WAIT 0x9B
#define YXC_X86_PUSHFD 0x9C
#define YXC_X86_POPFD 0x9D
#define YXC_X86_SAHF 0x9E
#define YXC_X86_LAHF 0x9F

#define YXC_X86_MOV_TO_AL 0xA0
#define YXC_X86_MOV_TO_EAX 0xA1
#define YXC_X86_MOV_AL_TO 0xA2
#define YXC_X86_MOV_EAX_TO 0xA3

#define YXC_X86_MOVS_BYTE 0xA4
#define YXC_X86_MOVS_DWORD 0xA5
#define YXC_X86_CMPS_BYTE 0xA6
#define YXC_X86_CMPS_DWORD 0xA7

#define YXC_X86_TEST_AL 0xA8
#define YXC_X86_TEST_EAX 0xA9

#define YXC_X86_STOS_BYTE 0xAA
#define YXC_X86_STOS_DWORD 0xAB
#define YXC_X86_LODS_BYTE 0xAC
#define YXC_X86_LODS_DWORD 0xAD
#define YXC_X86_SCAS_BYTE 0xAE
#define YXC_X86_SCAS_DWORD 0xAF

#define YXC_X86_MOV_AL 0xB0
#define YXC_X86_MOV_CL 0xB1
#define YXC_X86_MOV_DL 0xB2
#define YXC_X86_MOV_BL 0xB3
#define YXC_X86_MOV_AH 0xB4
#define YXC_X86_MOV_CH 0xB5
#define YXC_X86_MOV_DH 0xB6
#define YXC_X86_MOV_BH 0xB7
#define YXC_X86_MOV_EAX 0xB8
#define YXC_X86_MOV_ECX 0xB9
#define YXC_X86_MOV_EDX 0xBA
#define YXC_X86_MOV_EBX 0xBB
#define YXC_X86_MOV_ESP 0xBC
#define YXC_X86_MOV_EBP 0xBD
#define YXC_X86_MOV_ESI 0xBE
#define YXC_X86_MOV_EDI 0xBF

#define YXC_X86_SAR_BYTE_SEG 0xC0
#define YXC_X86_SAR_DWORD_SEG 0xC1

// 0xC2 0800 : RETN 8
#define YXC_X86_RETN_X 0xC2

// 0xC3 : RETN
#define YXC_X86_RETN 0xC3

// 0xC4 :
#define YXC_X86_LES_REG_SEG 0xC4
#define YXC_X86_LDS_REG_SEG 0xC5

#define YXC_X86_MOV_BYTE_SEG 0xC6
#define YXC_X86_MOV_DWORD_SEG 0xC7

#define YXC_X86_ENTER 0xC8
#define YXC_X86_LEAVE 0xC9
#define YXC_X86_RETF_X 0xCA
#define YXC_X86_RETF 0xCB

#define YXC_X86_INT3 0xCC

// 0xCD23 : INT 23
#define YXC_X86_INT 0xCD
#define YXC_X86_INTO 0xCE
#define YXC_X86_IRETD 0xCF

#define YXC_X86_ROL_BYTE_SEG 0xD0
#define YXC_X86_ROL_DWORD_SEG 0xD1
#define YXC_X86_ROL_BYTE_SEG_REG 0xD2
#define YXC_X86_ROL_DWORD_SEG_REG 0xD3

// 0xD404 : AAM 04 ASCII MUL
#define YXC_X86_AAM 0xD4

// 0xD405 : AAM 05 ASCII DIV
#define YXC_X86_AAD 0xD5

#define YXC_X86_SALC 0xD6

// 0xD7 : XLAT BYTE PTR DS:[EBX+AL] lookup table command.
#define YXC_X86_XLAT 0xD7

// 0xD8 : FLOAT OPERATIONS
#define YXC_X86_FLOAT_DWORD_OP1 0xD8
#define YXC_X86_FLOAT_DWORD_OP2 0xD9
#define YXC_X86_FLOAT_DWORD_OP3 0xDA
#define YXC_X86_FLOAT_DWORD_OP4 0xDB

#define YXC_X86_FLOAT_QWORD_OP1 0xDC
#define YXC_X86_FLOAT_QWORD_OP2 0xDD
#define YXC_X86_FLOAT_WORD_OP1 0xDE
#define YXC_X86_FLOAT_WORD_OP2 0xDF
#define YXC_X86_LOOPNZ_SHORT 0xE0
#define YXC_X86_LOOPZ_SHORT 0xE1
#define YXC_X86_LOOP_SHORT 0xE2
#define YXC_X86_JECXZ_SHORT 0xE3

// 0xE427 : IN AL, 27
#define YXC_X86_IN_AL 0xE4
#define YXC_X86_IN_EAX 0xE5
#define YXC_X86_OUT_AL 0xE6
#define YXC_X86_OUT_EAX 0xE7

// 0xE8 80000000 : call (current offset + 5) + 0x80
#define YXC_X86_CALL 0xE8

// 0xE9 80000000 : jmp (current offset + 5) + 0x80
#define YXC_X86_JMP 0xE9

#define YXC_X86_FAR_JMP 0xEA

// 0xEB F6 : JMP SHORT /
#define YXC_X86_SHORT_JMP 0xEB

// I/O command
#define YXC_X86_IN_AL_DX 0xEC
#define YXC_X86_IN_EAX_DX 0xED
#define YXC_X86_OUT_DX_AL 0xEE
#define YXC_X86_OUT_DX_EAX 0xEF

#define YXC_X86_LOCK_PREFIX 0xF0
#define YXC_X86_INT1 0xF1

#define YXC_X86_REPNE_PREFIX 0xF2
#define YXC_X86_REP_PREFIX 0xF3

// STOP
#define YXC_X86_HLT 0xF4

// neg CF
#define YXC_X86_CMC 0xF5

#define YXC_X86_1OP_BYTE_SEG 0xF6
#define YXC_X86_1OP_DWORD_SEG 0xF7

// clear CF
#define YXC_X86_CLC 0xF8

// set CF
#define YXC_X86_STC 0xF9

// clear interrupt table
#define YXC_X86_CLI 0xFA

// set interrupt table
#define YXC_X86_STI 0xFB

// clear DF
#define YXC_X86_CLD 0xFC

// set DF
#define YXC_X86_STD 0xFD

#define YXC_X86_INC_DEC_BYTE_SEG 0xFE
#define YXC_X86_INC_DEC_DWORD_SEG 0xFF

#define YXC_X86_MAX_INSTRUCION_LEN 8

static inline ybool_t _YXC_IsPrefixByte_X86(ybyte_t cByte)
{
	static ybool_t bIsPrefixArray[256] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};
	return bIsPrefixArray[cByte];
}

static inline ybyte_t _YXC_GetInsBytes_X86(ybyte_t cInstruction)
{
	static ybyte_t byBytesArr[256] = {
		255, 255, 255, 255, 2, 5, 1, 1, 255, 255, 255, 255, 2, 5, 1, 0,
		255, 255, 255, 255, 2, 5, 1, 1, 255, 255, 255, 255, 2, 5, 1, 1,
		255, 255, 255, 255, 2, 5, 0, 1, 255, 255, 255, 255, 2, 5, 0, 1,
		255, 255, 255, 255, 2, 5, 0, 1, 255, 255, 255, 255, 2, 5, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 255, 255, 0, 0, 0, 0, 5, 0, 2, 254, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7, 1, 1, 1, 1, 1, // todo : fix
		5, 5, 5, 5, 1, 1, 1, 1, 2, 5, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 5,
		0, 0, 3, 1, 255, 255, 254, 254, 1, 1, 3, 1, 1, 2, 1, 1,
		255, 255, 255, 255, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		2, 2, 2, 2, 2, 5, 2, 5, 5, 5, 7, 2, 1, 1, 1, 1,
		0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 255, 255,
	};
	return byBytesArr[cInstruction];
}

static inline ybyte_t _YXC_GetPostFixLen_X86(ybyte_t byRx)
{
	static ybyte_t byPostFixArr[256] = {
		0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 1, 4, 0, 0, 0,
		0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 1, 4, 0, 0, 0,
		0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 1, 4, 0, 0, 0,
		0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 1, 4, 0, 0, 0,
		1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1,
		1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1,
		1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1,
		1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1,
		4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4,
		4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4,
		4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4,
		4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};
	return byPostFixArr[byRx];
}

static inline void _YXC_GetInstructionLen_X86(
	ybyte_t* pBaseAddr,
	ybyte_t* pByInstructionLen
)
{
	ybyte_t x = *pBaseAddr++;

	ybool_t bIsPrefix = _YXC_IsPrefixByte_X86(x);

	ybyte_t byBaseLen = 0;

	if (bIsPrefix)
	{
		if (x == YXC_X86_PREFIX_WORD || x == YXC_X86_PREFIX_WORD_REG)
		{
			*pByInstructionLen = 0;
			return;
		}

		byBaseLen = 1;

		x = *pBaseAddr++;
		return;
	}

	ybyte_t byLen = _YXC_GetInsBytes_X86(x);
	if (byLen == 0)
	{
		*pByInstructionLen = 0; // Can't parse now.
		return;
	}
	else if (byLen == 255)
	{
		*pByInstructionLen = _YXC_GetPostFixLen_X86(*pBaseAddr) + 2 + byBaseLen;
	}
	else if (byLen == 254)
	{
		*pByInstructionLen = 0; // Can't parse now.
		return;
	}
	else
	{
		*pByInstructionLen = byLen + byBaseLen;
	}
}

//inline void _YXC_GetAssemblyCodeInfoX86(
//	ybyte_t*& pBaseAddr,
//	ybyte_t* pInstructionLen,
//	ybyte_t* pPrefix,
//	ybyte_t* pcInstruction,
//	ybyte_t* pDesc,
//	ybyte_t* pcInfo,
//	yuint32_t* pdwInfo
//)
//{
//	ybyte_t c = *pBaseAddr++;
//
//	if (_YXC_IsPrefixByte(c))
//	{
//		*pPrefix = c;
//		c = *pBaseAddr++;
//	}
//
//	*pcInstruction = c;
//	if (_YXC_IsOneByteIns(c))
//	{
//		*pInstructionLen = 1;
//		return;
//	}
//}

#endif /* __INNER_INC_YXC_X86_PE_ASSEMBLY_CODE_HPP__ */
