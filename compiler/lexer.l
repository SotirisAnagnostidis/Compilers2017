%{
	#include <stdlib.h>
    #include <stdio.h>
	#include "ast.h"
    #include "parser.hpp"
    #include "lexer.h"
    #include "error.h"
    #include "general.h"

	#define T_eof 0	
	#define MAX_STRING_SIZE 256	
	#define TAB_SIZE 8
	int countlines ;                   /* depends on the size of the libraries */
   	// char * filename;
	int num_comment;
	int num_eisagwgika;
	int kena;
        int id_line1=0;
        int id_line2=0;
	int var = 0;
        int prev_l =0;
	int statements = 0;
	char *temp;
	char is_def = 0;
	stack S;

	static void yyunput (int c, char * yy_bp );

	void my_free(List *temp){
		if (temp!=NULL){
			if (temp->next != NULL)
				my_free(temp->next);
		}
		free(temp);
	}


	int to_number(char c){
		if      (c >= 'a' && c <= 'f')  return c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')  return c - 'A' + 10;
		return c - '0';
	}

	void pop(){
		Node* temp = S;		
		S = S->next;
		my_free(temp->definitions);
		free(temp);
	}

	int isempty(){
		if (S==NULL)
			return 1;
		else
			return 0;
	}
	
	void push(int spaces){
		struct node* he = (struct node*)malloc(sizeof(struct node));
		he->next = S;
		he->beggining = spaces;
		he->statements = 0;
		he->has_begin = 0;
		he->pseudo_begin = 0;
		he->is_def = 0;
		he->definitions = NULL;
		S = he;
	}

	int checkit(int sp) {
		if (isempty()){
			fprintf(stderr,"Something went wrong with blocks at line %d\n",countlines);	
            exit(1);
        }
		if (S->beggining < sp) return 0;
		return 1;
	}

	int check(int sp){
		return checkit(sp);
	}
	
	void read_again(){
		int i;
		/* Copy yytext because unput() trashes yytext */
		char *yycopy = strdup( yytext );		
		for ( i = yyleng - 1; i >= 0; --i )
			unput(yycopy[i]);
		free(yycopy);
	}	

	void return_id(char *temp){
		int i=0;
		while (temp[i]!=' '&& temp[i]!='[' && temp[i]!='(' && temp[i]!=':' && temp[i]!='\t' && temp[i]!='\n'){
			i++;
		}
		i++;
		yytext = (char*)malloc(i*sizeof(char));
		i=0;
		while (temp[i]!=' '&& temp[i]!='['  && temp[i]!='(' && temp[i]!='\t' && temp[i]!='\n'){
			yytext[i] = temp[i];
			i++;
		}
		yytext[i] = '\0';
		int j = strlen(temp) - 1;
		while (j>=i){
			unput(temp[j]);
			j--;
		}
	}

	void return_string(char *temp){
		int i=1;
		while (temp[i]!='\"' || temp[i-1]=='\''){
			i++;
		}
		i+=2;
		yytext = (char*)malloc(i*sizeof(char));
		yytext[0] = '\"';
		i=1;
		while (temp[i]!='\"' || temp[i-1]=='\''){
			yytext[i] = temp[i];
			i++;
		}
		yytext[i] = '\"';
		yytext[++i] = '\0';
	}
	
	
	void rewrite_set_value(){
		unput('=');
		unput(':');
	}
	
	void add_to_list(char *name){
		List *ne = (List*)malloc(sizeof(List));
		ne->next = S->next->definitions;
		ne->name = strdup(name);
		S->next->definitions = ne;
	}
	
	int in_list(){
		Node* temp = S;
		while (temp!=NULL){
			List* iter = temp->definitions;
			while (iter!=NULL){
				if (strcmp(iter->name,yytext)==0){
					return 1;
				}
				iter = iter->next;
			}
			temp = temp->next;
		}
		return 0;
	}
	
	#define CHECK_LAST if(S->has_begin == 0 && check(kena)){if(S->pseudo_begin == 0){S->pseudo_begin = 1;} read_again(); yytext="end"; pop(); return T_end; }
	#define CHECK_FIRST if(S->statements <= 1 && S->has_begin == 0 && S->pseudo_begin == 0){ S->pseudo_begin = 1;} 
	#define ALL_CKECKS CHECK_FIRST S->statements++; CHECK_LAST 
	#define L_VALUE_CHECK temp = strdup(yytext); ALL_CKECKS return_id(temp); 
	#define L_VALUE_CHECK_STRING temp = strdup(yytext); return_string(temp); rewrite_set_value(); ALL_CKECKS
	#define L_VALUE_CHECK_ARRAY temp = strdup(yytext); ALL_CKECKS return_id(temp);
	#define CHECK if(S->beggining>=kena){CHECK_FIRST CHECK_LAST}
%}


L [a-zA-Z]
D [0-9]
W [ \t\r]
Lh [a-fA-F]

%x comment
%x string

%%

"and"		{ return T_and; }
"as"		{ return T_as; }
"begin" 	{ S->statements = 0; S->has_begin = 1; /*???yylex()???*/ }
"break"		{ ALL_CKECKS return T_break; }	
"byte"		{ return T_byte; }
"continue"	{ ALL_CKECKS return T_continue; }
"decl"		{ CHECK push(kena); S->is_def=1; return T_decl; }
"def"		{ CHECK is_def = 1; if(!isempty()){push(kena);} S->is_def=1; return T_def; }
"elif"		{ ALL_CKECKS push(kena); return T_elif; }
"else"		{ ALL_CKECKS push(kena); return T_else; }
"end"		{ if(!S->has_begin){read_again(); yytext="end"; pop(); return T_end;}pop(); return T_end; }
"exit"		{ ALL_CKECKS return T_exit; }
"false"		{ return T_false; }
"if"		{ ALL_CKECKS push(kena); return T_if; }
"is"		{ return T_is; }
"int"		{ return T_int; }
"loop"		{ ALL_CKECKS push(kena); return T_loop; }
"not"		{ return T_not; }
"or"		{ return T_or; }
"ref"		{ return T_ref; }
"return"	{ ALL_CKECKS return T_return; }
"skip"		{ ALL_CKECKS return T_skip; }
"true"		{ return T_true; }
"var"		{ CHECK return T_var; }
"<>"		{ return T_not_equal; }
"<="		{ return T_less_equal; }
">="		{ return T_greater_equal; }
":="		{ var=0; return T_set_value; }

[()+\-*/%!&|=<>\[\],:] { var=0; return yytext[0]; }

'{L}'		{ yylval.n=yytext[1]; return T_characters; }
'{D}'		{ yylval.n=yytext[1]; return T_characters; }
'\\x({Lh}|{D})({Lh}|{D})'	{

		char c1 = to_number(yytext[3]);
		char c2 = to_number(yytext[4]);
        yylval.n = c1*16 + c2; 
		return T_characters; }

\'\\0\'			{ yylval.n = '\0'; return T_characters; }
\'\\n\'			{ yylval.n = '\n'; return T_characters; }
\'\\t\'			{ yylval.n = '\t'; return T_characters; }
\'\\r\'			{ yylval.n = '\r'; return T_characters; }
\'\\\\\'			{ yylval.n = '\\'; return T_characters; }
\'\\\'\'			{ yylval.n = '\''; return T_characters; }
\'\\\"\'			{ yylval.n = '\"'; return T_characters; }		



{L}({L}|{D})*{W}*("[".*"]"{W}*)?":="	{ /*set value to an id so it is a statement*/  L_VALUE_CHECK_ARRAY  yylval.s = strdup(yytext); id_line1=countlines; if (var==0) id_line2=countlines; else id_line2= prev_l; prev_l=countlines; var=1; return T_id; }

{L}({L}|{D})*		{ if (is_def == 1 ){is_def = 0; add_to_list(yytext);}else{if(in_list()){ ALL_CKECKS } }  yylval.s = strdup(yytext); id_line1=countlines; if (var==0) id_line2=countlines; else id_line2= prev_l; prev_l=countlines; var=1; return T_id;  }

{D}+    		{ yylval.n=atoi(yytext);  return T_const; }


\"([^\\\"]|\\.)*\"{W}*":=" { id_line1 = countlines; L_VALUE_CHECK_STRING yylval.s = strdup(yytext); if (var==0) id_line2=countlines; else id_line2= prev_l; prev_l=countlines; var=1; return T_id; }

\"([^\\\"]|\\.)*\"  	{yylval.s = strdup(yytext); id_line1 = countlines; if (var==0) id_line2=countlines; else id_line2= prev_l; prev_l=countlines; var=1; return T_string;}


"(*"			{ num_comment = 1; BEGIN(comment); }
<comment>[^*(\n]* 	{ /* eat anything that’s not a ’*’ */ }
<comment>"("+[^(*\n]* 
<comment>"*"+[^*)\n]* 
<comment>"("+"*"	{ num_comment++; }
<comment>\n 		{ countlines++; }
<comment>"*"+")" 	{ num_comment--; if (num_comment==0) BEGIN(0);  }	

#.*\n			{ kena=0; countlines++; } 



[ ]	    	{ kena++; }
[\t]		{ kena+=TAB_SIZE; }
[\n]		{ kena=0; countlines++; }


"*)"		{ my_countlines = countlines; fatal("A comment closed that never opened"); }

<<EOF>>		{ /*if end of file close all unclosed blocks*/ if(S->statements <= 1 && S->has_begin == 0 && S->pseudo_begin == 0){S->pseudo_begin = 1; } if(S->has_begin == 0){ yytext="end"; pop(); if (num_comment > 0 ) { my_countlines = countlines; fatal("Comment never closed"); }return T_end; } if (num_comment>0){ my_countlines = countlines; fatal("Comment never closed"); } return T_eof; }

.		{ my_countlines = countlines; fatal("symbol \'%s\' not recognized" , yytext); }


%%

