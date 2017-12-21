.data

.code 
;eint16_t opcArr1[4] = { opacity, opacity, opacity, opacity };  rsp + 24
;eint16_t opc255[4] = { 255, 255, 255, 255 }; rsp + 16 
;ebyte_t opc255_8[8] = { 255, 255, 255, 255, 255, 255, 255, 255 }; rsp + 8			   		   
;eint16_t opc128[4] = { 128, 128, 128, 128 }; rsp
;void _sse_alpha_blend_yuv420(const ebyte_t* src, ebyte_t* dst, ebyte_t* alpha, int number, ebyte_t opacity);
;src = ecx, dst = edx, alpha = r10, number = r9d, opacity = rsp + 20
_sse_alpha_blend_yuv420 proc
	xor eax, eax;
	mov eax, dword ptr [rsp + 28h];
	
	push r12;
	push r13;
	push r14;
	push rsi;
	sub rsp, 32;
	
	mov r12, rcx; src
	mov r13, rdx; dst
	
	mov r14d, eax;
	shl r14d, 16;
	add r14d, eax;
	mov dword ptr [rsp + 24], r14d;
	mov dword ptr [rsp + 28], r14d;
	
	mov dword ptr [rsp], 00800080h;
	mov dword ptr [rsp + 4], 00800080h;
	mov dword ptr [rsp + 8], 0ffffffffh;
	mov dword ptr [rsp + 12], 0ffffffffh;
	mov dword ptr [rsp + 16], 00FF00FFh;
	mov dword ptr [rsp + 20], 00FF00FFh;

	xor rax, rax;
	mov eax, r9d;
	mov r14, 0;
begin:	 
	movq mm0, qword ptr [r12 + r14];
	movq mm1, qword ptr [r13 + r14];

	mov rsi, r8;
	cmp rsi, 0;
	je set_alpha_to_255;
	add rsi, r14;
	jmp do_work;

set_alpha_to_255:
	lea rsi, qword ptr [rsp + 8];

	;/* mm2 = eint32_t a = (eint32_t)EJ_Round(a_raw * opacity); */
	;/* mm3 = zero */
do_work:  
	movq mm2, qword ptr [rsi];
	pxor mm3, mm3;
	punpcklbw mm2, mm3;
	movq mm4, qword ptr [rsp + 24];
	pmullw mm2, mm4;
	movq mm4, qword ptr [rsp];
	paddusw mm2, mm4;
	psrlw mm2, 8;

	;/* mm4 = src_y_row[0] * a */
	movq mm4, mm1;
	punpcklbw mm4, mm3;
	movq mm5, qword ptr [rsp + 16];
	psubusw mm5, mm2;
	pmullw mm4, mm5;

	;/* mm6 = result. */
	movq mm6, mm0; 
	punpcklbw mm6, mm3;
	pmullw mm6, mm2;
	paddusw mm6, mm4;   
	movq mm4, qword ptr [rsp];
	paddusw mm6, mm4;
	psrlw mm6, 8;

	movq mm2, qword ptr [rsi];

	;/* High 4 bits. */
	punpckhbw mm2, mm3;
	movq mm4, qword ptr [rsp + 24];
	pmullw mm2, mm4;  
	movq mm4, qword ptr [rsp];
	paddusw mm2, mm4;
	psrlw mm2, 8;

	;/* mm4 = src_y_row[0] * a */
	movq mm4, mm1;
	punpckhbw mm4, mm3;
	movq mm5, qword ptr [rsp + 16];
	psubusw mm5, mm2;
	pmullw mm4, mm5;

	;/* mm6 = result. */
	movq mm7, mm0; 
	punpckhbw mm7, mm3;
	pmullw mm7, mm2;
	paddusw mm7, mm4;   
	movq mm4, qword ptr [rsp];
	paddusw mm7, mm4;
	psrlw mm7, 8;

	packuswb mm6, mm7;
	movq qword ptr [r13+r14], mm6;

	add r14, 8;
	cmp r14, rax;
	jl begin;
	
	add rsp, 32;
	pop rsi;
	pop r14;
	pop r13;
	pop r12;
	ret;

_sse_alpha_blend_yuv420 ENDP
end

