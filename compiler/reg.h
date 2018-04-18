#ifndef __REG_H_
#define __REG_H_

#include "symbol.h"

#define NO_REG	-1

// 寄存器種類
enum {EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI};

enum {$zero, 
	$at, 
	$v0, $v1,
	$a0, $a1, $a2, $a3,
	$t0, $t1, $t2, $t3, $t4, $t5, $t6, $t7,
	$s0, $s1, $s2, $s3, $s4, $s5, $s6, $s7,
	$t8, $t9,
	$k0, $k1,
	$gp,
	$fp,
	$ra
};

class Reg {
protected:
	Symbol Regs[$ra+1];
	int UsedRegs;	//標誌位，由低位開始代表EAX...00000001

public:
	Symbol	CreateReg(string name, int no);

	void	ClearRegs();
	void	SpillReg(Symbol reg);
	int		SelectSpillReg();
	int		FindEmptyReg();

	Symbol	GetRegInternal();

};
#endif
