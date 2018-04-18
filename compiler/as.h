#ifndef __AS_H_
#define __AS_H_

#include "parser.h"
#include "symbol.h"

#include <iomanip>

typedef vector<TreeNode>::iterator TreeNode_it;

class AS : protected Parser , protected SymTable {
private:
	bool inner_error;

protected:

private:
	void _program(TreeNode* parent);
	void _declBlock(TreeNode* parent);
	void _declaration(TreeNode* parent);
	void _declType(TreeNode* parent);
	void _declVar(TreeNode* parent);
	void _declFunc(TreeNode* parent);
	void _fparaBlock(TreeNode* parent);
	void _fparaList(TreeNode* parent);
	void _fparameter(TreeNode* parent);
	void _statBlock(TreeNode* parent);
	void _innerDeclar(TreeNode *parent);
	void _innerDeclVar(TreeNode *parent);
	void _statString(TreeNode* parent);
	void _statement(TreeNode* parent);
	void _statIf(TreeNode* parent);
	void _statWhile(TreeNode* parent);
	void _statReturn(TreeNode* parent);
	void _statAssign(TreeNode* parent);
	void _expression(TreeNode* parent);
	void _exprArith(TreeNode* parent);
	void _item(TreeNode* parent);
	void _factor(TreeNode* parent);
	void _ftype(TreeNode* parent);
	void _call(TreeNode* parent);
	void _aparameter(TreeNode* parent);
	void _aparaList(TreeNode* parent);

	void advance(TreeNode *parent, TreeNode_it &node_it);

	FunctionSymbol FSYM;	//指當前正翻譯的FunctionSymbol

private:
	void Error_ReDef(Symbol Fvar, const char *ID, int type, int line);
	void Error_UnDef(const char *ID, int line);

public:
	void analyze(string);

};

#endif
