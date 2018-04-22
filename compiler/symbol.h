#ifndef __SYMBOL_H_
#define __SYMBOL_H_

#include <string>
#include <map>
#include <vector>

using namespace std;

enum
{
	SK_Constant, SK_Variable, SK_Temp,
	SK_Label, SK_Function, SK_Register
};

#define SYMBOL_COMMON     \
    int		kind;         \
    string	name;         \
	string	aname;		  \
    int		level;        \
    int		ref;          \
	int		val;		  \
	int		line;		  \
    struct symbol *reg;   \
	struct symbol *link;  \
    struct symbol *next;

typedef struct bblock *BBlock;

typedef struct symbol
{
	SYMBOL_COMMON
} *Symbol;

typedef struct variableSymbol
{
	SYMBOL_COMMON
	int	idata;
	int offset;
} *VariableSymbol;

typedef struct functionSymbol
{
	SYMBOL_COMMON
	vector<Symbol> params;
	vector<Symbol> locals;
	int nbblock;
	BBlock entryBB;
	BBlock exitBB;
} *FunctionSymbol;

typedef struct table
{
	vector<Symbol> buckets;
	int level;
	struct table *outer;
} *Table;

class SymTable {
protected:
	// normal identifiers in current scope
	Table Identifiers;
	// normal identifiers in global scope
	struct table GlobalIDs;
	// all the constants
	struct table Constants;

	// Level increment; exiting each nesting level, Level decrement
	int Level;
	// number of temporaries
	int TempNum;
	// number of labels, see CreateLabel(void)
	int LabelNum;

private:
	Symbol AddSymbol(Table tbl, Symbol sym);
	Symbol DoLookupSymbol(Table tbl, string name, int searchOuter = 1);

public:
	void InitSymbolTable(void);
	void EnterScope(void);
	void ExitScope(void);

	Symbol AddVariable(string name, int line);
	Symbol AddFunction(string name, int line);
	Symbol AddConstant(int i);

	Symbol CreateTemp();
	Symbol CreateLabel();
	Symbol IntConstant(int i);

	Symbol LookupID(string name, int searchOuter = 1);
};

#endif