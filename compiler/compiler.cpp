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
	int i;
	for (i = 0; i <= $ra; i++) {
		Regs[i] = NULL;
	}
	
	Regs[$zero] = CreateReg("$zero", $zero);

	Regs[$v0] = CreateReg("$v0", $v0);
	Regs[$v1] = CreateReg("$v1", $v1);

	Regs[$a0] = CreateReg("$a0", $a0);
	Regs[$a1] = CreateReg("$a1", $a1);
	Regs[$a2] = CreateReg("$a2", $a2);
	Regs[$a3] = CreateReg("$a3", $a3);

	Regs[$t0] = CreateReg("$t0", $t0);
	Regs[$t1] = CreateReg("$t1", $t1);
	Regs[$t2] = CreateReg("$t2", $t2);
	Regs[$t3] = CreateReg("$t3", $t3);
	Regs[$t4] = CreateReg("$t4", $t4);
	Regs[$t5] = CreateReg("$t5", $t5);
	Regs[$t6] = CreateReg("$t6", $t6);
	Regs[$t7] = CreateReg("$t7", $t7);
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

	Load(reg, p);
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
		p->aname = p->name;
		break;
	case SK_Variable:
	case SK_Temp:
		if (p->level == 0)
			p->aname = "_" + p->name;
		else {
			char tmp[100];
			sprintf(tmp, "%d($sp)", ((VariableSymbol)p)->offset);
			p->aname = tmp;
		}
		break;
	case SK_Label:
		p->aname = "_" + p->name;
		break;
	case SK_Function:
		p->aname = "_" + p->name;
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

	offset = fsym->arguments * PRESERVE_REGS;
	it = fsym->locals.begin();
	for (; it != fsym->locals.end(); it++) {
		((VariableSymbol)(*it))->offset = offset;
		offset += PRESERVE_REGS;
	}
}

void Compiler::Move(Symbol dst, Symbol src)
{
	Symbol opds[2];

	opds[0] = dst;
	opds[1] = src;
	PutASMCode(MIPS_MOVE, opds);
}

void Compiler::Save(Symbol dst, Symbol src)
{
	Symbol opds[2];

	if (dst->reg != NULL){
		SpillReg(dst->reg);
	}

	opds[0] = dst;
	opds[1] = src;
	PutASMCode(MIPS_SW, opds);
}

void Compiler::Load(Symbol dst, Symbol src)
{
	Symbol opds[2];

	opds[0] = dst;
	opds[1] = src;
	PutASMCode(MIPS_LW, opds);
}

void Compiler::PushArgument(Symbol arg, int n)
{
	if (n > 4) {
		Symbol opds[2];
		opds[0] = Regs[$a0 + n];
		opds[1] = arg;
		if (arg->kind == SK_Variable)
			PutASMCode(MIPS_LW, opds);
		else
			PutASMCode(MIPS_MOVE, opds);
	}
	else {
		// 入棧

	}
}


/*
	函數定義的序言
	@ EmitPrologue() 保護棧結構
	@ EmitEpilogue() 恢復棧結構
*/

void Compiler::EmitPrologue(int varsize)
{
	Symbol sym0 = IntConstant(-varsize*sizeof(int));
	Symbol sym1 = IntConstant(varsize*sizeof(int) - 4);
	Symbol sym2 = IntConstant(varsize*sizeof(int) - 8);
	Symbol sym[3];

	sym[0] = sym0;
	sym[1] = sym1;
	sym[2] = sym2;

	PutASMCode(PROLOGUE, sym);
}

void Compiler::EmitEpilogue(int varsize)
{
	Symbol sym0 = IntConstant(varsize*sizeof(int));
	Symbol sym1 = IntConstant(varsize*sizeof(int) - 4);
	Symbol sym2 = IntConstant(varsize*sizeof(int) - 8);
	Symbol sym[3];

	sym[0] = sym0;
	sym[1] = sym1;
	sym[2] = sym2;

	PutASMCode(EPILOGUE, sym);


	PutChar('\n');
}

void Compiler::EmitArguments(FunctionSymbol p)
{
	int size = (p->params.size() > 4) ? 4 : p->params.size();
	for (int i = 0; i <size ; i++)
	{
		Save(p->params[i], Regs[$a0 + i]);
	}
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
		SpillReg(Regs[$v0]);

		// load im
		Symbol opds[2];
		opds[0] = Regs[$v0];
		opds[1] = SRC1;
		PutASMCode(MIPS_LI, opds);

		if (SRC1->kind != SK_Temp)
		{
			Save(DST, Regs[$v0]);
		}
		else {
			DST->reg = Regs[$v0];
			Regs[$v0]->link = DST;
		}
	}
	else
	{
		AllocateReg(inst, 1);
		AllocateReg(inst, 0);
		if (SRC1->reg == NULL && DST->reg == NULL)
		{
			Load(Regs[$v0], SRC1);
			Save(DST, Regs[$v0]);
		}
		else
		{
			Save(DST, SRC1);
			SpillReg(SRC1->reg);
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
	
		if (SRC1->kind == SK_Constant & SRC2->kind == SK_Constant) {

			// load im
			Symbol opds[2];
			opds[0] = DST;
			opds[1] = SRC1;
			PutASMCode(MIPS_LI, opds);

			opds[0] = DST;
			if (OP == $ADD) opds[1] = SRC1;
			if (OP == $SUB) opds[1] = AddConstant(-SRC1->val);

			PutASMCode(MIPS_ADDIU, opds);

		}
		else if (SRC1->kind == SK_Constant) {
			if (DST->reg != SRC2->reg)
			{
				Load(DST, SRC2);
			}

			Symbol opds[2];
			opds[0] = DST;
			if (OP == $ADD) opds[1] = SRC1;
			if (OP == $SUB) opds[1] = AddConstant(-SRC1->val);

			PutASMCode(MIPS_ADDIU, opds);
		}
		else if (SRC2->kind == SK_Constant) {
			if (DST->reg != SRC1->reg)
			{
				Load(DST, SRC1);
			}

			Symbol opds[2];
			opds[0] = DST;
			if (OP == $ADD) opds[1] = SRC2;
			if (OP == $SUB) opds[1] = AddConstant(-SRC2->val);

			PutASMCode(MIPS_ADDIU, opds);
		}
		else {
			if (DST->reg != SRC1->reg)
			{
				Load(DST, SRC1);
			}
			
			Symbol reg = PutInReg(SRC2);

			Symbol opds[2];
			opds[0] = DST;
			opds[1] = reg;

			if (OP == $ADD) PutASMCode(MIPS_ADDU, opds);
			if (OP == $SUB) PutASMCode(MIPS_SUBU, opds);
		}
		break;

	case $MUL:
	case $DIV:

		AllocateReg(inst, 1);
		AllocateReg(inst, 2);
		AllocateReg(inst, 0);
		if (SRC1->kind == SK_Constant & SRC2->kind == SK_Constant)
		{
			// load im
			Symbol opds[2];
			opds[0] = DST;
			opds[1] = SRC1;
			PutASMCode(MIPS_LI, opds);

			// load im
			Symbol reg = GetRegInternal();
			opds[0] = reg;
			opds[1] = SRC2;
			PutASMCode(MIPS_LI, opds);

			opds[0] = DST;
			opds[1] = reg;
			if (OP == $MUL) PutASMCode(MIPS_MUL, opds);
			if (OP == $DIV) PutASMCode(MIPS_DIV, opds);
		}
		else if (SRC1->kind == SK_Constant)
		{
			if (DST->reg != SRC2->reg)
			{
				Load(DST, SRC2);
			}

			// load im
			Symbol reg = GetRegInternal();
			Symbol opds[2];
			opds[0] = reg;
			opds[1] = SRC1;
			PutASMCode(MIPS_LI, opds);

			opds[0] = DST;
			opds[1] = reg;
			if (OP == $MUL) PutASMCode(MIPS_MUL, opds);
			if (OP == $DIV) PutASMCode(MIPS_DIV, opds);

		}
		else if (SRC2->kind == SK_Constant) 
		{
			if (DST->reg != SRC1->reg)
			{
				Load(DST, SRC1);
			}

			// load im
			Symbol reg = GetRegInternal();
			Symbol opds[2];
			opds[0] = reg;
			opds[1] = SRC2;
			PutASMCode(MIPS_LI, opds);

			opds[0] = DST;
			opds[1] = reg;
			if (OP == $MUL) PutASMCode(MIPS_MUL, opds);
			if (OP == $DIV) PutASMCode(MIPS_DIV, opds);
		}
		else {
			if (DST->reg != SRC1->reg)
			{
				Load(DST, SRC1);
			}

			Symbol reg = PutInReg(SRC2);

			Symbol opds[2];
			opds[0] = DST;
			opds[1] = reg;

			if (OP == $MUL) PutASMCode(MIPS_MUL, opds);
			if (OP == $DIV) PutASMCode(MIPS_DIV, opds);

		}
		
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

	SRC1->ref--;
	SRC2->ref--;
	if (SRC1->kind == SK_Constant & SRC2->kind == SK_Constant) {
		Symbol reg1, reg2;
		Symbol opds[2];

		// load im
		reg1 = GetRegInternal();
		opds[0] = reg1;
		opds[1] = SRC1;

		PutASMCode(MIPS_LI, opds);
		SRC1->reg = reg1;
		reg1->link = SRC1;

		// load im
		reg2 = GetRegInternal();
		opds[0] = reg2;
		opds[1] = SRC2;

		PutASMCode(MIPS_LI, opds);
		SRC2->reg = reg2;
		reg2->link = SRC2;

		if (OP == $JE) {
			PutASMCode(MIPS_BEQ, inst->opds);
		}
		else if (OP == $JNE) {
			PutASMCode(MIPS_BNE, inst->opds);
		}
		else if (OP == $JG) {
			Symbol opds[3];
			opds[0] = SRC2;
			opds[1] = SRC1;
			PutASMCode(MIPS_SLT, opds);

			opds[0] = DST;
			opds[1] = SRC2;
			opds[2] = Regs[$zero];
			PutASMCode(MIPS_BNE, opds);
		}
		else if (OP == $JL) {
			Symbol opds[3];
			opds[0] = SRC1;
			opds[1] = SRC2;
			PutASMCode(MIPS_SLT, opds);

			opds[0] = DST;
			opds[1] = SRC1;
			opds[2] = Regs[$zero];
			PutASMCode(MIPS_BNE, opds);
		}
		else if (OP == $JGE) {
			Symbol opds[3];
			opds[0] = SRC1;
			opds[1] = SRC2;
			PutASMCode(MIPS_SLT, opds);

			opds[0] = DST;
			opds[1] = SRC1;
			opds[2] = Regs[$zero];
			PutASMCode(MIPS_BEQ, opds);
		}
		else if (OP == $JLE) {
			Symbol opds[3];
			opds[0] = SRC2;
			opds[1] = SRC1;
			PutASMCode(MIPS_SLT, opds);

			opds[0] = DST;
			opds[1] = SRC2;
			opds[2] = Regs[$zero];
			PutASMCode(MIPS_BEQ, opds);
		}
	}
	else if (SRC1->kind == SK_Constant) {
		Symbol reg1, reg2;
		Symbol opds[2];

		if (SRC2->reg == NULL) {
			reg2 = GetRegInternal();
			Load(reg2, SRC2);
			SRC2->reg = reg2;
			reg2->link = SRC2;
		}


		// load im
		reg1 = GetRegInternal();
		opds[0] = reg1;
		if (OP != $JL && OP != $JGE) {
			opds[1] = SRC1;
		}
		else
			opds[1] = AddConstant(SRC1->val + 1);

		PutASMCode(MIPS_LI, opds);
		SRC1 = reg1;

		Symbol tempS;
		tempS = SRC1;
		SRC1 = SRC2;
		SRC2 = tempS;
		if (OP == $JE) {
			PutASMCode(MIPS_BEQ, inst->opds);
		}
		else if (OP == $JNE) {
			PutASMCode(MIPS_BNE, inst->opds);
		}
		else if (OP == $JG || OP == $JGE) {
			opds[0] = SRC1;
			opds[1] = SRC2;
			PutASMCode(MIPS_SLT, opds);

			SRC2 = Regs[$zero];
			PutASMCode(MIPS_BNE, inst->opds);
		}
		else if (OP == $JL || OP == $JLE) {
			opds[0] = SRC1;
			opds[1] = SRC2;
			PutASMCode(MIPS_SLT, opds);

			SRC2 = Regs[$zero];
			PutASMCode(MIPS_BEQ, inst->opds);
		}
	} 
	else if (SRC2->kind == SK_Constant) {
		Symbol reg1, reg2;
		Symbol opds[2];

		if (SRC1->reg == NULL) {
			reg1= GetRegInternal();
			Load(reg1, SRC1);
			SRC1->reg = reg1;
			reg1->link = SRC1;
		}
		

		// load im
		reg2 = GetRegInternal();
		opds[0] = reg2;
		if (OP != $JG && OP != $JLE) {
			opds[1] = SRC2;
		}
		else
			opds[1] = AddConstant(SRC2->val + 1);

		PutASMCode(MIPS_LI, opds);
		SRC2 = reg2;
		if (OP == $JE) {
			PutASMCode(MIPS_BEQ, inst->opds);
		}
		else if (OP == $JNE) {
			PutASMCode(MIPS_BNE, inst->opds);
		}
		else if (OP == $JG || OP == $JGE) {
			opds[0] = SRC1;
			opds[1] = SRC2;
			PutASMCode(MIPS_SLT, opds);

			SRC2 = Regs[$zero];
			PutASMCode(MIPS_BEQ, inst->opds);
		}
		else if (OP == $JL || OP == $JLE) {
			opds[0] = SRC1;
			opds[1] = SRC2;
			PutASMCode(MIPS_SLT, opds);

			SRC2 = Regs[$zero];
			PutASMCode(MIPS_BNE, inst->opds);
		}
	}
	else {
		Symbol reg1, reg2;
		if (SRC1->reg == NULL) {
			reg1 = GetRegInternal();
			Load(reg1, SRC1);
			SRC1->reg = reg1;
			reg1->link = SRC1;
		}

		if (SRC2->reg == NULL) {
			reg2 = GetRegInternal();
			Load(reg2, SRC2);
			SRC2->reg = reg2;
			reg2->link = SRC2;
		}

		if (OP == $JE) {
			PutASMCode(MIPS_BEQ, inst->opds);
		}
		else if (OP == $JNE) {
			PutASMCode(MIPS_BNE, inst->opds);
		}
		else if (OP == $JG) {
			Symbol opds[3];
			opds[0] = SRC2;
			opds[1] = SRC1;
			PutASMCode(MIPS_SLT, opds);

			opds[0] = DST;
			opds[1] = SRC2;
			opds[2] = Regs[$zero];
			PutASMCode(MIPS_BNE, opds);
		}
		else if (OP == $JL) {
			Symbol opds[3];
			opds[0] = SRC1;
			opds[1] = SRC2;
			PutASMCode(MIPS_SLT, opds);

			opds[0] = DST;
			opds[1] = SRC1;
			opds[2] = Regs[$zero];
			PutASMCode(MIPS_BNE, opds);
		}
		else if (OP == $JGE) {
			Symbol opds[3];
			opds[0] = SRC1;
			opds[1] = SRC2;
			PutASMCode(MIPS_SLT, opds);

			opds[0] = DST;
			opds[1] = SRC1;
			opds[2] = Regs[$zero];
			PutASMCode(MIPS_BEQ, opds);
		}
		else if (OP == $JLE) {
			Symbol opds[3];
			opds[0] = SRC2;
			opds[1] = SRC1;
			PutASMCode(MIPS_SLT, opds);

			opds[0] = DST;
			opds[1] = SRC2;
			opds[2] = Regs[$zero];
			PutASMCode(MIPS_BEQ, opds);
		}


	}

	ClearRegs();
}

void Compiler::EmitJump(IRInst inst)
{
	DST = ((BBlock)DST)->sym;
	PutASMCode(MIPS_B, inst->opds);
}

void Compiler::EmitCall(IRInst inst)
{
	ParameterList argslist = (ParameterList)SRC2;
	Symbol arg;
	int i, stksize = 0;

	for (i = argslist->npara - 1; i >= 0; i--)
	{
		arg = argslist->args[i];
		PushArgument(arg, i);
		stksize += sizeof(int) * 1;
	}
	// call func
	PutASMCode(MIPS_JAL, inst->opds);

	if (DST) DST->ref--;

	DST->reg = Regs[$v0];
}

void Compiler::EmitReturn(IRInst inst)
{
	if (DST->reg != Regs[$v0])
	{
		Load(Regs[$v0], DST);
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

	varsize = p->locals.size() + p->arguments + 2;

	LayoutFrame(p, varsize);
	/*
		pushl %ebp
		movl %esp, %ebp
	*/
	EmitPrologue(varsize);
	EmitArguments(p);
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
	sprintf(tmp, "\t.globl\t_%s\n", p->name.c_str());
	PutString(tmp);
	sprintf(tmp, "\t_%s: .word 0 \n", p->name.c_str());
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
	for (i = $zero; i <= $ra; i++)
	{
		if (Regs[i] != NULL) {
			Regs[i]->link = NULL;
		}
	}

	PutString("# Code auto-generated by leikinman\n");
	char tmp[100];
	sprintf(tmp, ".file	%s\n", SourceFileName);
	PutString(tmp);
}

void Compiler::Data_Segment()
{
	PutString(".data\n");
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
	PutString(".text\n");
}

void Compiler::EmitFunctions(void)
{
	vector<Symbol>::iterator p;

	for (p = GlobalIDs.buckets.begin(); p != GlobalIDs.buckets.end(); p++) {
		if ((*p)->kind == SK_Function && (*p)->name == "main") {
			PutString("\tj	_main\n");
		}
	}

	for (p = GlobalIDs.buckets.begin(); p != GlobalIDs.buckets.end(); p++) {
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
	sprintf(SourceFileName, "%s", filename.c_str());

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
