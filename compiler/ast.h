#ifndef __AST_H__
#define __AST_H__

#include "symbol.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>
#if defined(LLVM_VERSION_MAJOR) && LLVM_VERSION_MAJOR >= 4
#include <llvm/Transforms/Scalar/GVN.h>
#endif

typedef enum {
	IF = 0, ELIF, INT, CHAR, SEQ,
    HEADER, LET, VAR, PLUS, MINUS,
    TIMES, DIV, MOD, NOT_E, AND_E, 
    OR_E, NOT_C, AND_C, OR_C, GREATER,
    LOWER, GREATEREQ, LOWEREQ, EQUAL, DIFFERENT, 
    VARDEF, SKIP, EXIT, BREAK, CONTINUE, 
    RETURN, FUNCDEF, PROCCALL, FUNCCALL, EXPRLIST,
    OTHER, LOOP, FPARDEF, REF, STR_VAR
} kind;

//typedef union { *mark_id, *mark_loop, *mark_func } ast;

typedef struct list_node { 
    int line;
    struct list_node* next;
} *my_list;

typedef struct declarations {
//	char *id_name;
	Dimensions dim;
	my_Type T;
	struct declarations *next;
} *Declarations;

union my_types{
    struct {
        char *id_name;
        struct mark* expr;
        int nesting_diff;
        int offset;
    } lvalue;

    struct { 
        struct mark *left,*right;
        short is_number;
    } op;

    struct {
        char* loop_id;
        struct mark* block;
	int again = 1;
    } loop;

    struct {
        char* loop_id;
        struct mark* loop;
    } break_continue;

    struct {
        struct mark *header,*localdef,*block;
        int num_vars;
    } func;

    struct {
        char *func_name;
        my_Type T;
	int from_libraries; 
        struct mark *func_decl;
    } func_header;

    struct { 
        my_Type T;
        Dimensions dim;
    } TypeStruct;

    struct {
        my_Type T;
        int val;
    } my_const;

    struct {
        struct mark *cond;
        struct mark *block;
        struct mark *st_list;
        struct mark *else_part;
	Value* my_cond;
    } if_stmt;
};



typedef struct mark {
	kind k;
    union my_types u;
    short is_array;                 // also used for seq to declare not necessary addition
    int line_number;
    short ar;			   // used to check if a function wants to return an array
    my_Type general_type;
} *ast;



//void print_quad();


extern ast ast_make_loop(kind k,char* loop_id,ast block,int cnt);
extern ast ast_make_type(kind k,char* datatype, Dimensions dim,int cnt);
extern ast ast_make_lvalue(kind k,char* name, ast exp, int cnt);
extern ast ast_make_op(kind k,ast left,ast right, int cnt);
extern ast ast_make_func(kind k,ast h,ast def_l, ast block, int cnt);
extern ast ast_make_header(kind k, char* name, char* datatype, int h, ast declaration_list, int cnt);
extern ast ast_make_const(kind k,char* datatype, int value, int cnt);
extern ast ast_make_if(kind k,ast cond,ast block, ast list, ast els, int cnt);
extern ast ast_make_break(kind k,char* loop_id, int cnt);
void llvm_compile_and_dump (ast t, int optimize);
extern Dimensions dim_add(int n,Dimensions next,int check);
#endif
