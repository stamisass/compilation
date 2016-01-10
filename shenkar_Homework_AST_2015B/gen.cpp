#include <stdio.h>
#include <stdlib.h> // exit ()
#include <stdarg.h>
#include "ast.h"

static
int newTemp ()
{
   static int counter = 1;
   return counter++;
} 

static
int newlabel ()
{
   static int counter = 1;
   return counter++;
} 

// emit works just like  printf  --  we use emit 
// to generate code and print it to the standard output.
void emit (const char *format, ...)
{
    // if (errors > 0) return; // do not generate code if there are errors. 
	
    va_list argptr;
	va_start (argptr, format);
	// all the arguments following 'format' are passed on to vprintf
	vprintf (format, argptr); 
	va_end (argptr);
} 

/* there are two versions of each arithmetic operator in the
   generated code. One is  used for  operands having type int. 
   The other is used for operands having type float.
*/   
struct operator_names { 
    const char *int_name; 
	const char *float_name; 
};

static
struct operator_names 
opNames [] = { {"+", "plus"},
            { "-", "minus"},
			{ "*", "mul" },
			{ "/", "div" }};

/* convert operator  to  string  suitable for the given type
  e.g. opName (PLUS, _INT)  returns "+"
       opName (PLUS, _FLOAT) returns  "plus"
*/
const char *
opName (enum op op, myType t)
{
    if (op > DIV) { fprintf (stderr, "internal compiler error #1"); exit (1); }
    if (t == _INT)
	    return opNames [op].int_name;
	else
	    return opNames [op].float_name;
}

void BinaryOp::genExp ()
{
	_left->genExp ();
	_right->genExp ();
	
	_result = newTemp ();

    const char *the_op;
	
    switch (_op) {
	    case PLUS:
		    the_op = "+";
			break;
		case MINUS:
		    the_op = "-";
			break;
		case MUL:
		    the_op = "*";
			break;
		case DIV:
		    the_op = "/";
			break;
		default:
		    fprintf (stderr, "internal compiler error #2\n"); exit (1);
	}
	
  	emit ("_t%d = _t%d %s _t%d\n", _result, _left->_result, the_op, _right->_result);
}

void NumNode::genExp () 
{
    _result = newTemp ();
	if (_type == _INT)
  	    emit ("_t%d = %d\n", _result, _u.ival);
	else
	    emit ("_t%d = %.2f\n", _result, _u.fval);
}

void IdNode::genExp ()
{
    _result = newTemp ();
		
	emit ("_t%d = %s\n", _result, _name);
}

void RelationalExp::genBoolExp (int truelabel, int falselabel)
{
    if (truelabel == FALL_THROUGH && falselabel == FALL_THROUGH)
	    return; // no need for code 

    const char *the_op;
	
	_left->genExp ();
	_right->genExp ();
	
    switch (_op) {
	    case LT:
		    the_op = "<";
			break;
		case GT:
		    the_op = ">";
			break;
		case LE:
		    the_op = "<=";
			break;
		case GE:
		    the_op = ">=";
			break;
		case EQ:
		    the_op = "==";
			break;
		case NE:
		    the_op = "!=";
			break;
		default:
		    fprintf (stderr, "internal compiler error #3\n"); exit (1);
	}
	
	if  (truelabel == FALL_THROUGH)
   	    emit ("ifFalse _t%d %s _t%d goto label%d\n", _left->_result, the_op, _right->_result, falselabel);
    else if (falselabel == FALL_THROUGH)
   	    emit ("if _t%d %s _t%d goto label%d\n", _left->_result, the_op, _right->_result, truelabel);
	else { // no fall through
  	    emit ("if _t%d %s _t%d goto label%d\n", _left->_result, the_op, _right->_result, truelabel);
	    emit ("goto label%d\n", falselabel);
	}
}

void Or::genBoolExp (int truelabel, int falselabel)
{
    if (truelabel == FALL_THROUGH && falselabel == FALL_THROUGH)
	    return; // no need for code 

    if  (truelabel == FALL_THROUGH) {
	    int next_label = newlabel(); // FALL_THROUGH implemented by jumping to next_label
	    _left->genBoolExp (next_label, // if left operand is true then the OR expression
		                               //is true so jump to next_label (thus falling through
									   // to the code following the code for the OR expression)
		                   FALL_THROUGH); // if left operand is false then 
						                  // fall through and evaluate right operand   
		_right->genBoolExp (FALL_THROUGH, falselabel);
        emit ("label%d :\n", next_label);
    }  else if (falselabel == FALL_THROUGH) {
       _left->genBoolExp (truelabel, // if left operand is true then the OR expresson is true 
	                                 // so jump to  truelabel (without evaluating right operand)
                          FALL_THROUGH); // if left operand is false then 
						                  // fall through and evaluate right operand
	   _right->genBoolExp (truelabel, FALL_THROUGH);
	} else { // no fall through
	   _left->genBoolExp (truelabel, // if left operand is true then the or expresson is true 
	                                 // so jump to  truelabel (without evaluating right operand)
						  FALL_THROUGH); // if left operand is false then 
						                  // fall through and evaluate right operand
	   _right->genBoolExp (truelabel, falselabel);
	}
}

void And::genBoolExp (int truelabel, int falselabel)
{
	if (truelabel == FALL_THROUGH && falselabel == FALL_THROUGH)
	    return; // no need for code
	
	if (truelabel == FALL_THROUGH)
	{
		_left->genBoolExp (truelabel, falselabel);
		_right->genBoolExp(FALL_THROUGH, falselabel);
	} else if (falselabel == FALL_THROUGH)
	{
		_left->genBoolExp (truelabel, falselabel);
		_right->genBoolExp(truelabel, FALL_THROUGH);
	} else 
	{
		_left->genBoolExp (truelabel, falselabel);
		_right->genBoolExp(truelabel, falselabel);
	}
}

void Not::genBoolExp (int truelabel, int falselabel) 
{
	if (truelabel == FALL_THROUGH && falselabel == FALL_THROUGH)
	    return; // no need for code
	
	_boolexp->genBoolExp (falselabel, truelabel);
	
}

void AssignStmt::genStmt()
{
    _rhs->genExp();
	
	emit ("%s = _t%d\n", _lhs->_name, _rhs->_result);
}


void IfStmt::genStmt()
{
    // int thenStmtlabel = newlabel ();
    int elseStmtlabel = newlabel ();
	int exitlabel = newlabel ();
	
	_condition->genBoolExp (FALL_THROUGH, elseStmtlabel);
	
    _thenStmt->genStmt ();
	emit ("goto label%d\n", exitlabel);
	emit ("label%d:\n", elseStmtlabel);
    _elseStmt->genStmt();
	emit ("label%d:\n", exitlabel);
}

void WhileStmt::genStmt()
{
    int condlabel = newlabel ();
	int exitlabel = newlabel ();
	
	emit ("label%d:\n", condlabel);
	_condition->genBoolExp (FALL_THROUGH, exitlabel);
	
	_body->genStmt ();
	
	emit ("goto label%d\n", condlabel);
	emit ("label%d:\n", exitlabel);
}

void ForStmt::genStmt()
{
	int condlabel = newlabel ();
	int exitlabel = newlabel ();
	
	_initialization->genStmt ();
	
	emit ("label%d\n", condlabel);
	_condition->genBoolExp (FALL_THROUGH, exitlabel);
	
	_body->genStmt ();
	
	_incrementation->genStmt ();
	
	emit ("goto label%d\n", condlabel);
	emit ("label%d\n", exitlabel);	
}

void Block::genStmt()
{
    for (Stmt *stmt = _stmtlist; stmt != NULL; stmt = stmt->_next)
	    stmt->genStmt();
}
