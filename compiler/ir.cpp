#include "ir.h"
#include <stdlib.h>

void IR::printResult()
{
	vector<Symbol>::iterator buckets_it = GlobalIDs.buckets.begin();

	for (; buckets_it != GlobalIDs.buckets.end(); buckets_it++) {
		if ((*buckets_it)->kind == SK_Function) {
			cout << endl;
			cout << "function " << (*buckets_it)->name;
			cout << "(";
			vector<Symbol>::iterator it = ((FunctionSymbol)(*buckets_it))->params.begin();
			for (; it != ((FunctionSymbol)(*buckets_it))->params.end(); it++)
			{
				cout << "int " << (*it)->name;
				if (it + 1 != ((FunctionSymbol)(*buckets_it))->params.end())
					cout << ",";
			}
			cout << ")";
			cout << endl;
			cout << "---------------------" << endl;
			BBlock entryBB = ((FunctionSymbol)(*buckets_it))->entryBB;

			while (true) {
				cout << entryBB->sym->name << ":" << endl;
				IRInst inst = entryBB->insth.next;
				while (inst != &entryBB->insth) {
					cout << "   ";
					if (inst->opcode == $JG)
						cout << "if (" << inst->opds[1]->name << " > " << inst->opds[2]->name << ") goto " << ((BBlock)(inst->opds[0]))->sym->name << endl;
					else if (inst->opcode == $JL)
						cout << "if (" << inst->opds[1]->name << " < " << inst->opds[2]->name << ") goto " << ((BBlock)(inst->opds[0]))->sym->name << endl;
					else if (inst->opcode == $JE)
						cout << "if (" << inst->opds[1]->name << " == " << inst->opds[2]->name << ") goto " << ((BBlock)(inst->opds[0]))->sym->name << endl;
					else if (inst->opcode == $JGE)
						cout << "if (" << inst->opds[1]->name << " >= " << inst->opds[2]->name << ") goto " << ((BBlock)(inst->opds[0]))->sym->name << endl;
					else if (inst->opcode == $JLE)
						cout << "if (" << inst->opds[1]->name << " <= " << inst->opds[2]->name << ") goto " << ((BBlock)(inst->opds[0]))->sym->name << endl;
					else if (inst->opcode == $JNE)
						cout << "if (" << inst->opds[1]->name << " != " << inst->opds[2]->name << ") goto " << ((BBlock)(inst->opds[0]))->sym->name << endl;
					else if (inst->opcode == $JMP)
						cout << "goto " << ((BBlock)(inst->opds[0]))->sym->name << endl;
					else if (inst->opcode == $MOV)
						cout << inst->opds[0]->name << " = " << inst->opds[1]->name << endl;
					else if (inst->opcode == $ADD)
						cout << inst->opds[0]->name << " = " << inst->opds[1]->name << "+" << inst->opds[2]->name << endl;
					else if (inst->opcode == $SUB)
						cout << inst->opds[0]->name << " = " << inst->opds[1]->name << "-" << inst->opds[2]->name << endl;
					else if (inst->opcode == $MUL)
						cout << inst->opds[0]->name << " = " << inst->opds[1]->name << "*" << inst->opds[2]->name << endl;
					else if (inst->opcode == $DIV)
						cout << inst->opds[0]->name << " = " << inst->opds[1]->name << "/" << inst->opds[2]->name << endl;
					else if (inst->opcode == $CALL) {
						cout << inst->opds[0]->name << ": " << inst->opds[1]->name << "(";
						vector<Symbol>::iterator it = ((ParameterList)(inst->opds[2]))->args.begin();
						for (; it != ((ParameterList)(inst->opds[2]))->args.end(); it++) {
							cout << (*it)->name;
							if (it + 1 != ((ParameterList)(inst->opds[2]))->args.end())
								cout << ",";
						}
						cout << ")" << endl;
					}
					else if (inst->opcode == $RETU)
						cout << "return " << inst->opds[0]->name << endl;
					else if (inst->opcode == $RET)
						cout << "ret " << endl;

					inst = inst->next;
				}

				if (entryBB != ((FunctionSymbol)(*buckets_it))->exitBB) {
					entryBB = entryBB->next;
				}
				else
					break;

			}
		}
	}
}

void IR::advance(TreeNode *parent, TreeNode_it &node_it)
{
	if (node_it != parent->children.end()) node_it++;
}

void IR::_program(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $DeclBlock)
	{
		_declBlock(&(*node_it));
	}
}

void IR::_declBlock(TreeNode* parent)
{
	TreeNode_it node_it = parent->children.begin();

	for (; node_it != parent->children.end(); ++node_it) {
		if (node_it->nt_symbol == $Declaration) {
			_declaration(&(*node_it));
		}
	}
}

void IR::_declaration(TreeNode* parent)
{
	TreeNode_it node_it = parent->children.begin();
		
	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $INT) {
		string ID;
		advance(parent, node_it);
		if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $ID) {
			advance(parent, node_it);

			if (node_it->nt_symbol == $DeclType) {
				if (node_it->children.begin()->nt_symbol == $DeclVar) {
					
				}
				else if (node_it->children.begin()->nt_symbol == $DeclFunc) {
					FSYM = (FunctionSymbol)node_it->val;
				}

				_declType(&(*node_it));
			}
		}
	} else if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $VOID) {
		advance(parent, node_it);

		FSYM = (FunctionSymbol)node_it->val;

		advance(parent, node_it);
		_declFunc(&(*node_it));	
	}

}

void IR::_declType(TreeNode* parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $DeclVar) {
		_declVar(&(*node_it));
	}
	else if (node_it->nt_symbol == $DeclFunc) {
		_declFunc(&(*node_it));
	}
}

void IR::_declVar(TreeNode* parent)
{

}

void IR::_declFunc(TreeNode* parent)
{
	EnterScope();
	
	// 申請函數入口BBlock
	FSYM->entryBB = CreateBBlock();

	// 令當前块(BBlock)指向entryBB
	StartBBlock(FSYM->entryBB);

	TreeNode_it node_it = parent->children.begin();
	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $LPAREN) {
		advance(parent, node_it);

		if (node_it->nt_symbol == $FparaBlock) {
			_fparaBlock(&(*node_it));
			advance(parent, node_it);

			if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $RPAREN) {
				advance(parent, node_it);

				if (node_it->nt_symbol == $StatBlock) {
					_statBlock(&(*node_it));
					advance(parent, node_it);
				}
			}
		}
	}

	
	// 令當前块(BBlock)指向exitBB
	FSYM->exitBB = GetCurrentBB();
	GenerateRet();

	// 為每一個块申請Label
	BBlock bb;
	bb = FSYM->entryBB;
	while (bb != NULL)
	{
		// to show the basic blocks more accurately	
		bb->sym = CreateLabel();
		bb = bb->next;
	}

	ExitScope();
}

void IR::_fparaBlock(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	
	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $VOID) {
		return;
	}
	else if (node_it->nt_symbol == $FpapraList)
		_fparaList(&(*node_it));
}

void IR::_fparaList(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	while (true)
	{
		if (node_it->nt_symbol == $Fparameter) {
			_fparameter(&(*node_it));

			advance(parent, node_it);
			if (node_it != parent->children.end() && node_it->t_symbol == $COMMA) {
				advance(parent, node_it);
			} else {
				break;
			}
		}
	}
}

void IR::_fparameter(TreeNode *parent)
{
	/*
	@ 形參部分
	*/
}

void IR::_statBlock(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $LCURLY) {
		advance(parent, node_it);

		if (node_it->nt_symbol == $InnerDeclar) {
			_innerDeclar(&(*node_it));
			advance(parent, node_it);

			if (node_it->nt_symbol == $StatString) {
				_statString(&(*node_it));
				advance(parent, node_it);

				if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $RCURLY) {
					advance(parent, node_it);
				}
			}
		}
	}
}

void IR::_innerDeclar(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	while (node_it != parent->children.end()) {
		if (node_it->nt_symbol == $InnerDeclVar) {
			_innerDeclVar(&(*node_it));
			advance(parent, node_it);

			if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $SEMICOLON) {
				advance(parent, node_it);
			}
		}
		else {
			break;
		}
	}

}

void IR::_innerDeclVar(TreeNode *parent)
{
	/*
	@ 局部變量部分
	*/
}

void IR::_statString(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	for (; node_it != parent->children.end(); node_it++) {
		_statement(&(*node_it));
	}
}

void IR::_statement(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $StatAssign) {
		_statAssign(&(*node_it));
	}
	else if (node_it->nt_symbol == $StatWhile) {
		_statWhile(&(*node_it));
	}
	else if (node_it->nt_symbol == $StatIf) {
		_statIf(&(*node_it));
	}
	else if (node_it->nt_symbol == $StatReturn) {
		_statReturn(&(*node_it));
	}
}

void IR::_statAssign(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	Symbol dst, src;

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $ID) {
		dst = node_it->val;

		advance(parent, node_it);

		if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $ASSIGN) {
			advance(parent, node_it);

			if (node_it->nt_symbol == $Expression) {
				src = _expression(&(*node_it));

				if (dst != NULL && src != NULL) {
					GenerateMove(dst, src);
				}
			}
		}
	}
}

void IR::_statWhile(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $WHILE) {
		advance(parent, node_it);

		if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $LPAREN) {
			advance(parent, node_it);

			if (node_it->nt_symbol == $Expression) {
				TreeNode_it cont = node_it;
				advance(parent, node_it);

				if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $RPAREN) {
					advance(parent, node_it);

					if (node_it->nt_symbol == $StatBlock) {
						BBlock loopBB, contBB, nextBB;

						// create loop, continue, next Block
						loopBB = CreateBBlock();
						contBB = CreateBBlock();
						nextBB = CreateBBlock();

						GenerateJump(contBB);
						// loop 
						StartBBlock(loopBB);
						_statBlock(&(*node_it));
						// continue
						StartBBlock(contBB);
						loopBB = _Whileexpression(&(*cont), loopBB);

						StartBBlock(nextBB);
						advance(parent, node_it);
					}
				}
			}
		}
	}
}

void IR::_statIf(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $IF) {
		advance(parent, node_it);
		if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $LPAREN) {
			advance(parent, node_it);
			if (node_it->nt_symbol == $Expression) {
				TreeNode_it cont = node_it;
				advance(parent, node_it);


				if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $RPAREN) {
					advance(parent, node_it);

					if (node_it->nt_symbol == $StatBlock) {
						BBlock nextBB, trueBB, falseBB;
						nextBB = CreateBBlock();
						trueBB = CreateBBlock();
						TreeNode_it truestat = node_it;

						advance(parent, node_it);

						if (node_it != parent->children.end() && node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $ELSE) {
							advance(parent, node_it);

							if (node_it->nt_symbol == $StatBlock) {
								/*
								 *		if !expr goto falseBB
								 * trueBB:
								 *		stmt1
								 *		goto nextBB
								 * falseBB :
								 *		stmt2
								 * nextBB :
								 *		...
								 */
								TreeNode_it falsestat = node_it;
								falseBB = CreateBBlock();

								falseBB = _Ifexpression(&(*cont), falseBB);
								// true
								StartBBlock(trueBB);
								_statBlock(&(*truestat));
								GenerateJump(nextBB);
								// flase
								StartBBlock(falseBB);
								_statBlock(&(*falsestat));
								// next
								StartBBlock(nextBB);
							}
						}
						else {
							/*
							 *		if !expr goto nextBB
							 *		goto nextBB
							 * trueBB:
							 *		stmt1
							 * nextBB :
							 *		...
							 */
							nextBB = _Ifexpression(&(*cont), nextBB);
						
							// true;
							StartBBlock(trueBB);
							_statBlock(&(*truestat));
							// next
							StartBBlock(nextBB);

						}
					}
				}
			}
		}
	}
}

void IR::_statReturn(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol dst;

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $RETURN) {
		advance(parent, node_it);

		if (node_it->nt_symbol == $Expression) {
			dst = _expression(&(*node_it));
			advance(parent, node_it);

			if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $SEMICOLON) {
				GenerateReturn(dst);
			}
		}
	}
}

BBlock IR::_Ifexpression(TreeNode *parent, BBlock trueBB)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol dst, src1, src2;

	if (node_it->nt_symbol == $ExprArith) {
		dst = _exprArith(&(*node_it));
		advance(parent, node_it);

		while (node_it != parent->children.end()) {
			int opcode;
			string op = node_it->token;

			if (op == "==")
				opcode = $JNE;
			else if (op == "!=")
				opcode = $JE;
			else if (op == ">")
				opcode = $JLE;
			else if (op == "<")
				opcode = $JGE;
			else if (op == ">=")
				opcode = $JL;
			else if (op == "<=")
				opcode = $JG;
			else
				opcode = $NOP;

			advance(parent, node_it);

			if (node_it->nt_symbol == $ExprArith) {

				src2 = _exprArith(&(*node_it));
				src1 = dst;

				GenerateBranch(trueBB, opcode, src1, src2);
				advance(parent, node_it);
				return trueBB;
			}
		}
	}
	return NULL;
}

BBlock IR::_Whileexpression(TreeNode *parent, BBlock loopBB)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol dst, src1, src2;

	if (node_it->nt_symbol == $ExprArith) {
		dst = _exprArith(&(*node_it));
		advance(parent, node_it);

		while (node_it != parent->children.end()) {
			int opcode;
			string op = node_it->token;

			if (op == "==")
				opcode = $JE;
			else if (op == "!=")
				opcode = $JNE;
			else if (op == ">")
				opcode = $JG;
			else if (op == "<")
				opcode = $JL;
			else if (op == ">=")
				opcode = $JGE;
			else if (op == "<=")
				opcode = $JLE;
			else
				opcode = $NOP;

			advance(parent, node_it);

			if (node_it->nt_symbol == $ExprArith) {

				src2 = _exprArith(&(*node_it));
				src1 = dst;

				GenerateBranch(loopBB, opcode, src1, src2);
				advance(parent, node_it);
				return loopBB;
			}
		}
	}
	return NULL;
}

Symbol IR::_expression(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol dst, src1, src2;

	if (node_it->nt_symbol == $ExprArith) {
		dst = _exprArith(&(*node_it));
		advance(parent, node_it);

		while (node_it != parent->children.end()) {
			int opcode;
			string op = node_it->token;

			if (op == "==")
				opcode = $JE;
			else if (op == "!=")
				opcode = $JNE;
			else if (op == ">")
				opcode = $JG;
			else if (op == "<")
				opcode = $JL;
			else if (op == ">=")
				opcode = $JGE;
			else if (op == "<=")
				opcode = $JLE;
			else if (op == "<")
				opcode = $JLE;
			else
				opcode = $NOP;

			advance(parent, node_it);

			if (node_it->nt_symbol == $ExprArith) {
				BBlock nextBB, trueBB, falseBB;

				nextBB = CreateBBlock();
				trueBB = CreateBBlock();
				falseBB = CreateBBlock();

				src2 = _exprArith(&(*node_it));
				src1 = dst;
				dst = CreateTemp();

				GenerateBranch(trueBB, opcode, src1, src2);
				
				// t = 0
				StartBBlock(falseBB);
				GenerateMove(dst, IntConstant(0));
				GenerateJump(nextBB);
				StartBBlock(trueBB);
				// t = 1
				GenerateMove(dst, IntConstant(1));
				StartBBlock(nextBB);

				advance(parent, node_it);
				
			}
		}
	}
	return dst;
}

Symbol IR::_exprArith(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol dst, src1, src2;

	if (node_it->nt_symbol == $Item) {
		dst = _item(&(*node_it));
		advance(parent, node_it);
	}

	while (node_it!=parent->children.end()) {
		int opcode;
		string op = node_it->token;

		if (op == "+")
			opcode = $ADD;
		else if (op == "-")
			opcode = $SUB;

		advance(parent, node_it);

		if (node_it->nt_symbol == $Item) {
			src2 = _item(&(*node_it));
			src1 = dst;
			dst = CreateTemp();

			GenerateAssign(dst, opcode, src1, src2);
			advance(parent, node_it);
		}
	}

	return dst;
}

Symbol IR::_item(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol dst, src1, src2;

	if (node_it->nt_symbol == $Factor) {
		dst = _factor(&(*node_it));
		node_it++;
	}

	while (node_it != parent->children.end()) {
		int opcode;
		string op = node_it->token;

		if (op == "*")
			opcode = $MUL;
		else if (op == "/")
			opcode = $DIV;

		advance(parent, node_it);
		if (node_it->nt_symbol == $Factor) {
			src2 = _factor(&(*node_it));
			src1 = dst;
			dst = CreateTemp();

			GenerateAssign(dst, opcode, src1, src2);
			advance(parent, node_it);
		}
	}
	
	return dst;
}

Symbol IR::_factor(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol sym, recv;

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $NUM) {
		sym = node_it->val;
	}
	else if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $ID) {
		sym = node_it->val;

		advance(parent, node_it);

		if (node_it != parent->children.end() && node_it->nt_symbol == $Ftype) {
			if (sym->kind == SK_Function)
			{
				ParameterList list;

				list = _ftype(&(*node_it));
				FSYM->arguments = (list->npara > FSYM->arguments) ? list->npara : FSYM->arguments;

				recv = CreateTemp();
				GenerateFunctionCall(recv, sym, list);
				sym = recv;
				
			}
		}
	}
	else if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $LPAREN) {
		advance(parent, node_it);
		if (node_it->nt_symbol == $Expression) {
			sym = _expression(&(*node_it));
		}
	}
	return sym;
}

ParameterList IR::_ftype(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	ParameterList list;

	if (node_it != parent->children.end() && node_it->nt_symbol == $Call) {
		list = _call(&(*node_it));
	}

	return list;
}

ParameterList IR::_call(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	ParameterList list;

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $LPAREN) {
		advance(parent, node_it);

		if (node_it->nt_symbol == $Aparameter) {
			list = _aparameter(&(*node_it));

			if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $RPAREN) {
				advance(parent, node_it);
			}
		}
	}

	return list;
}

ParameterList IR::_aparameter(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	ParameterList list;
	
	if (node_it != parent->children.end() && node_it->nt_symbol == $AparaList) {
		// 含有參數表
		list = _aparaList(&(*node_it));
	}
	
	return list;
}

ParameterList IR::_aparaList(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol sym;
	ParameterList list = new parameterlist;
	
	while (node_it != parent->children.end()) {
		string arg;
		if (node_it->nt_symbol == $Expression) {
			sym = _expression(&(*node_it));

			list->args.push_back(sym);
			list->npara++;
		}
		advance(parent, node_it);
	}

	return list;
}

void IR::analyze(string input)
{
	AS::analyze(input);

	if (synTree.nt_symbol == $Program)
	{
		// start 
		_program(&synTree);
	}
}
