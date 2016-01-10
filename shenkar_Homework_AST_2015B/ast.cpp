#include "symtab.h"
#include "ast.h"

IdNode::IdNode (char *name, int line) 
{
	strcpy (this->_name, name); 
	_line = line;
}

BinaryOp::BinaryOp (enum op op, Exp *left, Exp *right, int line) 
{
    this->_op = op; 
	this->_left = left; 
	this->_right = right;
	_line = line;
}

AssignStmt::AssignStmt (IdNode *lhs, Exp *rhs, int line) 
   : Stmt ()  // call base class (super class) constructor
{
   _lhs = lhs; 
   _rhs = rhs; 
   _line = line; 
}

