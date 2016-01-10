%code {
#include <stdio.h>
#include <string>
#include <stdarg.h>
#include "symtab.h"

  /* yylex () and yyerror() need to be declared here */
int yylex (void);
void yyerror (std::string s);

void emit (const char *format, ...);

// number of errors 
int errors;
}

%code requires {
#include "gen.h"
#include "ast.h"
};	

%union {
   int ival;	
   // float numbers in the source program are stored as double
   double fval;
   enum op op;
   char name[100];
   
   myType _type;
   
   //  pointers to AST nodes:
   Stmt *stmt;
   Block *block;
   Stmt *stmtlist; // points to first Stmt in the list (NULL if list is empty)
   AssignStmt *assign_stmt;
   IfStmt *if_stmt;
   WhileStmt *while_stmt;
   Exp  *exp;
   BoolExp *boolexp;
   ForStmt *for_stmt;
}

%token <ival> INT_NUM
%token <fval> FLOAT_NUM
%token <op> ADDOP MULOP RELOP
%token <name> ID

%token IF ELSE WHILE INT FLOAT OR AND FOR NOT

%type <_type> type

%type <exp> expression
%type <boolexp> boolexp 
%type <stmt> stmt
%type <block> block
%type <stmtlist> stmtlist 
%type <assign_stmt> assign_stmt 
%type <while_stmt> while_stmt
%type <if_stmt>  if_stmt 
%type <for_stmt> for_stmt

%left OR
%left AND
%left ADDOP
%left MULOP

%error-verbose

%%
program    : declarations stmt { 
                     /* if (errors == 0) { for debugging: generate code even if errors found */
					     $2->genStmt (); emit ("halt"); 
						 /*} */
				     }

declarations: declarations type ID ';'
             | /* empty */ ;

type: INT { $$ = _INT; } |
      FLOAT { $$ = _FLOAT; };			  

stmt       :  assign_stmt { $$ = $1; } |
              while_stmt  { $$ = $1; } |
	          if_stmt     { $$ = $1; } |
			  block       { $$ = $1; } |
			  for_stmt	  { $$ = $1; } ;

assign_stmt:  ID '='  expression ';' { $$ = new AssignStmt (new IdNode ($1, @1.first_line),
                                                            $3, @2.first_line); };

while_stmt :  WHILE '(' boolexp ')' stmt { $$ = new WhileStmt ($3, $5); };

if_stmt    :  IF '(' boolexp ')' stmt ELSE stmt { $$ = new IfStmt ($3, $5, $7); };

/* there is another ';' after the second assign_stmt because there is one also in your for.txt example */
for_stmt   :  FOR '(' assign_stmt ';' boolexp ';' assign_stmt ';' ')' stmt { $$ = new ForStmt ($3, $5, $7, $10); };

block: '{' stmtlist '}' { $$ = new Block ($2); };

/*  right recursion is used here because it simplifies the code  a bit.
    but left recursion  (i.e. stmtlist: stmtlist stmt)  would be better
    because it uses less stack space.
    Semantic value of stmtlist is a pointer to the first Stmt in a list 
	of Stmts. Each Stmt points to the next Stmt on the list
*/
stmtlist:  stmt stmtlist {  $1->_next = $2;  // also works when $2 is NULL
                            $$ = $1;  
						 };
stmtlist:  /* empty */ { $$ = NULL; };
	  
expression : expression ADDOP expression {
                  $$ = new BinaryOp ($2, $1, $3, @2.first_line); } |
		     expression MULOP expression {
                  $$ = new BinaryOp ($2, $1, $3, @2.first_line); };

expression: '(' expression ')' { $$ = $2; } |
            ID          { $$ = new IdNode ($1, @1.first_line);} |
            INT_NUM     { $$ = new NumNode ($1); } |
			FLOAT_NUM   { $$ = new NumNode ($1); };
			
boolexp: expression RELOP expression { $$ = new RelationalExp ($2, $1, $3); };

boolexp: boolexp OR boolexp { $$ = new Or ($1, $3); } |
		 boolexp AND boolexp { $$ = new And ($1, $3); } |
		 NOT '(' boolexp ')' { $$ = new Not ($3); }; 
		 

/* These  productions should be added:
         boolexp: boolexp AND boolexp |
		          NOT '(' boolexp ')' 
		        ;
         stmt:  for_stmt;
         for_stmt: FOR '(' assign_stmt boolexp ';' assign_stmt ')' stmt ;	 
*/
	  
	  
%%
main (int argc, char **argv)
{
  extern FILE *yyin;
  if (argc != 2) {
     fprintf (stderr, "Usage: %s <input-file-name>\n", argv[0]);
	 return 1;
  }
  yyin = fopen (argv [1], "r");
  if (yyin == NULL) {
       fprintf (stderr, "failed to open %s\n", argv[1]);
	   return 2;
  }
  
  errors = 0;
  yyparse ();
   
  if (errors > 0) {
      fprintf(stderr, "compilation failed\n");
	  return 3;
  }
  
  fclose (yyin);
  return 0;
}

void yyerror (std::string s)
{
  extern int yylineno;  // defined by flex
  errors++;
  fprintf (stderr, "line %d: %s\n", yylineno, s.c_str ());
}

void errorMsg (const char *format, ...)
{
    va_list argptr;
	va_start (argptr, format);
	// all the arguments following 'format' are passed on to vfprintf
	vfprintf (stderr, format, argptr); 
	va_end (argptr);
	
	errors++;
} 






