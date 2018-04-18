#ifndef __GEN_H_
#define __GEN_H_

#include "symbol.h"
#include "opcode.h"

typedef bblock *CFGEdge;

typedef struct irinst
{
	struct irinst *prev;
	struct irinst *next;
	int opcode;
	Symbol opds[3];
} *IRInst;

struct bblock 
{
	struct bblock *prev;
	struct bblock *next;
	Symbol sym;
	// successors
	CFGEdge succs;
	// predecessors
	CFGEdge preds;
	struct irinst insth;
	// number of instructions
	int ninst;
	// number of successors
	int nsucc;
	// number of predecessors
	int npred;
	int ref;
};

typedef struct parameterlist
{
	vector<Symbol> args;
	int npara = 0;
} *ParameterList;

class IRGen {
private:
	BBlock CurrentBB = NULL;	//當前的基本块

public:
	BBlock CreateBBlock(void);
	void   StartBBlock(BBlock bb);
	BBlock GetCurrentBB();

	void AppendInst(IRInst inst);

	void GenerateMove(Symbol dst, Symbol src);
	void GenerateBranch(BBlock dstBB, int opcode, Symbol src1, Symbol src2);
	void GenerateJump(BBlock dstBB);
	void GenerateAssign(Symbol dst, int opcode, Symbol src1, Symbol src2);
	void GenerateFunctionCall(Symbol recv, Symbol faddr, ParameterList args);
	void GenerateReturn(Symbol src);
	void GenerateRet();
};
#endif

