#define PROLOGUE	"pushl %%ebp;movl %%esp, %%ebp"
#define EPILOGUE	"movl %%ebp, %%esp;popl %%ebp;ret"

#define EXPANDF		"subl %0, %%esp"

#define X86_MOVI4	"movl %1, %0"

#define X86_ADDI4	"addl %2, %0"
#define X86_SUBI4	"subl %2, %0"

#define X86_MULI4	"imull %2, %%eax"
#define X86_DIVI4	"cdq;idivl %2"

#define X86_JEI4	"cmpl %2, %1;je %0"
#define X86_JNEI4	"cmpl %2, %1;jne %0"
#define X86_JGI4	"cmpl %2, %1;jg %0"
#define X86_JLI4	"cmpl %2, %1;jl %0"
#define X86_JGEI4	"cmpl %2, %1;jge %0"
#define X86_JLEI4	"cmpl %2, %1;jle %0"

#define X86_JMP		"jmp %0"

#define X86_PUSH	"pushl %0"
#define X86_CALL	"call %1"
#define X86_REDUCEF	"addl %0, %%esp"
