%{ 

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

void yyerror (const char *msg);

extern int countlines;
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
   funcdef {printf("ficdef yytext: %s\n" , yytext); }
;

localdef_list:
  /*nothing*/
| localdef localdef_list
;

funcdef: 
  "def" {printf("before header yytext: %s\n" , yytext); } header {printf("after header yytext: %s\n" , yytext); } localdef_list{printf("before block yytext: %s\n" , yytext); } block 
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
| "elif" cond ':' block st_list st_list
;

stmt:
  "skip" 
| lvalue ":=" expr 
| proccall 
| "exit" 
| "return" ':' expr
| "if" cond ':' block st_list
| "if" cond ':' block st_list  "else" ':' block
| "loop" T_id ':' block 
| "lopp" ':' block
| "break" ':' T_id  
| "continue" ':' T_id 
;

block:
  {printf("before block yytext: %s\n" , yytext); }"begin" stmt_list "end" 
;

exprlist:
  /*nothing*/
| ',' expr exprlist
;

proccall:
  T_id ':' expr exprlist
;

funccall: 
  T_id
| T_id '(' expr exprlist  ')'
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
| "true" 
| "false" 
| '!' expr 
| expr '&' expr 
| expr '|' expr
;

cond:
  expr 
| '(' cond ')'
| "not" cond 
| cond "and" cond 
| cond  "or" cond
| expr '=' expr 
| expr "<>" expr 
| expr '<'  expr 
| expr '>' expr 
| expr "<=" expr 
| expr ">=" expr
;

%%

void yyerror (const char *msg) {
  fprintf(stderr, "Dana error: %s\n", msg);
  fprintf(stderr, "Aborting, I've had enough with line %d...\n",
          countlines);
  exit(1);
}

void insert_libraries(char *name,char **libraries,int pos){
	
	libraries[pos] = name;
}

void add_libraries(){
        int size = 1;
	char **libraries;
	libraries = malloc(size*sizeof(char*));
        insert_libraries("writeString",libraries,0);
	int i = 0;
	while (i<size){
		List *new = malloc(sizeof(new));
		new->next = S->definitions;
		new->name = libraries[i];
		S->definitions = new;
		i++;
	}
}

int main() {
	printf("parsing...\n");
        //Make a node in stack corresponding to global namespace
        S = malloc(sizeof(stack));
        S->beggining = 0;
        S->statements = 5;		//random nnumer > 1
        S->has_begin = 1;
	add_libraries();
	if (yyparse()) return 1;
	printf("Compilation was successful.\n");
	return 0;
}
   
