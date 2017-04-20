%{ 

#include <stdio.h>
#include <stdlib.h>

void yyerror (const char *msg);

extern int countlines;
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

funcdef: 
  "def" header (local-def)* block
;

header: 
  T_id [ "is" datatype ] [ ":" fpardef ( "," fpardef)* ]
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

type:
  datatype ("["T_const"]")*
;

fpartype:
  type 
| "ref" datatype 
| datatype "[" "]" ("["T_const"]")*
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

stmt:
  "skip" 
| lvalue ":=" expr 
| proccall 
| "exit" 
| "return" ":" expr
| "if" cond ":" block ("elif" cond ":" block)*  ["else" ":" block]
| "loop" [T_id] ":" block 
| "break" [ ":" T_id ] 
| "continue" [ ":" T_id ]
;

block:
  "begin" stmt_list "end" 
;

proccall:
  T_id [":" expr ( "," expr )* ]
;

funccall: 
  T_id "(" [expr ( "," expr )* ] ")"
;

lvalue: 
  T_id 
| T_string 
| lvalue "[" expr "]"
;

expr: 
  T_const 
| lvalue
| "(" expr ")" 
| funccall
| ( "+" | "-" ) expr 
| expr ( "+" | "-" |  " * " | "/" | "%" ) expr
| "true" | "false" | "!" expr | expr ( "&" | "|" ) expr
;

cond:
  expr 
| "(" cond ")"
| "not" cond 
| cond ( "and" | "or" ) cond
| expr ( "=" | "<>" | "<" | ">" | "<=" | ">=" ) ⟨expr⟩
;

%%
