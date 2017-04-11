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
#define T_smaller_equal 1026
#define T_bigger_equal 1027
#define T_set_value 1028

#define T_id 1029
#define T_const 1030
#define T_characters 1031

int countlines = 1;
int num_comment;
int num_eisagwgika;

%}


L [a-zA-Z]
D [0-9]
W [ \t\r]

%x comment
%x string

%%

"and"		{ return T_and; }
"as"		{ return T_as; }
"begin" 	{ return T_begin; }
"break"		{ return T_break; }	
"byte"		{ return T_byte; }
"continue"	{ return T_continue; }
"decl"		{ return T_decl; }
"def"		{ return T_def; }
"elif"		{ return T_elif; }
"else"		{ return T_else; }
"end"		{ return T_end; }
"exit"		{ return T_exit; }
"false"		{ return T_false; }
"if"		{ return T_if; }
"is"		{ return T_is; }
"int"		{ return T_int; }
"loop"		{ return T_loop; }
"not"		{ return T_not; }
"or"		{ return T_or; }
"ref"		{ return T_ref; }
"return"	{ return T_return; }
"skip"		{ return T_skip; }
"true"		{ return T_true; }
"var"		{ return T_var; }

"<>"		{return T_not_equal; }
"<="		{return T_smaller_equal; }
">="		{return T_bigger_equal; }
":="		{return T_set_value; }

[()+\-*/%!&|=<>\[\],:] { return yytext[0]; }

'{L}'		{ printf("ues\n");return yytext[1]; }

{L}({L}|{D})*	{ return T_id; }
{D}+    	{ return T_const; }





"(*"		{num_comment = 0; BEGIN(comment); }
<comment>[^*(\n]* {/* eat anything that’s not a ’*’ */ }
<comment>"("+[^(*\n]* 
<comment>"*"+[^*)\n]* 
<comment>"("+"*" {num_comment++; }
<comment>\n 	{countlines++; }
<comment>"*"+")" { if (num_comment==0) BEGIN(0); else num_comment--; }	

#.*\n		{ countlines++; } 



{W}+    	{ /* nothing */ }
[\n]		{ countlines++; }

.		{ printf("symbol %s in line %d not recognized\n" , yytext ,countlines ); }


%%

int main () {
	int token;
	do {
		token = yylex();
		printf("token=%d, lexeme=\"%s\"\n", token, yytext);
	} while (token != T_eof);
	printf("num_comment is %d\n" , num_comment);
	if (num_comment!=0)
		printf("Plesase insert a valid number of start and close comments\n");
	return 0;
}
