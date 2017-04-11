%{
	#define T_eof 0
	#define T_and 1001
	#define T_as 1002
	#define T_begin 1003
	#define T_break 1004
	#define T_byte 1005
	#define T_continue 1006
	#define T_decl 1007
	#define T_def 1008
	#define T_elif 1009
	#define T_else 1010
	#define T_end 1011
	#define T_exit 1012
	#define T_false 0
	#define T_if 1014
	#define T_is 1015
	#define T_int 1016
	#define T_loop 1017
	#define T_not 1018
	#define T_or 1019
	#define T_ref 1020
	#define T_return 1021
	#define T_skip 1022
	#define T_true 1
	#define T_var 1024

	#define T_not_equal 1025
	#define T_less_equal 1026
	#define T_greater_equal 1027
	#define T_set_value 1028

	#define T_id 1029
	#define T_const 1030
	#define T_characters 1031
	#define T_string 1032
        
	
	#define MAX_STRING_SIZE 256	
	#define TAB_SIZE 8	
	int countlines = 1;
	int num_comment;
	int num_eisagwgika;
	int kena;
	int begin_exists;

	
	typedef struct node {
		struct node* next;
		int beggining;
	} Node;

	typedef Node* stack;
	stack S=NULL;
	
	void pop(){
		Node* temp = S;		
		S = S->next;
		free(temp);
		//yytext = "end"
		
	}

	int isempty(){
		if (S==NULL)
			return 1;
		else
			return 0;
	}
	
	void push(int spaces){
		printf("begin\n");
		Node* new = malloc(sizeof(Node));
		new->next = S;
		new->beggining = spaces;
		S = new;
	}

	void checkit (int sp) {
		if (S==NULL)
			printf("vlakeia\n");	
		if (S->beggining < sp) return;
		else {
			printf("end\n");
			pop();
			checkit(sp); 
		}
	}

	void check(int sp){
		if (begin_exists==1)
			return;
		checkit(sp);
	}

	
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
"begin" 	{ begin_exists = 1;return T_begin; }
"break"		{ check(kena); return T_break; }	
"byte"		{ return T_byte; }
"continue"	{ check(kena); return T_continue; }
"decl"		{ return T_decl; }
"def"		{ begin_exists = 0; push(kena); return T_def; }
"elif"		{ check(kena); begin_exists = 0; push(kena); return T_elif; }
"else"		{ check(kena); begin_exists = 0; push(kena); return T_else; }
"end"		{ return T_end; }
"exit"		{ check(kena); return T_exit; }
"false"		{ return T_false; }
"if"		{ check(kena); begin_exists = 0; push(kena); return T_if; }
"is"		{ return T_is; }
"int"		{ return T_int; }
"loop"		{ check(kena); begin_exists = 0; push(kena); return T_loop; }
"not"		{ return T_not; }
"or"		{ return T_or; }
"ref"		{ return T_ref; }
"return"	{ check(kena); return T_return; }
"skip"		{ check(kena); return T_skip; }
"true"		{ return T_true; }
"var"		{ return T_var; }

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
'[\n\t\r\0\\\'\"]' {return T_characters; }


{L}({L}|{D})*	{ check(kena); return T_id; }
{D}+    	{ return T_const; }


\"([^\\\"]|\\.)*\"  {return T_string;}


"(*"		{num_comment = 1; BEGIN(comment); }
<comment>[^*(\n]* {/* eat anything that’s not a ’*’ */ }
<comment>"("+[^(*\n]* 
<comment>"*"+[^*)\n]* 
<comment>"("+"*" {num_comment++; }
<comment>\n 	{countlines++; }
<comment>"*"+")" {num_comment--; if (num_comment==0) BEGIN(0);  }	

#.*\n		{ countlines++; } 



[ ]	    	{ kena++;}
[\t]		{ kena+=TAB_SIZE; }
[\n]		{ kena=0; countlines++; }


"*)"		{ printf("At line %d a comment closed that never opened\n" , countlines); }
.		{ printf("symbol %s in line %d not recognized\n" , yytext ,countlines ); }


%%

int main () {
	int token;
	do {
		token = yylex();
		if (token != T_characters)
			printf("token=%d, lexeme=\"%s\"\n", token, yytext);
		else
			printf("token=%d, lexeme=\"%c\"\n", token, yytext[1]);
		kena += strlen(yytext);
	} while (token != T_eof);
	if (num_comment != 0) 
		printf("Not equal number of comments\n");
	return 0;
}
