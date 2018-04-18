#ifndef __IR_H_
#define __IR_H_

#include "as.h"
#include "gen.h"

#include <iomanip>

typedef vector<TreeNode>::iterator TreeNode_it;

class IR : protected AS , protected IRGen {
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
	BBlock _Ifexpression(TreeNode* parent, BBlock trueBB);
	BBlock _Whileexpression(TreeNode* parent, BBlock trueBB);
	Symbol _expression(TreeNode* parent);
	Symbol _exprArith(TreeNode* parent);
	Symbol _item(TreeNode* parent);
	Symbol _factor(TreeNode* parent);
	ParameterList _ftype(TreeNode* parent);
	ParameterList _call(TreeNode* parent);
	ParameterList _aparameter(TreeNode* parent);
	ParameterList _aparaList(TreeNode* parent);

	void advance(TreeNode *parent, TreeNode_it &node_it);

	FunctionSymbol FSYM;	//指當前正翻譯的FunctionSymbol
public:
	void analyze(string);
	void printResult();
};

#endif
