#ifndef __OPCODE_H_
#define __OPCODE_H_

using namespace std;

enum
{
	$NOP,
	$ADD, $SUB, $MUL, $DIV,
	$JE, $JNE, $JG, $JL, $JGE, $JLE,
	$JMP, $IJMP,
	$MOV,
	$CALL, $RETU,
	$RET
};

/*
 * ADD,     "+",
 * SUB,     "-",
 * MUL,     "*",
 * DIV,     "/",
 * JE,      "==",
 * JNE,     "!=",
 * JG,      ">",
 * JL,      "<",
 * JGE,     ">=",
 * JLE,     "<=",
 * JMP,		"jmp",
 * IJMP,	"ijmp",
 * MOV,		"=",
 * CALL,    "call",
 * RETU,    "return", 
 * RET,		"ret";
 */

#endif
