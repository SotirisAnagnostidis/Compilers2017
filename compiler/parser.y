%{ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"
#include "symbol.h"
#include "general.h"
#include "error.h"

void yyerror (const char *msg);
int yylex();
extern int countlines;
extern int kena;
extern const char* filename;
extern char* yytext;
void initialize();
void finalize();
int temp_lines;
int lines;

ast t, t1;
int ch = 0;

void check() {
	if (ch==0) {
		lines = id_line1; 
    }
	else{
		ch=0;
		lines = id_line2; 
    }
}

void add_to_mystack(char *name){
	List *ne = (List*)malloc(sizeof(ne)); 	
	ne->next = S->definitions; 			
	ne->name = strdup(name); 				
	S->definitions = ne;
}

typedef struct lines_str {
   
	int l;
	int times;
        struct lines_str* next;

} *lines_ptr;

lines_ptr start= NULL;
void lines_push(int a) {
	lines_ptr nt;
	nt = (lines_ptr)malloc(sizeof(struct lines_str));
	nt-> l = a;
        nt->times =0;
        nt-> next = start;
	start = nt;
	
}

int lines_peek(){
    if (start == NULL)
        return -1;
    else
        return start->l;
}

int lines_pop() {

	if  (start == NULL) 
		return -1;
	else {
		int a= start->l;
        lines_ptr temp = start;
		start = start->next;
        free(temp);
		return a;
	}
	      
}
%}

%union{
	ast a;
	char *s;
	int n;
	my_Type t;
	Dimensions d;
}

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

%token<s> T_id
%token<n> T_const 
%token<n> T_characters
%token<s> T_string

%token T_not_equal "<>"
%token T_less_equal "<="
%token T_greater_equal ">="
%token  T_set_value ":="


%left "or"
%left "and"
%left "not"
%left '=' "<>" '>' '<' "<=" ">="
%left '+' '-' '|'
%left '*' '/' '%' '&'
%left UPLUS UMINUS UNEG

%type<a> program
%type<a> localdef_list
%type<a> funcdef
%type<a> fpardef_list
%type<a> fpardef_list1
%type<a> header
%type<a> id_list
%type<a> id_list0
%type<a> fpardef
%type<s> datatype
%type<d> type_list
%type<a> type
%type<a> type_id
%type<a> fpartype
%type<a> localdef
%type<a> funcdecl
%type<a> vardef
%type<a> st_list
%type<a> stmt
%type<a> stmt_list
%type<a> block
%type<a> exprlist
%type<a> proccall
%type<a> funccall
%type<a> lvalue
%type<a> expr
%type<a> cond
%type<a> expr_list
%type<a> add_program
%type<a> libraries
%type<a> add_funcdecl
%type<a> add_header

%%

add_program:
   libraries { start = NULL;} program			{ t = $$ = ast_make_op(SEQ,$1,$3,countlines);}
;

libraries:
   /*nothing*/				                    { $$ = NULL; }
|  add_funcdecl "end" libraries			        { $$ = ast_make_op(SEQ,$1,$3,countlines); }
;

add_funcdecl:
  "decl" add_header                             { $$ = $2; }
;

add_header: 
  T_id				                        	{ add_to_mystack($1); $$ = ast_make_header(HEADER, $1 , NULL, 1, NULL,countlines); } 
| T_id "is" datatype 		 	                { add_to_mystack($1); $$ = ast_make_header(HEADER, $1, $3,1, NULL,countlines); }
| T_id ':' fpardef_list1 	 	                { add_to_mystack($1); $$ = ast_make_header(HEADER, $1, NULL, 1,$3,countlines ); }
| T_id "is" datatype ':' fpardef_list1 	        { add_to_mystack($1); $$ = ast_make_header(HEADER, $1, $3,1,$5,countlines); }
;

program: 
   funcdef  				                    { t1 = $$ = $1;}
;

localdef_list:
  /*nothing*/   			                    { $$ = NULL; }
| localdef  localdef_list  		                { $$ = ast_make_op(SEQ,$1,$2,countlines); } 
;

funcdef: 
  "def" { temp_lines =countlines; } header localdef_list block 	{ $$ = ast_make_func(FUNCDEF,$3,$4,$5,temp_lines); }  /* block? */
;

fpardef_list1:
  fpardef fpardef_list  		                { $$ = ast_make_op(SEQ,$1,$2,countlines); } 
;

fpardef_list:
  /*nothing*/			                    	{ $$ = NULL; } 
| ',' fpardef fpardef_list 		                { $$ = ast_make_op(SEQ,$2,$3,countlines); }
;
  
header: 
  T_id		                                    { check();  lines_push(lines); $$ = ast_make_header(HEADER, $1 , NULL,0,NULL,lines_pop()); } 
| T_id "is" datatype                            {  check();  lines_push(lines); $$ = ast_make_header(HEADER, $1, $3, 0, NULL,lines_pop()); } /*check this*/
| T_id ':'  { check();  lines_push(lines);}  fpardef_list1 { $$ = ast_make_header(HEADER, $1, NULL, 0, $4 ,lines_pop() ); }
| T_id  "is"  datatype   { check();  lines_push(lines);}  ':' fpardef_list1  { $$ = ast_make_header(HEADER, $1, $3, 0,$6,lines_pop()); }
;

id_list:
  type_id id_list0 			                    { $$ = ast_make_op(SEQ,$1,$2,countlines); }
;

type_id:
  T_id  				                        { check();  $$ = ast_make_lvalue(VAR,$1, NULL,lines); }
;

id_list0:
  /*nothing*/   			{ $$ = NULL; } 
| type_id id_list0 			{ $$ = ast_make_op(SEQ,$1,$2,countlines); }
;

stmt_list:
  /*nothing*/ 				{ $$ = NULL; }
| stmt stmt_list  			{ $$ = ast_make_op(SEQ,$1, $2,countlines); }
;

fpardef: 
  id_list { temp_lines = countlines; } "as" fpartype { $$ = ast_make_op(FPARDEF,$1,$4,temp_lines); }
;

datatype: 
  "int"  				{ $$ = "int"; }
| "byte" 				{ $$ = "byte"; }
;

type_list:
  /*nothing*/ 				    { $$ = NULL; }
|'[' T_const ']' type_list 		{ $$ = dim_add($2,$4,0); }		/*check not negative*/ 
;

type:
  datatype { temp_lines = countlines; } type_list { $$ = ast_make_type(OTHER,$1,$3,temp_lines); } 
;

fpartype:
  type 				            	{ $$ = $1; }
| "ref" datatype 			        { $$ = ast_make_type(REF,$2,NULL,countlines); } 
| datatype '[' ']' type_list		{ $$ = ast_make_type(OTHER,$1,dim_add(-1,$4,1),countlines); }
;

localdef: 
  funcdef   { $$ = $1; }
| funcdecl  { $$ = $1; }
| vardef    { $$ = $1; }
;

funcdecl:
  "decl" add_header "end" { $$ = $2; }
;


vardef:
  "var" id_list "is" type { $$ = ast_make_op(VARDEF,$2,$4,countlines); }
;

st_list:
  /*nothing*/ { $$ = NULL; }

| "elif" cond { temp_lines = countlines; }':' block st_list { $$ = ast_make_op(SEQ,ast_make_if(ELIF,$2, $5, $6,NULL,temp_lines),NULL,countlines); }
;

stmt:
  "skip"                              			        { $$ = ast_make_op(SKIP,NULL,NULL,countlines); }
| lvalue  {ch= 1;} ":="  expr                           { ch = 0;  $$ = ast_make_op(LET,$1, $4, lines_pop());  }
| proccall  			                                { $$ = $1; }
| "exit"  			                                    { $$ = ast_make_op(EXIT,NULL,NULL,countlines); }
| "return" ':' {temp_lines = countlines; ch= 1;} expr   { ch = 0;} { $$ = ast_make_op(RETURN,NULL, $4,temp_lines); }
|  "if"  cond ':' block st_list 	                    {  $$ = ast_make_if(IF,$2, $4, $5,NULL,countlines); }
| "if" cond ':' block st_list  "else" ':' block         { $$ = ast_make_if(IF,$2, $4, $5, $8,countlines); }
| "loop" T_id {temp_lines = countlines;} ':' block      { $$ = ast_make_loop(LOOP, $2, $5,temp_lines); }
| "loop" ':' block 		                                { $$ = ast_make_loop(LOOP, NULL, $3,countlines); }
| "break" ':' T_id  		                            { $$ = ast_make_break(BREAK, $3,countlines); }
| "break" 			                                    { $$ = ast_make_break(BREAK, NULL,countlines); }
| "continue" ':' T_id  		                            { $$ = ast_make_break(CONTINUE, $3,countlines); }
| "continue" 			                                { $$ = ast_make_break(CONTINUE, NULL,countlines); }
;

block:
   stmt_list "end"  { $$ = $1; }
;

exprlist:
  /*nothing*/  		    { $$ = NULL; }
| ',' expr exprlist   	{ $$ = ast_make_op(EXPRLIST,$2, $3,countlines); }
;

proccall:
  T_id  		{check();  $$ = ast_make_lvalue(PROCCALL,$1,NULL,lines); }
| T_id ':'      { check();  lines_push(lines);  ch=0; } expr { ch=1; } exprlist  { ch =0; $$ = ast_make_lvalue(PROCCALL,$1,ast_make_op(EXPRLIST,$4, $6,lines_peek()), lines_pop()); }
;

funccall: 
  T_id '('')'   {check(); $$ = ast_make_lvalue(FUNCCALL,$1,NULL,lines); }
| T_id '('      { check();  lines_push(lines); ch=0;}  expr { ch=1; } exprlist {ch=0;} ')' { $$ = ast_make_lvalue(FUNCCALL,$1,ast_make_op(EXPRLIST,$4,$6,lines_peek()),lines_pop()); }  /* to vala meta thn parenthesi gt evgaze warning kai de douleve den katalava giati */
;

lvalue: 
  T_id          { check();  lines_push(lines); $$ = ast_make_lvalue(VAR,$1, NULL, lines_peek()); }
| T_string      { check();  lines_push(lines); $$ = ast_make_lvalue(STR_VAR,$1, NULL,lines_peek());} 
| T_id          { check();  lines_push(lines); } expr_list	{ if (ch==1) ch=0; $$ = ast_make_lvalue(VAR,$1,$3,lines_peek()); }
| T_string      { check();  lines_push(lines); } expr_list  {  if (ch==1) ch=0; $$ = ast_make_lvalue(STR_VAR,$1,$3,lines_peek()); }
;




expr_list:
  /*nothing*/			{$$ = NULL; }
| '[' expr ']' expr_list	{$$ = ast_make_op(SEQ,$2,$4,countlines); }
;

expr: 
  T_const  		    { $$ = ast_make_const(INT,"int",$1,countlines);  }  
| T_characters 		{ $$ = ast_make_const(CHAR,"char",$1,countlines);}  
| lvalue 		    { $$ = $1 ; lines_pop();}
| '(' expr ')' 		{ $$ = $2 ; }
| funccall 	    	{ $$ = $1 ; }
| '+' expr 	    	{ $$ = ast_make_op(PLUS,ast_make_const(INT,"int",0,countlines), $2,countlines); }
| '-' expr 	    	{ $$ = ast_make_op(MINUS,ast_make_const(INT,"int",0,countlines), $2,countlines); }
| expr '+' expr 	{ $$ = ast_make_op(PLUS,$1, $3,countlines); }
| expr '-' expr 	{ $$ = ast_make_op(MINUS,$1, $3,countlines); }
| expr '*' expr 	{ $$ = ast_make_op(TIMES,$1, $3,countlines); }
| expr '/' expr 	{ $$ = ast_make_op(DIV,$1, $3,countlines); }
| expr '%' expr 	{ $$ = ast_make_op(MOD,$1, $3,countlines); }
| "true" 	    	{ $$ = ast_make_const(CHAR,"bool",1,countlines);  }   /* ??????? */
| "false" 	    	{ $$ = ast_make_const(CHAR,"bool",0,countlines);  }  /* ??????? */
| '!' expr  		{ $$ = ast_make_op(NOT_E,$2, NULL,countlines); }
| expr '&' expr 	{ $$ = ast_make_op(AND_E,$1, $3,countlines); }
| expr '|' expr 	{ $$ = ast_make_op(OR_E,$1, $3,countlines); }
;

cond:
  expr       		{ $$ = $1 ; }
| '(' cond ')'	 	{ $$ = $2 ; }
| "not" cond  		{ $$ = ast_make_op(NOT_C,$2, NULL,countlines); }
| cond "and" cond 	{ $$ = ast_make_op(AND_C,$1, $3,countlines); }
| cond  "or" cond 	{ $$ = ast_make_op(OR_C,$1, $3,countlines); }
| expr '=' expr 	{ $$ = ast_make_op(EQUAL,$1, $3,countlines); }
| expr "<>" expr 	{ $$ = ast_make_op(DIFFERENT,$1, $3,countlines); }
| expr '<'  expr 	{ $$ = ast_make_op(LOWER,$1, $3,countlines); }
| expr '>' expr 	{ $$ = ast_make_op(GREATER,$1, $3,countlines); }
| expr "<=" expr 	{ $$ = ast_make_op(LOWEREQ,$1, $3,countlines); }
| expr ">=" expr 	{ $$ = ast_make_op(GREATEREQ,$1, $3,countlines); }
;

%%

void yyerror (const char *msg) {
  my_countlines = countlines;
  fatal("Could not find appropriate syntax, last read token %s",yytext);
}

void insert_libraries(char *name,char **libraries,int pos){
	
	libraries[pos] = name;
}



char x[1024];

char* read_words (FILE *f) {
    while (fscanf(f, " %1023s", x) != EOF) {
        return x;
    }
    return NULL;
}


int main(int argc, char** argv) {
	S = NULL;
	if (argc < 4){
		fprintf(stderr,"Called with argc: %d\n" , argc);
		fprintf(stderr,"Usage: %s <number of lines in library file> <input_file> < <optimize>\n" , argv[0]);
		return 1;
	}
	printf("arg ok\n");
	countlines = - atoi(argv[1]) + 1;
	filename = strdup(argv[2]);	
	push(0);

    S->statements = 5;		//random number > 1
    S->has_begin = 1;

	/*
    // USE THIS FOR LEXICAL ANALYSIS 
	int token;	
	do{
		token = yylex();
		printf("new token %s\n" , yytext);
	}while(token!=0);
	return 0;
	*/

	if (yyparse()) 
        return 1;

	initSymbolTable(9997);
	initialize();				

	llvm_compile_and_dump(t, atoi(argv[3]));
	finalize();
  	destroySymbolTable();
	return 0;
}
   
