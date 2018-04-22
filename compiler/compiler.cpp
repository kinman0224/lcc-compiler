#include "compiler.h"
#include "template.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

/*
	@ 輸出到文件用的，包括寫到緩沖區和刷新緩沖
	@ PutString
	@ PutChar
	@ Flush
	@ PutASMCode
*/

void Compiler::SetUpASMfile(char *output)
{

	char name[256], *p = SourceFileName;
	while (*p != '.') {
		name[p - SourceFileName] = *p++;
	}
	name[p - SourceFileName] = '\0';

	if (output == NULL)
		sprintf(ASMFileName, "%s.s", name);
	else
		sprintf(ASMFileName, "%s", output);

	if (ASMFileName) {
		ASMFile = fopen(ASMFileName, "w");
	}
}

void Compiler::PutString(const char *s)
{
	int len = strlen(s);
	int i;
	char *p;

	if (len > BUF_LEN)
	{
		fwrite(s, 1, len, ASMFile);
		return;
	}

	if (len > BUF_LEN - BufferSize)
	{
		Flush();
	}

	p = OutBuffer + BufferSize;
	for (i = 0; i < len; ++i)
	{
		p[i] = s[i];
	}
	BufferSize += len;
}

void Compiler::PutChar(const char c)
{
	if (BufferSize + 1 > BUF_LEN)  
	{                              
		Flush();                   
	}                              
	OutBuffer[BufferSize++] = c;
}

void Compiler::Flush(void)
{
	if (BufferSize != 0)
	{
		fwrite(OutBuffer, 1, BufferSize, ASMFile);
	}
	BufferSize = 0;
}

void Compiler::PutASMCode(const char *str, Symbol opds[])
{
	int i;
	const char *fmt = str;
	PutChar('\t');
	while (*fmt)
	{
		switch (*fmt)
		{
		case ';':
			PutString("\n\t");
			break;

		case '%':
			// 	Linux:
			// TEMPLATE(X86_MOVI4,    "movl %1, %0")
			fmt++;
			if (*fmt == '%')
			{
				PutChar('%');
			}
			else
			{
				i = *fmt - '0';
				if (opds[i]->reg != NULL)
				{
					PutString((opds[i]->reg->name).c_str());
				}
				else
				{
					PutString(GetAccessName(opds[i]).c_str());
				}
			}
			break;

		default:
			PutChar(*fmt);
			break;
		}
		fmt++;
	}
	PutChar('\n');
}

/*
	@ 初始化寄存器
	- SetupRegisters

	@ 分配寄存器
	- AllocateReg
*/
void Compiler::SetupRegisters()
{
	/*
	ESP, EBP are used for stack,
	so we don't have to CreateReg
	*/
	Regs[EAX] = CreateReg("%eax", EAX);
	Regs[EBX] = CreateReg("%ebx", EBX);
	Regs[ECX] = CreateReg("%ecx", ECX);
	Regs[EDX] = CreateReg("%edx", EDX);
	Regs[ESI] = CreateReg("%esi", ESI);
	Regs[EDI] = CreateReg("%edi", EDI);

	Regs[ESP] = NULL;
	Regs[EBP] = NULL;
}

void Compiler::AllocateReg(IRInst inst, int index)
{
	Symbol reg;
	Symbol p;

	p = inst->opds[index];

	// 只分配給臨時變量
	if (p->kind != SK_Temp)
		return;

	// if it is already in a register, mark the register as used by current uil
	if (p->reg != NULL)
	{
		UsedRegs |= 1 << p->reg->val;
		return;
	}
	
	if (index == 0 && SRC1->ref == 1 && SRC1->reg != NULL)
	{
		reg = SRC1->reg;
		reg->link = NULL;

		//Add Var To Reg
		p->link = reg->link;
		reg->link = p;
		p->reg = reg;
		return;
	}

	reg = GetRegInternal();
	if (index != 0)
	{
		Move(reg, p);
	}

	//Add Var To Reg
	p->link = reg->link;
	reg->link = p;
	p->reg = reg;
}

Symbol Compiler::PutInReg(Symbol p)
{
	Symbol reg;
	if (p->reg != NULL)
	{
		return p->reg;
	}
	reg = GetRegInternal();
	Move(reg, p);
	return reg;
}

/*
	@ 函數名稱	:	GetAccessName
	@ 參數		:	Symbol p
	@ 目的		:	返回符號在滙編中使用的名稱
	@ 返回		:	符號的使用名稱
*/

string Compiler::GetAccessName(Symbol p)
{
	if (p->aname != "")
		return p->aname;

	switch (p->kind)
	{
	case SK_Constant:
		p->aname = "$" + p->name;
		break;
	case SK_Variable:
	case SK_Temp:
		if (p->level == 0)
			p->aname = p->name;
		else {
			char tmp[100];
			sprintf(tmp, "%d(%%ebp)", ((VariableSymbol)p)->offset);
			p->aname = tmp;
		}
		break;
	case SK_Label:
		p->aname = p->name;
		break;
	case SK_Function:
		p->aname = p->name;
		break;
	}

	return p->aname;
}

/*

*/
void Compiler::Export(Symbol p)
{
	char tmp[1024];
	sprintf(tmp, ".globl\t_%s\n", p->name.c_str());
	PutString(tmp);
}


void Compiler::LayoutFrame(FunctionSymbol fsym, int fstParamPos)
{
	int offset;

	offset = fstParamPos * PRESERVE_REGS;
	vector<Symbol>::iterator it = fsym->params.begin();
	for (; it != fsym->params.end(); it++) {
		((VariableSymbol)(*it))->offset = offset;
		offset += PRESERVE_REGS;
	}
	offset = 0;
	it = fsym->locals.begin();
	for (; it != fsym->locals.end(); it++) {
		offset += PRESERVE_REGS;
		((VariableSymbol)(*it))->offset = -offset;
	}
}

void Compiler::Move(Symbol dst, Symbol src)
{
	Symbol opds[2];

	opds[0] = dst;
	opds[1] = src;
	PutASMCode(X86_MOVI4, opds);
}

void Compiler::PushArgument(Symbol arg)
{
	PutASMCode(X86_PUSH, &arg);
}


/*
	函數定義的序言
	@ EmitPrologue() 保護棧結構
	@ EmitEpilogue() 恢復棧結構
*/

void Compiler::EmitPrologue(int varsize)
{
	PutASMCode(PROLOGUE, NULL);

	if (varsize != 0) {
		Symbol sym = IntConstant(varsize*sizeof(int));
		PutASMCode(EXPANDF, &sym);
	}
}

void Compiler::EmitEpilogue(int varsize)
{
	PutASMCode(EPILOGUE, NULL);
}

/*
	產生滙編指令
	@ EmitMove
	@ EmitAssign
	@ EmitBranch
	@ EmitJump
	@ EmitCall
	@ EmitReturn
*/
void Compiler::EmitMove(IRInst inst)
{
	if (SRC1->kind == SK_Constant)
	{
		Move(DST, SRC1);
	}
	else
	{
		AllocateReg(inst, 1);
		AllocateReg(inst, 0);
		if (SRC1->reg == NULL && DST->reg == NULL)
		{
			Symbol reg;
			reg = GetRegInternal();
			Move(reg, SRC1);
			Move(DST, reg);
		}
		else
		{
			Move(DST, SRC1);
		}
	}

	if (DST) DST->ref--;
	if (SRC1) SRC1->ref--;
}

void Compiler::EmitAssign(IRInst inst)
{
	switch (OP)
	{
	case $ADD:
	case $SUB:

		AllocateReg(inst, 1);
		AllocateReg(inst, 2);
		AllocateReg(inst, 0);
		//char name[40];
		//sprintf(name, "%s", SRC1->reg->name.c_str());
		//PutString(name);
		
	
		if (DST->reg != SRC1->reg)
		{
			Move(DST, SRC1);
		}
		if (OP == $ADD) PutASMCode(X86_ADDI4, inst->opds);
		if (OP == $SUB) PutASMCode(X86_SUBI4, inst->opds);
		break;

	case $MUL:
	case $DIV:
		if (SRC1->reg == Regs[EAX])
		{
			SpillReg(Regs[EAX]);
		}
		else
		{
			Symbol sym = Regs[EAX]->link;

			if (sym != NULL && sym->ref > 0) {
				Symbol reg = GetRegInternal();
				SpillReg(Regs[EAX]);

				Move(reg, Regs[EAX]);
				sym->reg = reg;
				reg->link = sym;
			}
			else {
				SpillReg(Regs[EAX]);
			}
			
			Move(Regs[EAX], SRC1);
		}
		// SpillReg(Regs[EDX]);
		// 將SRC1擴展後放在EAX與EDX
		UsedRegs = 1 << EAX ;
		if (SRC2->kind == SK_Constant)
		{
			Symbol reg = GetRegInternal();

			Move(reg, SRC2);
			SRC2 = reg;
		}
		else
		{
			AllocateReg(inst, 2);
		}

		if (OP == $MUL) PutASMCode(X86_MULI4, inst->opds);
		if (OP == $DIV) PutASMCode(X86_DIVI4, inst->opds);

		DST->link = Regs[EAX]->link;
		Regs[EAX]->link = DST;
		DST->reg = Regs[EAX];
		break;

	default:
		break;
	}

	if (DST) DST->ref--;
	if (SRC1) SRC1->ref--;
	if (SRC2) SRC2->ref--;
}

void Compiler::EmitBranch(IRInst inst)
{
	BBlock p = (BBlock)DST;
	DST = p->sym;

	if (SRC2) {
		if (SRC2->kind != SK_Constant) {
			SRC1 = PutInReg(SRC1);
		}
	}
	
	if (SRC1->reg != NULL) {
		SRC1 = SRC1->reg;
	}

	SRC1->ref--;

	if (SRC2) {
		SRC2->ref--;
		if (SRC2->reg != NULL) {
			SRC2 = SRC2->reg;
		}
	}

	ClearRegs();

	if (OP == $JE) PutASMCode(X86_JEI4, inst->opds);
	if (OP == $JGE) PutASMCode(X86_JGEI4, inst->opds);
	if (OP == $JG) PutASMCode(X86_JGI4, inst->opds);
	if (OP == $JL) PutASMCode(X86_JLI4, inst->opds);
	if (OP == $JGE) PutASMCode(X86_JGEI4, inst->opds);
	if (OP == $JLE) PutASMCode(X86_JLEI4, inst->opds);

}

void Compiler::EmitJump(IRInst inst)
{
	DST = ((BBlock)DST)->sym;
	PutASMCode(X86_JMP, inst->opds);
}

void Compiler::EmitCall(IRInst inst)
{
	ParameterList argslist = (ParameterList)SRC2;
	Symbol arg;
	int i, stksize = 0;

	for (i = argslist->npara - 1; i >= 0; i--)
	{
		arg = argslist->args[i];
		PushArgument(arg);
		stksize += sizeof(int) * 1;
	}
	// call func
	PutASMCode(X86_CALL, inst->opds);

	// 如果參數不為0, 則返回時需要復原棧頂
	if (stksize != 0) {
		Symbol p;
		p = IntConstant(stksize);
		PutASMCode(X86_REDUCEF, &p);
	}

	if (DST) DST->ref--;
	// 這裡DST 不會 DST == NULL，因為根據要求為ID = call ID.

	//函數返回值放在EAX
	//AllocateReg(inst, 0);
	//if (DST->reg != Regs[EAX])
	//{
	//	Move(DST, Regs[EAX]);
	//}
	DST->reg = Regs[EAX];
}

void Compiler::EmitReturn(IRInst inst)
{
	if (DST->reg != Regs[EAX])
	{
		Move(Regs[EAX], DST);
	}
}

/*

*/
void Compiler::EmitIRInst(IRInst inst) 
{
	switch (inst->opcode)
	{
	case $MOV:
		EmitMove(inst);
		break;
	case $ADD:
	case $SUB:
		EmitAssign(inst);
		break;
	case $MUL:
	case $DIV:
		EmitAssign(inst);
		break;
	case $JE:
	case $JNE:
	case $JG:
	case $JL:
	case $JGE:
	case $JLE:
		EmitBranch(inst);
		break;
	case $JMP:
		EmitJump(inst);
		break;
	case $CALL:
		EmitCall(inst);
		break;
	case $RETU:
		EmitReturn(inst);
	default:
		break;
	}
}

void Compiler::EmitBBlock(BBlock bb)
{
	IRInst inst = bb->insth.next;

	while (inst != &bb->insth)
	{
		UsedRegs = 0;
		EmitIRInst(inst);
		inst = inst->next;
	}

	// 清空寄存器
	ClearRegs();
}

void Compiler::EmitFunction(FunctionSymbol p)
{
	BBlock bb;
	int varsize;

	Export((Symbol)p);
	DefineLabel((Symbol)p);

	LayoutFrame(p, 2);
	/*
		pushl %ebp
		movl %esp, %ebp
	*/
	varsize = p->locals.size();
	EmitPrologue(varsize);
	
	bb = p->entryBB;
	while (true) 
	{
		if (bb->ref != 0) DefineLabel(bb->sym);

		EmitBBlock(bb);

		if (bb != p->exitBB)
			bb = bb->next;
		else
			break;

	}
	/*
		movl %sebp, %esp
		popl %ebp
	*/
	EmitEpilogue(varsize);
}

void Compiler::DefineGlobal(Symbol p)
{
	char tmp[1024];
	sprintf(tmp, "\t.lcomm\t%s\t%d\n", p->name.c_str(), sizeof(int));
	PutString(tmp);
}

void Compiler::DefineLabel(Symbol p)
{
	char tmp[1024];

	sprintf(tmp, "_%s:\n", p->name.c_str());
	PutString(tmp);
}

/*
	Emit Process
	@ BeginProgram
	@ Data_Segment
	@ EmitGlobals
	@ Text_Segment
	@ EmitFunctions
	@ EndProgram
*/

void Compiler::TranslationUnit()
{
	// 始初化寄存器
	SetupRegisters();

	//開始
	BeginProgram();

	//全局變量部分
	Data_Segment();
	EmitGlobals();

	//代碼函部分
	Text_Segment();
	EmitFunctions();

	//結束, 刷新緩沖區
	EndProgram();

	fclose(ASMFile);
}

void Compiler::BeginProgram(void)
{
	// Initialize register symbols to
	// make sure that no register contains data from variables.
	int i = 0;
	for (i = EAX; i <= EDI; i++)
	{
		if (Regs[i] != NULL) {
			Regs[i]->link = NULL;
		}
	}

	PutString("# Code auto-generated by leikinman\n");
}

void Compiler::Data_Segment()
{
	PutString(".section .data\n");
	PutString(".section .bss\n");
}

void Compiler::EmitGlobals(void)
{
	vector<Symbol>::iterator p = GlobalIDs.buckets.begin();

	for (; p != GlobalIDs.buckets.end(); p++) {
		if ((*p)->kind == SK_Variable) {
			DefineGlobal(*p);
		}
	}
}

void Compiler::Text_Segment()
{
	PutString(".section .text\n");
}

void Compiler::EmitFunctions(void)
{
	vector<Symbol>::iterator p = GlobalIDs.buckets.begin();

	for (; p != GlobalIDs.buckets.end(); p++) {
		if ((*p)->kind == SK_Function) {
			EmitFunction((FunctionSymbol)*p);
		}
	}
}

void Compiler::EndProgram(void)
{
	Flush();
}


/*
	
*/
void Compiler::analyze(string filename)
{
	strcpy(SourceFileName, filename.c_str());

	ifstream in(filename, ios::in);
	if (!in)
	{
		cout << "can't not opend " << filename << endl;
		exit(0);
	}
	istreambuf_iterator<char> beg(in), end;
	string input(beg, end);
	in.close();

	IR::analyze(input);
	//IR::printResult();
}

/*
	@ 名稱: emit()
	@ 作用: 生成汇編代碼
	@ 返回:
*/
void Compiler::emit(char *output)
{
	//
	SetUpASMfile(output);

	BufferSize = 0;

	TranslationUnit();
}

/*
	@ 外界接口
*/
void Compiler::Compilation(char *input, char *output)
{
	Error::filename = input;

	analyze(input);
	emit(output);
}
