%{ 

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"


void yyerror (const char *msg);
extern int countlines;
extern stack S;
extern char* yytext;
%}

%token T_and "and"
%token T_def "def"
%token T_is "is"
%token T_skip "skip"
%token T_as "as"
%token T_elif "elif"
%token T_int "int"
%token T_true "true"
%token T_else "else"
%token T_var "var"
%token T_loop "loop"
%token T_break "break"
%token T_not "not"
%token T_byte "byte"
%token T_exit "exit"
%token T_or "or"
%token T_continue "continue"
%token T_false "false"
%token T_ref "ref"
%token T_decl "decl"
%token T_return "return"
%token T_if "if"
%token T_then "then"
%token T_begin "begin"
%token T_end "end"

%token T_id
%token T_const 
%token T_characters
%token T_string

%token T_not_equal 
%token T_less_equal
%token T_greater_equal
%token T_set_value


%left "or"
%left "and"
%left "not"
%left '=' "<>" '>' '<' "<=" ">="
%left '+' '-' '|'
%left '*' '/' '%' '&'
%left UPLUS UMINUS UNEG

%%

program: 
   funcdef
;

localdef_list:
  /*nothing*/
| localdef localdef_list
;

funcdef: 
  "def" {printf("found a def\n");} header localdef_list {printf("found a def3\n");}  block  {printf("found a def4\n");}
;

fpardef_list:
  /*nothing*/
| ',' fpardef fpardef_list
;
  
header: 
  T_id 
| T_id "is" datatype 
| T_id ':' fpardef fpardef_list
| T_id "is" datatype ':' fpardef fpardef_list 
;

id_list:
  T_id
| T_id id_list
;

stmt_list:
  stmt
| stmt stmt_list
;

fpardef: 
  id_list "as" fpartype
;

datatype: 
  "int" 
| "byte"
;

type_list:
  /*nothing*/
|'[' T_const ']' type_list
;

type:
  datatype type_list
;

fpartype:
  type 
| "ref" datatype 
| datatype '[' ']' type_list
;


localdef: 
  funcdef 
| funcdecl 
| vardef
;

funcdecl:
  "decl" header
;


vardef:
  "var" id_list "is" type
;


st_list:
  /*nothing*/
| "elif" cond ':' block st_list
;

stmt:
  "skip" 
| lvalue ":=" expr 
| proccall 
| "exit" 
| "return" ':' expr
| "if" cond ':' block st_list  "else" ':' block
| "loop" T_id ':' block 
| "break" ':' T_id  
| "continue" ':' T_id 
;

block:
  {printf("%s\n",yytext); } stmt_list "end" 
;

exprlist:
  /*nothing*/
| ',' expr exprlist
;

proccall:
  T_id ':' expr exprlist
;

funccall: 
  T_id '(' expr exprlist  ')'
;

lvalue: 
  T_id 
| T_string 
| lvalue '[' expr ']'
;

expr: 
  T_const 
| lvalue
| '(' expr ')' 
| funccall
| '+' expr 
| '-' expr 
| expr '+' expr
| expr '-' expr
| expr '*' expr
| expr '/' expr
| expr '%' expr
| "true" | "false" | '!' expr | expr '&' expr | expr '|' expr
;

cond:
  expr 
| '(' cond ')'
| "not" cond 
| cond "and" cond | cond  "or" cond
| expr '=' expr | expr "<>" expr  | expr '<'  expr | expr '>' expr | expr "<=" expr | expr ">=" expr
;

%%

void yyerror (const char *msg) {
  fprintf(stderr, "Dana error: %s\n", msg);
  fprintf(stderr, "Aborting, I've had enough with line %d...\n",
          countlines);
  exit(1);
}

int main() {
        S = NULL;
	printf("parsing...\n");
        //Make a node in stack corresponding to global namespace
        push(0);
        S->statements = 5;		//random nnumer
        S->has_begin = 1;
	add_libraries();
	if (yyparse()) return 1;
	printf("Compilation was successful.\n");
	return 0;
}
   
