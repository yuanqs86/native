.data

.code 
_getstack_top proc

mov rax, gs:[8]
ret

_getstack_top ENDP
end