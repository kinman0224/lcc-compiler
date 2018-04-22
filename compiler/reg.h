#ifndef __REG_H_
#define __REG_H_

#include "symbol.h"

#define NO_REG	-1

// ¼Ä´æÆ÷·Nî
enum {EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI};

class Reg {
protected:
	Symbol Regs[EDI+1];
	int UsedRegs;

public:
	Symbol	CreateReg(string name, int no);

	void	ClearRegs();
	void	SpillReg(Symbol reg);
	int		SelectSpillReg();
	int		FindEmptyReg();

	Symbol	GetRegInternal();

};
#endif
