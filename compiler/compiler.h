#ifndef __COMPILER_H_
#define __COMPILER_H_

#include "gen.h"
#include "ir.h"
#include "reg.h"

#include <string>
#include <stdio.h>
#include <vector>

#define BUF_LEN  4096
#define PRESERVE_REGS 4

#define OP inst->opcode
#define DST inst->opds[0]
#define SRC1 inst->opds[1]
#define SRC2 inst->opds[2]

class Compiler : protected IR , protected Reg {
private:
	char SourceFileName[256];
	char ASMFileName[256];
	FILE *ASMFile;

	char OutBuffer[BUF_LEN];
	int BufferSize;

private:
	void TranslationUnit();

	// Emit Process
	void BeginProgram();
	void Data_Segment();
	void EmitGlobals();
	void Text_Segment();
	void EmitFunctions();
	void EndProgram();

	string GetAccessName(Symbol p);

	void Export(Symbol p);
	void EmitFunction(FunctionSymbol p);
	void DefineGlobal(Symbol p);
	void DefineLabel(Symbol p);
	void EmitPrologue(int varsize);
	void EmitEpilogue(int varsize);
	void LayoutFrame(FunctionSymbol fsym, int fstParamPos);
	void EmitBBlock(BBlock bb);
	void EmitIRInst(IRInst inst);
	void Move(Symbol dst, Symbol src);
	void PushArgument(Symbol arg);

	// Emit Inst
	void EmitMove(IRInst inst);
	void EmitAssign(IRInst inst);
	void EmitBranch(IRInst inst);
	void EmitJump(IRInst inst);
	void EmitCall(IRInst inst);
	void EmitReturn(IRInst inst);

	// About Registers
	void SetupRegisters();
	void AllocateReg(IRInst inst, int index);
	Symbol PutInReg(Symbol p);

	// About IO stream
	void SetUpASMfile(char *output);
	void PutChar(const char c);
	void PutString(const char *s);
	void Flush();
	void PutASMCode(const char *str, Symbol opds[]);

private:
	void analyze(string input);
	void emit(char *output);

public:
	void Compilation(char *input, char *output = NULL);
};

#endif