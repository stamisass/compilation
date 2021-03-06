#ifndef __AST_H
#define __AST_H 1

#include <string.h>

#include "gen.h"

/*  FALL_THROUGH may be used as an argument to BoolExp::genBoolExp().
    Labels  are represented as positive integers so FALL_THROUGH has negative value */
const int FALL_THROUGH = -1;

// all nodes  in the AST (Abstract Syntax Tree) are of types derived from ASTnode 
class ASTnode {
   
};

// expressions 

// this is an abstract class
class Exp : public  ASTnode {
public:
    virtual void genExp () = 0; // every subclass should override this (or be abstract too)
    
    Exp () { _result = -1; _type = UNKNOWN; }	
    // Exp (int result, myType _type) : ASTnode () { this->_result = result; this->_type = _type; }

    int _result;/* example: _result is 17 means result of expression will be in  _t17.
                   _result is -1 means: not known yet (_result is set to a new temporary when
				    code for the expression is generated) */
	myType _type;
    
};

class BinaryOp : public Exp {
public:
    BinaryOp (enum op op, Exp *left, Exp *right, int line);

	void genExp (); // override

    enum op _op;
	Exp *_left; // left operand
	Exp *_right; // right operand
	int _line;  // source line number of operator
};

class NumNode : public Exp {
public:
	void genExp (); // override
	
    NumNode (int ival) { _u.ival = ival; _type = _INT;}
    NumNode (double fval) { _u.fval = fval; _type = _FLOAT;}
	
	union {
	    int ival;
		double fval;
	} _u;
};

class IdNode : public Exp {
public: 
     void genExp (); // override

     IdNode (char *name, int line);
			          
     char _name [100];
	 int _line;  // source line number
};

//  boolean expressions
// this is an abstract class

class BoolExp : public  ASTnode {
public:
    /*  generate code for boolean expression. The code  should jump to
	    truelabel or to falselabel depending on the value of the expression.
        truelabel may  be  FALL_THROUGH meaning  code should fall through to 
		next instruction (rather than jump to a label)  when the expression is 
		true. falselabel  may also be  FALL_TROUGH */		
    virtual void genBoolExp (int truelabel, int falselabel) = 0; // every subclass should
                                            // override this (or be abstract too)
};
 
// nodes for  simple boolean expressions having the form 
// expression RELOP expression  
// for example  a < b
// another example:   (a + 3) < (z * 5 + y).    
class RelationalExp : public BoolExp {
public:
    RelationalExp (enum op op, Exp *left, Exp *right)
  	     { this->_op = op; this->_left = left; this->_right = right;}

    void genBoolExp (int truelabel, int falselabel); // override
	
    enum op _op;
	Exp *_left; // left operand
	Exp *_right; // right operand
};

class Or : public BoolExp {
public:
     Or (BoolExp *left, BoolExp *right) { _left = left; _right = right; }
	 void genBoolExp (int truelabel, int falselabel); // override
	 
	 BoolExp *_left; // left operand
	 BoolExp *_right; // right operand
};

class And : public BoolExp {
public:
	 And (BoolExp *left, BoolExp *right) { _left = left; _right = right; }
	 void genBoolExp (int truelabel, int falselabel); // override
	
	 BoolExp *_left; // left operand
	 BoolExp *_right; // right operand
};

class Not : public BoolExp {
public:
	 Not (BoolExp *boolexp) { _boolexp = boolexp; }
	 void genBoolExp (int truelabel, int falselabel); // override
	
	BoolExp *_boolexp;
};

	 // statements

// this is an abstract class
class Stmt: public ASTnode {
public:
   Stmt () {_next = NULL; } 
   
   virtual void genStmt () = 0; // every subclass should override this (or be abstract too)

   Stmt *_next;  // used to link together statements in the same block
};

class AssignStmt : public Stmt {
public:
      AssignStmt (IdNode *lhs, Exp *rhs, int line);

      void genStmt (); // override	  

      IdNode *_lhs; // left hand side
	  Exp *_rhs; // right hand side
	  int _line; // line in source code 
};
   
class IfStmt : public Stmt {
public:
     IfStmt (BoolExp *condition, Stmt *thenStmt, Stmt *elseStmt)
	          : Stmt ()
	          { this->_condition = condition; this->_thenStmt = thenStmt; this->_elseStmt = elseStmt; } 
	 void genStmt (); // override
	 
     BoolExp *_condition;
	 Stmt *_thenStmt;
	 Stmt *_elseStmt;
};

class WhileStmt: public Stmt {
public: 
	 // while_stmt :  WHILE '(' boolexp ')' stmt { $$ = new WhileStmt ($3, $5); };
     WhileStmt (BoolExp *condition, Stmt *body)
	          : Stmt ()
	          { _condition = condition; _body = body; }

	 void genStmt (); // override			  

     BoolExp *_condition;
	 Stmt *_body;
};

class ForStmt: public Stmt {
public:
	 // for_stmt   :  FOR '(' assign_stmt ';' boolexp ';' assign_stmt ';' ')' stmt { $$ = new ForStmt ($3, $5, $7, $10); };
	 ForStmt (AssignStmt *initialization, BoolExp *condition, AssignStmt *incrementation, Stmt *body)
	 : Stmt ()
	 { _initialization = initialization; _condition = condition; _incrementation = incrementation; _body = body; }
	 
	 void genStmt (); // override
	 
	 AssignStmt *_initialization;
	 BoolExp *_condition;
	 AssignStmt *_incrementation;
	 Stmt *_body;
};

//  a block contains a list of statements. For now -- no declarations in a block
class Block: public Stmt {
public:
	 Block (Stmt *stmtlist) { _stmtlist = stmtlist; }
	 void genStmt (); // override
	 
     Stmt* _stmtlist; // pointer to the first statement. each statement points to the next stmt
};

#endif  // not defined __AST_H