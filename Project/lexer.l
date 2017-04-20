%{
	#include <stdlib.h>
        #include <stdio.h>
        #include "parser.h"
        #include "lexer.h"

	#define T_eof 0	
	#define MAX_STRING_SIZE 256	
	#define TAB_SIZE 8
	int countlines = 1;
	int num_comment;
	int num_eisagwgika;
	int kena;
	int statements = 0;
	int token;
	char *temp;

	
	static void yyunput (int c, char * yy_bp );

	void my_free(List *temp){
		if (temp!=NULL){
			if (temp->next != NULL)
				my_free(temp->next);
		}
		free(temp);
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
		Node* new = malloc(sizeof(Node));
		new->next = S;
		new->beggining = spaces;
		new->statements = 0;
		new->has_begin = 0;
		new->pseudo_begin = 0;
		new->is_def = 0;
		new->definitions = NULL;
		S = new;
	}

	int checkit(int sp) {
		if (isempty())
			printf("Something went wrong with blocks at line %d\n",countlines);	
		if (S->beggining < sp) return 0;
		return 1;
	}

	int check(int sp){
		//if (isempty())
		//	printf("\n");
		//else
		//	printf("..first on the stack is %d\n", S->beggining);
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
		yytext = malloc(i*sizeof(char));
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
		yytext = malloc(i*sizeof(char));
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
		List *new = malloc(sizeof(List));
		new->next = S->next->definitions;
		new->name = strdup(name);
		S->next->definitions = new;
	}
	
	int in_list(){
		Node* temp = S;
		while (temp!=NULL){
			List* iter = temp->definitions;
			while (iter!=NULL){
				if (strcmp(iter->name,yytext)==0)
					return 1;
				iter = iter->next;
			}
			temp = temp->next;
		}
		return 0;
	}
	
	#define CHECK_LAST if(S->has_begin == 0 && check(kena)){if(S->pseudo_begin == 0){S->pseudo_begin = 1; read_again(); yytext = "begin"; return T_begin;} read_again(); yytext="end"; pop(); return T_end; }
	#define CHECK_FIRST if(S->statements <= 1 && S->has_begin == 0 && S->pseudo_begin == 0){read_again(); yytext="begin"; S->pseudo_begin = 1; return T_begin; } 
	#define ALL_CKECKS CHECK_FIRST S->statements++; CHECK_LAST
	#define L_VALUE_CHECK temp = strdup(yytext); ALL_CKECKS return_id(temp); 
	#define L_VALUE_CHECK_STRING temp = strdup(yytext); return_string(temp); rewrite_set_value(); ALL_CKECKS
	#define L_VALUE_CHECK_ARRAY temp = strdup(yytext); ALL_CKECKS return_id(temp);
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
"begin" 	{ S->statements = 0; S->has_begin = 1; return T_begin; }
"break"		{ ALL_CKECKS return T_break; }	
"byte"		{ return T_byte; }
"continue"	{ ALL_CKECKS return T_continue; }
"decl"		{ ALL_CKECKS push(kena); S->is_def=1; return T_decl; }
"def"		{ if(!isempty()){ALL_CKECKS push(kena);} S->is_def=1; return T_def; }
"elif"		{ ALL_CKECKS push(kena); return T_elif; }
"else"		{ ALL_CKECKS push(kena); return T_else; }
"end"		{ if(isempty()){printf("Not a begin corresponding to end at line %d\n" , countlines);} if(!S->has_begin){read_again(); yytext="end"; pop(); return T_end;}pop(); return T_end; }
"exit"		{ ALL_CKECKS return T_exit; }
"false"		{ return T_false; }
"if"		{ printf("statements %d has begin %d pseudo %d\n" , S->statements , S->has_begin , S->pseudo_begin);ALL_CKECKS push(kena); return T_if; }
"is"		{ return T_is; }
"int"		{ return T_int; }
"loop"		{ ALL_CKECKS push(kena); return T_loop; }
"not"		{ return T_not; }
"or"		{ return T_or; }
"ref"		{ return T_ref; }
"return"	{ ALL_CKECKS return T_return; }
"skip"		{ ALL_CKECKS return T_skip; }
"true"		{ return T_true; }
"var"		{ ALL_CKECKS return T_var; }
"<>"		{ return T_not_equal; }
"<="		{ return T_less_equal; }
">="		{ return T_greater_equal; }
":="		{ return T_set_value; }

[()+\-*/%!&|=<>\[\],:] { return yytext[0]; }

'{L}'		{ return T_characters; }
'{D}'		{ return T_characters; }
'\\x({Lh}|{D})({Lh}|{D})'	{
		int to_number(char c){
			if      (c >= 'a' && c <= 'f')  return c - 'a';
			else if (c >= 'A' && c <= 'F')  return c - 'A';
			return c - '0';
		}
	
		char c1 = to_number(yytext[3]);
		char c2 = to_number(yytext[4]);
		yytext[1] = c1*16 + c2;
		return T_characters; }
'[\n\t\r\0\\\'\"]' 	{ return T_characters; }
\'\\0\'			{ return T_characters; }
\'\\n\'			{ return T_characters; }
\'\\t\'			{ return T_characters; }
\'\\r\'			{ return T_characters; }
\'\\\\\'			{ return T_characters; }
\'\\\'\'			{ return T_characters; }
\'\\\"\'			{ return T_characters; }		



{L}({L}|{D})*{W}*("[".*"]"{W}*)?":="	{ /*set value to an id so it is an statement*/ L_VALUE_CHECK_ARRAY  return T_id; }

{L}({L}|{D})*		{ if (token==T_def){add_to_list(yytext);}else{if(in_list()){ ALL_CKECKS } } return T_id; }
{D}+    		{ return T_const; }


\"([^\\\"]|\\.)*\"{W}*":=" { L_VALUE_CHECK_STRING return T_id; }

\"([^\\\"]|\\.)*\"  	{return T_string;}


"(*"			{ num_comment = 1; BEGIN(comment); }
<comment>[^*(\n]* 	{ /* eat anything that’s not a ’*’ */ }
<comment>"("+[^(*\n]* 
<comment>"*"+[^*)\n]* 
<comment>"("+"*"	{ num_comment++; }
<comment>\n 		{ countlines++; }
<comment>"*"+")" 	{ num_comment--; if (num_comment==0) BEGIN(0);  }	

#.*\n			{ countlines++; } 



[ ]	    	{ kena++;}
[\t]		{ kena+=TAB_SIZE; }
[\n]		{ kena=0; countlines++; }


"*)"		{ printf("At line %d a comment closed that never opened\n" , countlines); }

<<EOF>>		{ /*if end of file close all unclosed blocks*/if(S->has_begin == 0){ yytext="end"; pop(); return T_end; } return T_eof; }

.		{ printf("symbol \'%s\' in line %d not recognized\n" , yytext ,countlines ); }


%%

