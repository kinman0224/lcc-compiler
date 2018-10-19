#include "as.h"
#include <stdlib.h>

#define SET_ERROR	error = true;\
					if (!inner_error) \
						cout << "In function " << "'" << FSYM->name << "':" << endl;\
						inner_error = true;

#define DEBUG_UNDECLARED_PRINT(id) SET_ERROR; cout << "error: " << "'" << id << "'" << " undeclared (first use in this function)" << endl;

void AS::advance(TreeNode *parent, TreeNode_it &node_it)
{
	if (node_it != parent->children.end()) node_it++;
}


void AS::Error_ReDef(Symbol Fvar, const char *ID, int type, int line)
{
	char tmp[1000];

	if (FSYM != NULL && inner_error == false)
	{
		sprintf(tmp, "In function '%s':", FSYM->name.c_str());
		Error::Do_Error(tmp);
		inner_error = true;
	}

	if (Fvar->kind != type) {
		sprintf(tmp, "'%s' redeclared as different kind of symbol", ID);
		Error::Do_Error(tmp, line);
		sprintf(tmp, "previous definition of '%s' was here", ID);
		Error::Do_Error(tmp, Fvar->line);
	}
	else
	{
		sprintf(tmp, "redeclaration of '%s' with no linkage", ID);
		Error::Do_Error(tmp, line);
		sprintf(tmp, "previous definition of '%s' was here", ID);
		Error::Do_Error(tmp, Fvar->line);
	}
}

void AS::Error_UnDef(const char *ID, int line)
{
	char tmp[1000];

	if (FSYM != NULL && inner_error == false)
	{
		sprintf(tmp, "In function '%s':", FSYM->name.c_str());
		Error::Do_Error(tmp);
		inner_error = true;
	}

	sprintf(tmp, "'%s' undeclared (first use in this function)", ID);
	Error::Do_Error(tmp, line);
	sprintf(tmp, "(Each undeclared identifier is reported only once");
	Error::Do_Error(tmp, line);
	sprintf(tmp, "for each function it appears in.)");
	Error::Do_Error(tmp, line);

}

///////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////

void AS::_program(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $DeclBlock)
	{
		_declBlock(&(*node_it));
	}
}

void AS::_declBlock(TreeNode* parent)
{
	TreeNode_it node_it = parent->children.begin();

	for (; node_it != parent->children.end(); ++node_it) {
		if (node_it->nt_symbol == $Declaration) {
			FSYM = NULL;
			_declaration(&(*node_it));
		}
	}
}

void AS::_declaration(TreeNode* parent)
{
	TreeNode_it node_it = parent->children.begin();
	string ID;
	int line;

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $INT) {

		advance(parent, node_it);

		if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $ID) {
			ID = node_it->token;
			line = node_it->line;
			advance(parent, node_it);

			if (node_it->nt_symbol == $DeclType) {
				if (node_it->children.begin()->nt_symbol == $DeclVar) {
					// ȫ��׃��

					// �z���ظ�, ������������׃����
					Symbol var;
					var = LookupID(ID, 0);
					if (var != NULL) {
						Error_ReDef(var, ID.c_str(), SK_Variable, line);
					}
					else {
						var = AddVariable(ID, line);
						// �@�e���ü����Z����
					}
				}
				else if (node_it->children.begin()->nt_symbol == $DeclFunc) {
					// c �]�����d����

					// �z���ظ�, ������������׃����
					Symbol var;
					var = LookupID(ID, 0);
					if (var != NULL) {
						Error_ReDef(var, ID.c_str(), SK_Function, line);
					}

					FSYM = (FunctionSymbol)AddFunction(ID, line);
					node_it->val = (Symbol)FSYM;
				}

				_declType(&(*node_it));
			}
		}
	}
	else if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $VOID) {
		advance(parent, node_it);
		string ID = node_it->token;
		int line = node_it->line;

		Symbol var;
		var = LookupID(ID, 0);
		if (var != NULL) {
			Error_ReDef(var, ID.c_str(), SK_Function, line);
		}

		FSYM = (FunctionSymbol)AddFunction(ID, line);
		node_it->val = (Symbol)FSYM;

		advance(parent, node_it);
		_declFunc(&(*node_it));
	}

}

void AS::_declType(TreeNode* parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $DeclVar) {
		_declVar(&(*node_it));
	}
	else if (node_it->nt_symbol == $DeclFunc) {
		_declFunc(&(*node_it));
	}
}

void AS::_declVar(TreeNode* parent)
{

}

void AS::_declFunc(TreeNode* parent)
{
	inner_error = false;

	EnterScope();

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

	ExitScope();
}

void AS::_fparaBlock(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $VOID) {
		return;
	}
	else if (node_it->nt_symbol == $FpapraList)
		_fparaList(&(*node_it));
}

void AS::_fparaList(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	while (true)
	{
		if (node_it->nt_symbol == $Fparameter) {
			_fparameter(&(*node_it));

			advance(parent, node_it);
			if (node_it != parent->children.end() && node_it->t_symbol == $COMMA) {
				advance(parent, node_it);
			}
			else {
				break;
			}
		}
	}
}

void AS::_fparameter(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	advance(parent, node_it);

	string ID = node_it->token;
	int line = node_it->line;
	Symbol var = AddVariable(ID, line);

	FSYM->params.push_back(var);
}

void AS::_statBlock(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $LCURLY) {
		advance(parent, node_it);

		//a new level table
		EnterScope();

		if (node_it->nt_symbol == $InnerDeclar) {
			// �ֲ�׃����
			_innerDeclar(&(*node_it));
			advance(parent, node_it);

			if (node_it->nt_symbol == $StatString) {
				_statString(&(*node_it));
				advance(parent, node_it);

				if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $RCURLY) {
					advance(parent, node_it);

					// back to the table before
					ExitScope();
				}
			}
		}
	}
}

void AS::_innerDeclar(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	while (node_it != parent->children.end()) {
		if (node_it->nt_symbol == $InnerDeclVar) {
			_innerDeclVar(&(*node_it));
			advance(parent, node_it);

			if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $SEMICOLON)
			{
				advance(parent, node_it);
			}
		}
		else {
			break;
		}
	}

}

void AS::_innerDeclVar(TreeNode *parent)
{
	/*
	@ �ֲ�׃������
	@ Ŀ��: ��ǰ������׃�������ֲ�׃��
	*/

	TreeNode_it node_it = parent->children.begin();

	// TYPE: int
	advance(parent, node_it);

	// ����̖����Ոһ����׃��������׃���ĵ�ַ
	string ID = node_it->token;
	int line = node_it->line;

	Symbol var;
	var = LookupID(ID, 0);
	if (var != NULL) {
		Error_ReDef(var, ID.c_str(), SK_Variable, line);
	}
	else {
		var = AddVariable(ID, line);
		
		// ��ͬ���Z�����в���
		// �ڮ�ǰ������׃�����м���ֲ�׃��
		FSYM->locals.push_back(var);
	}
}

void AS::_statString(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	for (; node_it != parent->children.end(); node_it++) {
		_statement(&(*node_it));
	}
}

void AS::_statement(TreeNode *parent)
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

void AS::_statAssign(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	Symbol dst;

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $ID) {
		string id = node_it->token;
		int line = node_it->line;
		dst = LookupID(id);

		// ���̖���Л]��
		if (dst == NULL) {
			Error_UnDef(id.c_str(), line);

			//�]�Єt�����̖��
			dst = AddVariable(id, line);
		}

		// �����Z����
		node_it->val = dst;

		advance(parent, node_it);

		if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $ASSIGN) {
			advance(parent, node_it);

			if (node_it->nt_symbol == $Expression) {
				_expression(&(*node_it));
			}
		}
	}

}

void AS::_statWhile(TreeNode *parent)
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
						// loop 
						_statBlock(&(*node_it));
						// continue
						_expression(&(*cont));

						advance(parent, node_it);
					}
				}
			}
		}
	}
}

void AS::_statIf(TreeNode *parent)
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
						TreeNode_it truestat = node_it;

						advance(parent, node_it);

						if (node_it != parent->children.end() && node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $ELSE) {
							advance(parent, node_it);

							if (node_it->nt_symbol == $StatBlock) {
								TreeNode_it falsestat = node_it;

								_expression(&(*cont));
								// true
								_statBlock(&(*truestat));
								// flase
								_statBlock(&(*falsestat));
								// next
							}
						}
						else {
							_expression(&(*cont));
							// true
							_statBlock(&(*truestat));
							// next

						}
					}
				}
			}
		}
	}
}

void AS::_statReturn(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol dst;

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $RETURN) {
		advance(parent, node_it);

		if (node_it->nt_symbol == $Expression) {
			_expression(&(*node_it));
			advance(parent, node_it);

			if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $SEMICOLON) {

			}
		}
	}
}

void AS::_expression(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol dst, src1, src2;

	if (node_it->nt_symbol == $ExprArith) {
		_exprArith(&(*node_it));
		advance(parent, node_it);

		while (node_it != parent->children.end()) {
			advance(parent, node_it);

			if (node_it->nt_symbol == $ExprArith) {
				_exprArith(&(*node_it));
				advance(parent, node_it);

			}
		}
	}

}

void AS::_exprArith(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $Item) {
		_item(&(*node_it));
		advance(parent, node_it);
	}

	while (node_it != parent->children.end()) {
		advance(parent, node_it);

		if (node_it->nt_symbol == $Item) {
			_item(&(*node_it));
			advance(parent, node_it);
		}
	}
}

void AS::_item(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it->nt_symbol == $Factor) {
		_factor(&(*node_it));
		node_it++;
	}

	while (node_it != parent->children.end()) {

		advance(parent, node_it);
		if (node_it->nt_symbol == $Factor) {
			_factor(&(*node_it));
			advance(parent, node_it);
		}
	}

}

void AS::_factor(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol sym;

	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $NUM) {
		string token = node_it->token;
		sym = IntConstant(atoi(token.c_str()));

		// �����Z����
		node_it->val = sym;

	}
	else if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $ID) {
		string token = node_it->token;
		int line = node_it->line;

		sym = LookupID(token);
		if (sym == NULL) {
			Error_UnDef(token.c_str(), line);

			//�]�Єt�����̖��
			sym = AddVariable(token, line);
		}

		// �����Z����
		node_it->val = sym;
		
		advance(parent, node_it);

		if (node_it != parent->children.end() && node_it->nt_symbol == $Ftype) {

			if (sym != NULL && sym->kind == SK_Function)
			{
				_ftype(&(*node_it));

			}
		}
	}
	else if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $LPAREN) {
		advance(parent, node_it);
		if (node_it->nt_symbol == $Expression) {
			_expression(&(*node_it));
		}
	}
}

void AS::_ftype(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it != parent->children.end() && node_it->nt_symbol == $Call) {
		_call(&(*node_it));
	}
}

void AS::_call(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	
	if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $LPAREN) {
		advance(parent, node_it);

		if (node_it->nt_symbol == $Aparameter) {
			_aparameter(&(*node_it));

			if (node_it->nt_symbol == $TerminalSymbol && node_it->t_symbol == $RPAREN) {
				advance(parent, node_it);
			}
		}
	}
}

void AS::_aparameter(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();

	if (node_it != parent->children.end() && node_it->nt_symbol == $AparaList) {
		// ���Ѕ�����
		_aparaList(&(*node_it));
	}

}

void AS::_aparaList(TreeNode *parent)
{
	TreeNode_it node_it = parent->children.begin();
	Symbol sym;

	while (node_it != parent->children.end()) {
		if (node_it->nt_symbol == $Expression) {
			_expression(&(*node_it));
		}
		advance(parent, node_it);
	}

}

void AS::analyze(string input)
{
	Parser::analyze(input);
	if (synTree.nt_symbol == $Program)
	{
		// Initial the symbol table
		InitSymbolTable();

		// start 
		_program(&synTree);
	}

	if (Error::ErrorCount > 0)
		exit(-1);
}
