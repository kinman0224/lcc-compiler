#include "reg.h"

Symbol Regs[EDI + 1];
int UsedRegs = 0;	//���Iλ���ɵ�λ�_ʼ����EAX...00000001

Symbol Reg::CreateReg(string name, int no)
{
	Symbol reg;

	reg = new symbol;
	reg->kind = SK_Register;
	reg->name = reg->aname = name;
	reg->val = no;
	reg->reg = reg;

	return reg;
}

void Reg::ClearRegs()
{
	int i;

	for (i = EAX; i <= EDI; i++)
	{
		if (Regs[i])
			SpillReg(Regs[i]);
	}
}

void Reg::SpillReg(Symbol reg)
{
	Symbol p;

	// �Ĵ���ָ��ķ�̖
	p = reg->link;

	if (p)
	{
		// ��ԓ�Ĵ������, ����ָ��NULL

		p->reg = NULL;
		// ����Ҫ����, �t��Ҫ���ص��ȴ�

	}
	reg->link = NULL;
}

int Reg::SelectSpillReg()
{
	Symbol p;
	int i;
	int reg = NO_REG;
	int mref = INT_MAX;

	// �ҳ��Ĵ��������Ô���С�ģ��@�e���ٵ�ͨ����0
	for (i = EAX; i <= EDI; i++)
	{
		if (Regs[i] == NULL || (1 << i & UsedRegs))
			continue;

		// �ҵ���ָ��ķ�̖
		p = Regs[i]->link;
		if (p->ref < mref)
		{
			mref = p->ref;
			reg = i;
		}

	}

	return reg;
}

int Reg::FindEmptyReg()
{
	int i;
	for (i = EAX; i <= EDI; i++)
	{
		if (Regs[i] != NULL && Regs[i]->link == NULL && !(1 << i & UsedRegs))
			return i;
	}

	return NO_REG;
}


/*
	@ �������Q	: GetRegInternal
	@ ����		: 
	@ ����		: ���ҿ��õļĴ���
	@ ����ֵ		: ���ؿ��õļĴ���
*/

Symbol Reg::GetRegInternal()
{
	int i;

	// �Lԇ�ҵ�һ��߀�]��ʹ�õļĴ����������ز����-1���������
	i = FindEmptyReg();
	// ���]�п���ļĴ���
	if (i == NO_REG)
	{
		i = SelectSpillReg();
		SpillReg(Regs[i]);
	}

	UsedRegs |= 1 << i;	//�O�Ø��Iλ

	return Regs[i];
}
