#define PROLOGUE	"addiu	$sp $sp, %0;sw		$ra, %1($sp);sw		$fp, %2($sp);move	$fp, $sp"
#define EPILOGUE	"move	$fp, $sp;lw		$fp, %2($sp);lw		$ra, %1($sp);addiu	$sp $sp, %0;jr		$ra;nop"

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

#define MIPS_LI		"li		%0, %1"

#define MIPS_SW		"sw		%1, %0"
#define MIPS_LW		"lw		%0, %1"

#define MIPS_MOVE	"move	%0, %1"
#define MIPS_ADDIU	"addiu	%0, %0, %1"
#define MIPS_ADDU	"addu	%0, %1, %0"

#define MIPS_SUBU	"subu	%0, %1, %0"

#define MIPS_MUL	"mul		%0, %0, %1"
#define MIPS_DIV	"div		%0, %0, %1"

#define MIPS_SLL	"sll		%0, %1, %2"

#define MIPS_SLT	"slt		%0, %0, %1"

#define MIPS_BEQ	"beq		%1, %2, %0;nop"		
#define MIPS_BNE	"bne		%1, %2, %0;nop"	

#define MIPS_B		"b		%0"

#define MIPS_JAL	"jal		%1"