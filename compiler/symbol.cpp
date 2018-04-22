#include "symbol.h"

Symbol SymTable::DoLookupSymbol(Table tbl, string name, int searchOuter) {
	vector<Symbol>::iterator it;

	do {
		for (it = tbl->buckets.begin(); it != tbl->buckets.end(); it++)
		{
			if ((*it)->name == name) {
				return  *it;
			}
		}
	} while ((tbl = tbl->outer) != NULL && searchOuter);

	return NULL;
}

Symbol SymTable::LookupID(string name, int searchOuter)
{
	return DoLookupSymbol(Identifiers, name, searchOuter);
}

void SymTable::EnterScope(void)
{
	Table t = new table;

	Level++;

	t->level = Level;
	t->outer = Identifiers;
	Identifiers = t;
}

void SymTable::ExitScope(void)
{
	Level--;
	Identifiers = Identifiers->outer;
}

Symbol SymTable::AddSymbol(Table tbl, Symbol sym)
{
	sym->level = tbl->level;
	tbl->buckets.push_back((Symbol)sym);

	return sym;
}

Symbol SymTable::AddVariable(string name, int line)
{
	VariableSymbol p = new variableSymbol;

	p->kind = SK_Variable;
	p->name = name;
	p->ref = 0;
	p->reg = NULL;
	p->link = NULL;
	p->line = line;

	return AddSymbol(Identifiers, (Symbol)p);
}

Symbol SymTable::AddFunction(string name, int line)
{
	FunctionSymbol p = new functionSymbol;

	p->kind = SK_Function;
	p->name = name;
	p->ref = 0;
	p->reg = NULL;
	p->link = NULL;
	p->line = line;

	if (Identifiers != &GlobalIDs) {
		AddSymbol(Identifiers, (Symbol)p);
	}

	return AddSymbol(&GlobalIDs, (Symbol)p);
}

Symbol SymTable::AddConstant(int i)
{
	Symbol p = new symbol;

	p->kind = SK_Constant;

	char buf[256];
	sprintf(buf, "%d", i);
	p->name = buf;
	p->ref = 0;
	p->val = i;
	p->reg = NULL;
	p->link = NULL;

	Constants.buckets.push_back((Symbol)p);

	return p;
}
Symbol SymTable::CreateTemp()
{
	VariableSymbol p = new variableSymbol;

	p->kind = SK_Temp;

	char buf[256];
	sprintf(buf, "t%d", TempNum++);
	p->name = buf;
	p->reg = NULL;
	p->link = NULL;
	p->level = 1;
	p->ref = 0;

	return (Symbol)p;
}

Symbol SymTable::CreateLabel(void)
{
	Symbol p = new symbol;

	p->kind = SK_Label;

	char buf[256];
	sprintf(buf, "BB%d", LabelNum++);
	p->name = buf;
	p->reg = NULL;
	p->link = NULL;
	
	return p;
}

Symbol SymTable::IntConstant(int i)
{
	return AddConstant(i);
}

void SymTable::InitSymbolTable(void)
{
	Level = 0;

	Identifiers = &GlobalIDs;
	Identifiers->outer = NULL;

	TempNum = LabelNum = 0;

}
