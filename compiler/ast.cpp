/*
 *  Added more loose type checks!
 *  Not only boolean but also char and Integer can form boolean expressions
 *  Also changed CHECK_SEM condition
 *  Not sure if correct
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "error.h"
#include "general.h"
#include "list.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <map>
#include <stack>
#include <vector>

#include "llvm/Passes/PassBuilder.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include <llvm/IR/CallingConv.h>
#include "llvm/ADT/APInt.h"
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

using namespace llvm;
Value* ast_sem (ast t);
std::vector<Value *> list_of_arg {};
Function* my_main = nullptr;
Module *module;
BasicBlock* BB;
SymbolEntry * p;
std::stack<ast> else_ast;
std::vector<BasicBlock* > after_loops;
std::vector<BasicBlock* > inside_loops;
std::vector<Type*> argTypes;
std::vector<Value*> args;
std::map<std::string, Type*> func_locals ;
std::vector<std::string> func_names;
std::stack<std::vector<std::string>> function_decl ;

std::stack<int> array_sizes;
int pushed_blocks;
int loop_index;
int return_pointer;               
int var_array = 0;
int my_countlines = 0;

GlobalVariable* new_string;
std::map<std::string, std::stack<GlobalVariable*>> globals ;

#define MAXLEN 100
char str1[MAXLEN];
char str2[MAXLEN];
char snum[MAXLEN];

struct CodeGenBlock {

    BasicBlock *block;
    std::map<std::string, Value*> locals ;
    CodeGenBlock* prev;
    std::vector<std::string> names;
    int a = 1;
    int is_Function = 0;
};

typedef CodeGenBlock* blockp;
blockp b_start = NULL;

void push_block (BasicBlock* a) {

    blockp new_b = new CodeGenBlock ;
    new_b->block = a;

    if (b_start == NULL) {
        new_b-> prev = NULL;
        b_start= new_b; }
    else {

        new_b->prev = b_start;
        b_start = new_b; }

    b_start->locals[" "] = NULL;
    return;
}

void pop_block() {

    if (b_start == NULL) {
        fprintf(stderr,"no block to pop\n");
        exit(1);
    }
    else {
        blockp tmp = b_start;
        b_start= b_start -> prev;
        tmp->prev = NULL;
        delete(tmp); }

}

BasicBlock* get_top() {

    if (b_start == NULL) {
        return NULL; }
    else{
        return b_start->block;  }
}

// Global LLVM variables related to the LLVM suite.
static LLVMContext TheContext;                       // a general class
static IRBuilder<> Builder(TheContext);    	     // This provides a uniform API for creating instructions and inserting them into a block
static std::unique_ptr<Module> TheModule;            // a module contains a list of global variables, functions, libraries etc.
static std::unique_ptr<legacy::FunctionPassManager> TheFPM; // manages FunctionPasses and BasicBlockPassManagers.
FunctionAnalysisManager* myFPM;

// Useful LLVM types.
static Type * i8 = IntegerType::get(TheContext, 8);   // This static method is the primary way of constructing an IntegerType. Only one 	    					      // instance with a given NumBits value is ever created
static Type * i32 = IntegerType::get(TheContext, 32);
MemoryDependenceAnalysis MDA ;


// Useful LLVM helper functions.
Value* c8(char c) {                     // Compiler replaces the definition of inline functions at compile time instead of 	    					     // referring function definition at runtime to reduce execution time . 
    // ConstantInt: This class represents both boolean and integral constants.
    return ConstantInt::get(TheContext, APInt(8, c, false));  //Create a new  unsigned APInt of 8 Bits width, initialized as c.
}

Value* c32(int n) {		     // return a new signed int of 32 Bits

    return ConstantInt::get(TheContext, APInt(32, n, true)); 
}


/****************************************************************************************************************************************/

#define INIT_AST(t,cnt) t->line_number = cnt;
#define NOTHING 0

char msg1[100] = "type mismatch in assignment of variable ";
char msg2[100] = "invalid programm call of ";
char msg3[100] = "No loop with id ";
char msg4[100] = "type mismatch in return value of function ";
char msg5[100] = "wrong lvalue: ";
char msg6[100] = "this is not a lvalue: ";


void my_fatal(int newline, const char* msg){
    my_countlines = newline;
    fatal(msg);
}


const char* type_kind[] = {"IF", "ELIF", "INT", "CHAR", "SEQ",
    "HEADER", "LET", "VAR", "PLUS", "MINUS",
    "TIMES", "DIV", "MOD", "NOT_E", "AND_E",
    "OR_E", "NOT_C", "AND_C", "OR_C", "GREATER",
    "LOWER", "GREATEREQ", "LOWEREQ", "EQUAL", "DIFFERENT",
    "VARDEF", "SKIP", "EXIT", "BREAK", "CONTINUE",
    "RETURN", "FUNCDEF", "PROCCALL", "FUNCCALL", "EXPRLIST",
    "OTHER", "LOOP", "FPARDEF", "REF", "STR_VAR"
};


/* Macros used for type checking */

#define CHECK_SEM(type)  					                                                                                \
    if (!(((equalType(t->u.op.left->general_type, typeInteger) && equalType(t->u.op.right->general_type, typeInteger))) ||	    \
                ((equalType(t->u.op.left->general_type, typeChar) && equalType(t->u.op.right->general_type, typeChar)))) ){		\
        my_countlines = t->u.op.left->line_number;     									                                        \
        fatal("type mismatch in %s operator",type);								                                                \
    }								   							                                                                \
check_valid_type(t->u.op.left);												                                                \
check_valid_type(t->u.op.right);                                                


/* Macros for adjusting the correct type after the operation */

#define CHECK_TYPE   									                    \
    if (equalType(t->u.op.left->general_type, typeInteger) ||			    \
            equalType(t->u.op.right->general_type, typeInteger))			\
t->general_type = typeInteger;						                \
else										                            \
t->general_type = typeChar;					


#define CHECK_BOOL(general_type) 								                        \
    if (!equalType(general_type, typeBoolean) && !equalType(general_type, typeChar) && 	\
            !equalType(general_type, typeInteger)){  						                \
        my_countlines = t->u.op.left->line_number;						                    \
        fatal("type mismatch in not operator");						                    \
    }     

#define CHECK_BOOL1(general_type, c)								                        \
    if (!equalType(general_type, typeBoolean) && !equalType(general_type, typeChar)){	\
        my_countlines = t->u.op.left->line_number; 					                    \
        if ( c == '&') 					\
        fatal("type mismatch in '&' operator");					                    \
        else if ( c == '|' )				\
        fatal("type mismatch in '|' operator");   \
        else if ( c == '!' )				\
        fatal("type mismatch in '!' operator");   \
        else						\
        fatal("type mismatch in this operator");  \
    }

#define EXIT_NO_MEM(t)                          \
    if (t == NULL){                             \
        fprintf(stderr, "Out of memory\n");     \
        exit(1);                                \
    }

/* 
 * 2 stacks used one for the loop_id to get the corresponding ast node for the right break or cintinue 
 * another to check the function and the nesting level of it to correspond the return statement to the 
 * correct function and make the appropriate type checks 
 * */

my_stack Loops = NULL;        
my_stack Functions = NULL;

/* 
 * Find the correct loop_id for break_continue statements.
 * If loop_ip == NULL returns the top element of the stack
 * */

ast find_element(char* loop_id, my_stack S){
    loop_index = 0;
    if (S == NULL) {
        return NULL; 
    }

    if (loop_id == NULL) 
        return S->element;
    my_stack temp = S;
    while(temp!=NULL){
        if (temp->element->u.loop.loop_id != NULL) {
            if (strcmp(temp->element->u.loop.loop_id,loop_id) == 0)
                return temp->element;
        }
        loop_index++;
        temp = temp->next;
    }
    return NULL;
}


/* Remove from stack */
extern int my_remove(my_stack *S){
    if (*S==NULL)
        return 1;
    my_stack temp = (*S)->next;
    free(*S);
    *S = temp;
    return 0;
}

/* Insert to stack */
void my_insert(ast element,my_stack *S){  

    my_stack s = (my_stack) malloc(sizeof(struct stack_node));
    if (*S == Loops and (element->u.loop.loop_id != NULL)) {
        ast search = find_element(element->u.loop.loop_id,Loops);
        if (search != NULL) {
            my_countlines = element->line_number;
            fatal("Duplicate loop identifier %s", element->u.loop.loop_id);
        }
    }
    EXIT_NO_MEM(s);
    s->next = *S;
    s->element = element;
    *S = s;
}

extern my_list merge_lists(my_list l1,my_list l2){
    if (l1 == NULL)
        return l2;
    while (l1->next!=NULL){
        l1 = l1->next;
    }
    l1->next = l2;
    return l1;
}

/*
 * Store the dmensions of var definitions 
 * I check == 1 allows the dimension value of -1 used to represent unknown value
 * */

Dimensions dim_add(int n,Dimensions next,int check){
    if (!check){
        if (n<0){
            fatal("Dimensions cannot be of negative value");
        }
    }
    Dimensions dim;
    dim = (Dimensions) malloc(sizeof(struct dimensions));
    EXIT_NO_MEM(dim);
    dim->arraysize=n;
    dim->next=next;
    return dim;
}

/* Functions used to make the ast tree */



ast ast_make_loop(kind k,char* loop_id,ast block,int cnt){
    ast p;
    p = (ast) malloc(sizeof(struct mark));
    EXIT_NO_MEM(p);

    p->k = k;
    p->u.loop.loop_id = loop_id;
    p->u.loop.block = block;
    INIT_AST(p,cnt);
    return p; 
}

ast ast_make_type(kind k,char* datatype, Dimensions dim,int cnt){
    ast p;
    p = (ast) malloc(sizeof(struct mark));
    EXIT_NO_MEM(p);   

    p->k = k;
    if (!strcmp(datatype,"int")){
        p->u.TypeStruct.T = typeInteger;
    }
    else{
        p->u.TypeStruct.T = typeChar;
    }
    p->u.TypeStruct.dim=dim;
    INIT_AST(p,cnt);
    return p;
}

ast ast_make_lvalue(kind k,char* name, ast exp, int cnt){
    ast p;
    p = (ast) malloc(sizeof(struct mark));
    EXIT_NO_MEM(p);
    p->k = k;
    p->u.lvalue.id_name = name;

    p->u.lvalue.expr = exp;	
    INIT_AST(p,cnt);
    return p;
}

ast ast_make_op(kind k,ast left,ast right, int cnt){
    ast p;
    p = (ast) malloc(sizeof(struct mark));
    EXIT_NO_MEM(p);
    p->k = k;
    p->u.op.left = left;
    p->u.op.right = right;
    p->is_array = 0;
    INIT_AST(p,cnt);
    return p;
}

ast ast_make_func(kind k,ast h,ast def_l, ast block, int cnt){
    ast p;
    p = (ast) malloc(sizeof(struct mark));
    EXIT_NO_MEM(p);
    p->k = k;
    p->u.func.header = h;
    p->u.func.localdef = def_l;
    p->u.func.block = block;
    INIT_AST(p,cnt);
    return p;
}

ast ast_make_header(kind k, char* name, char* datatype, int h, ast declaration_list, int cnt) {
    ast p;
    p = (ast) malloc(sizeof(struct mark));
    EXIT_NO_MEM(p);
    p->k = k;
    p->u.func_header.func_name = name;
    p->u.func_header.from_libraries = h;
    if (datatype == NULL){
        p->u.func_header.T = typeVoid; 
    }else{
        if (!strcmp(datatype,"int")){
            p->u.func_header.T = typeInteger;
        }
        else if  (!strcmp(datatype,"byte")){
            p->u.func_header.T = typeChar;			
        }
        else{
            p->u.func_header.T = typeVoid;			/* Void Type for function */
        }
    }
    p->u.func_header.func_decl = declaration_list;
    INIT_AST(p,cnt);
    return p;
}

ast ast_make_const(kind k,char* datatype, int value, int cnt){
    ast p;
    p = (ast) malloc(sizeof(struct mark));
    EXIT_NO_MEM(p);
    p->k = k;
    if (!strcmp(datatype,"int")){
        p->u.my_const.T = typeInteger;
        p->u.my_const.val = value; }

    else if  (!strcmp(datatype,"byte")){
        p->u.my_const.T = typeChar;
        p->u.my_const.val = value;			
    }
    else if (!strcmp(datatype,"char")){ 		
        p->u.my_const.T = typeChar;
        p->u.my_const.val = value;
    }
    else{
        p->u.my_const.T = typeBoolean;
        p->u.my_const.val = value;
    }
    INIT_AST(p,cnt);
    return p;
}

ast ast_make_if(kind k,ast cond,ast block, ast list, ast els, int cnt){
    ast p;
    p = (ast) malloc(sizeof(struct mark));
    EXIT_NO_MEM(p);
    p->k = k;
    p->u.if_stmt.cond = cond;
    p->u.if_stmt.block = block;
    p->u.if_stmt.st_list = list;
    p->u.if_stmt.else_part = els;
    INIT_AST(p,cnt);
    return p;
}

ast ast_make_break(kind k,char* loop_id, int cnt){
    ast p;
    p = (ast) malloc(sizeof(struct mark));
    EXIT_NO_MEM(p);
    p->k = k;
    p->u.break_continue.loop_id = loop_id;
    INIT_AST(p,cnt);
    return p;
}


int dimension_check(ast lvalue){
    my_countlines = lvalue->line_number;
    SymbolEntry *e = lookupEntry(lvalue->u.lvalue.id_name, LOOKUP_ALL_SCOPES, true);
    Dimensions dim = e->u.eVariable.dim;
    ast test = lvalue->u.lvalue.expr;
    while (test!=NULL){     
        if (dim == NULL)
            my_fatal(lvalue->line_number,"Incorrect dimensions for array");
        /* not testing expr value between [] at this level */
        dim = dim->next;
        test = test->u.op.right;
    }
    if (dim == NULL)
        return 0;
    return 1;
}

void check_valid_type(ast t){

    if (t == NULL) return;
    if (t->k == VAR){
        if(dimension_check(t))
            my_fatal(t->line_number,"Cannot use arrays in operation");
    }
    else if (t->k == STR_VAR){

        if( t->u.lvalue.expr == NULL)        
            my_fatal(t->line_number,"Cannot use string in operation");
        else if (((t->u.lvalue.expr)->u.op.right)!=NULL)
            my_fatal(t->line_number,"String is not a multidimensional array");
    }

}


/* Count the number os dimensions through the list created */
int count_dimensions(Dimensions dim){
    Dimensions temp = dim;
    int count = 0;
    while (temp!=NULL){
        count++;
        temp = temp->next;
    }
    return count;
}

/*
 * Checks the arguments used for the calling of each function
 * Checks the number of arguments and the type of each argument used (type and dimensions if an array)
 * */


int check_proccall(SymbolEntry* Argument,ast expr_list, char* name){    /* 1 correct result, 0 false result */
    ast test = expr_list;
    if (test == NULL){
        if (Argument == NULL)
            return 1; 
        else 
            return 0;
    }
    if (Argument == NULL){
        if (test != NULL) 
            return 0;
    }
    while(test!=NULL){
        if (Argument == NULL) 
            return 0;
        if (test->k!=EXPRLIST)
            fatal("Expected node of type EXPRLIST... something went wrong...");

        if (Argument->u.eParameter.mode == PASS_BY_REFERENCE)
            return_pointer = 1; 
        Value* arg = ast_sem(test->u.op.left);
        if (Argument->u.eParameter.mode == PASS_BY_REFERENCE)
            return_pointer = 0;

        args.push_back(arg);

        my_countlines = (test->u.op.left)->line_number;
        if (!equalType(test->u.op.left->general_type, Argument->u.eParameter.type))
            fatal("type mismatch in parameter of the calling function: %s", name);
        if (test->u.op.left->k == VAR){
            my_countlines = test->u.op.left->line_number;
            SymbolEntry *e = lookupEntry(test->u.op.left->u.lvalue.id_name, LOOKUP_ALL_SCOPES, true);
            Dimensions original_dim = e->u.eVariable.dim;
            /* remove extra dimensions used for call*/
            int original_dimensions = count_dimensions(original_dim);
            int call_dimensions = 0;
            ast temp = test->u.op.left->u.lvalue.expr;
            while (temp!=NULL){
                call_dimensions++;
                temp = temp->u.op.right;
            }

            int proc_dimensions = count_dimensions(Argument->u.eParameter.dim);
            if (original_dimensions != proc_dimensions + call_dimensions){
                fatal("Wrong dimensions in calling function: %s", name);
            }

        }else if (test->u.op.left->k == STR_VAR){

            int count_arg_dim = count_dimensions(Argument->u.eParameter.dim);
            if (test->u.op.left->u.lvalue.expr != NULL  && test->u.op.left->u.lvalue.expr->u.op.right!=NULL)
                fatal("String is not a multidimensional array");
            if(!((count_arg_dim==1 && test->u.op.left->u.lvalue.expr == NULL) || (count_arg_dim==0 && test->u.op.left->u.lvalue.expr!=NULL )))
                fatal("Wrong dimensions in calling function: %s", name);
        }
        else {
            int count_arg_dim = count_dimensions(Argument->u.eParameter.dim);
            if(!((count_arg_dim==1 && test->u.op.left->u.lvalue.expr == NULL) || (count_arg_dim==0 && test->u.op.left->u.lvalue.expr!=NULL )))
                fatal("Wrong dimensions in calling function: %s", name);
        }
        Argument = Argument->u.eParameter.next;
        test = test->u.op.right;
    }
    if (Argument != NULL)
        return 0; 
    return 1;
}

void initialize(){
    openScope();
}

void finalize(){
    closeScope();
}

int parameter;              // is it necessary?? 

int find_total_size(Dimensions dim){
    int result = 0;
    Dimensions temp = dim;
    while (temp!=NULL){
        if (temp == dim)
            result = temp->arraysize;
        else
            result *= temp->arraysize;
        temp = temp->next;
    }
    return result;
}

int is_zero;

int my_array_add_dimensions(Dimensions dim){
    if (dim == NULL){
        array_sizes.push((is_zero) ? 0 : 1);
        return 1;
    }
    int get_previous = my_array_add_dimensions(dim->next);
    array_sizes.push((is_zero) ? 0 : get_previous*dim->arraysize);
    return get_previous*dim->arraysize;
}


void array_add_dimensions(Dimensions dim){
    is_zero = 0; 
    Dimensions temp = dim;
    while (temp!=NULL){
        if (temp->arraysize == 0)
            is_zero = 1;
        temp = temp->next;
    }
    my_array_add_dimensions(dim->next);
}


int ref_var;

Value* ast_sem (ast t) {

    if (t == NULL) 
        return nullptr ;
    //fprintf(stderr,"\033[31;1m----New ast_sem with %s \033[0m\n" , type_kind[(t->k)]);

    switch (t->k) {
        case LET: {  

                      return_pointer = 1;
                      Value *l = ast_sem(t->u.op.left);
                      return_pointer = 0;
                      if (dimension_check(t->u.op.left)){
                          strcat(msg5,(t->u.op.left)->u.lvalue.id_name);
                          my_fatal(t->line_number, msg5);

                      }
                      check_valid_type(t->u.op.left);
                      Value *r = ast_sem(t->u.op.right);
                      check_valid_type(t->u.op.right);


                      if (!equalType(t->u.op.left->general_type, t->u.op.right->general_type)) {
                          strcat(msg1,(t->u.op.left)->u.lvalue.id_name);
                          my_fatal(t->line_number, msg1); }



                      BasicBlock* CurBB = b_start->block;
                      return new StoreInst(r, l, false, CurBB); 

                  }
        case LOOP: {              

                       my_insert(t,&Loops);
                       // Make the new basic block for the loop.
                       Function *TheFunction =get_top()->getParent(); // returns a pointer to the basic block
                       // creates a new basic block named "loop" 
                       // Insert an explicit fall-through from the current block, jump from the current to the loop block.


                       BasicBlock *InsideBB =
                           BasicBlock::Create(TheContext, "inside", TheFunction);
                       BasicBlock *AfterBB =
                           BasicBlock::Create(TheContext, "after", TheFunction);

                       after_loops.push_back(AfterBB);
                       inside_loops.push_back(InsideBB);

                       Builder.CreateBr(InsideBB);
                       Builder.SetInsertPoint(InsideBB);
                       push_block(InsideBB);
                       ast_sem(t->u.loop.block);

                       // End of loop.
                       Builder.CreateBr(InsideBB);
                       pop_block();

                       after_loops.pop_back();
                       inside_loops.pop_back();

                       push_block(AfterBB);
                       Builder.SetInsertPoint(AfterBB);
                       pushed_blocks ++ ;
                       my_remove(&Loops);
                       //backpatch(t->NEXT);
                       return nullptr;
                   }
        case IF: {       
                     BasicBlock* ThenBB;
                     BasicBlock* ElseBB;
                     BasicBlock* ElifBB;
                     BasicBlock* MergeBB;
                     Value *cond = ast_sem(t->u.if_stmt.cond);

                     if (t->u.if_stmt.cond->general_type != typeBoolean){
                         if (t->u.if_stmt.cond->general_type == typeInteger)
                             cond = Builder.CreateICmpNE(cond, c32(0), "tmpdif");
                         else
                             cond = Builder.CreateICmpNE(cond, c8(0), "tmpdif");
                     }


                     t->u.if_stmt.my_cond = cond;           // den kanei tipota ...
                     //backpatch(t->u.if_stmt.cond->TRUE);
                     CHECK_BOOL(t->u.if_stmt.cond->general_type);
                     check_valid_type(t->u.if_stmt.cond);   

                     pushed_blocks++;
                     // this is the false condition
                     Function *TheFunction = get_top()->getParent();

                     // create "then" block
                     ThenBB =
                         BasicBlock::Create(TheContext, "then", TheFunction);

                     // create "elif" block
                     if (t->u.if_stmt.st_list != NULL) 
                         ElifBB = BasicBlock::Create(TheContext, "elsif", TheFunction);

                     // create "else" block
                     if (t->u.if_stmt.else_part != NULL)  {
                         if (t->u.if_stmt.st_list == NULL) 
                             ElseBB = BasicBlock::Create(TheContext, "else", TheFunction); // wants "The Function";
                         else_ast.push(t->u.if_stmt.else_part); }

                     // create "after if" block
                     MergeBB =
                         BasicBlock::Create(TheContext, "ifcont", TheFunction);

                     // check where to go
                     if (t->u.if_stmt.st_list != NULL) 
                         Builder.CreateCondBr(cond, ThenBB, ElifBB);
                     else if (t->u.if_stmt.st_list == NULL and t->u.if_stmt.else_part != NULL) 
                         Builder.CreateCondBr(cond, ThenBB, ElseBB);
                     else 
                         Builder.CreateCondBr(cond, ThenBB, MergeBB);


                     // go to the "then" block
                     Builder.SetInsertPoint(ThenBB);
                     push_block(ThenBB);
                     ast_sem(t->u.if_stmt.block);

                     // go to the "after if" block

                     Builder.CreateBr(MergeBB);
                     pop_block();

                     if (t->u.if_stmt.st_list != NULL)  {

                         Builder.SetInsertPoint(ElifBB);
                         push_block(ElifBB);
                         ast_sem(t->u.if_stmt.st_list);  
                         pop_block();
                         Builder.CreateBr(MergeBB);
                         pop_block(); 
                     }

                     else if (t->u.if_stmt.st_list == NULL and t->u.if_stmt.else_part != NULL)  {

                         Builder.SetInsertPoint(ElseBB);
                         push_block(ElseBB);


                         ast_sem(t->u.if_stmt.else_part);
                         Builder.CreateBr(MergeBB);
                         pop_block();

                     }

                     Builder.SetInsertPoint(MergeBB);
                     push_block(MergeBB);

                     return nullptr ;
                 }
        case ELIF: {
                       BasicBlock* ThenBB;
                       BasicBlock* ElifBB;
                       BasicBlock* MergeBB;
                       BasicBlock* ElseBB;
                       ast k = NULL;
                       Value* cond= ast_sem(t->u.if_stmt.cond);


                       if (t->u.if_stmt.cond->general_type != typeBoolean){
                           if (t->u.if_stmt.cond->general_type == typeInteger)
                               cond = Builder.CreateICmpNE(cond, c32(0), "tmpdif");
                           else
                               cond = Builder.CreateICmpNE(cond, c8(0), "tmpdif");
                       }

                       CHECK_BOOL(t->u.if_stmt.cond->general_type);
                       check_valid_type(t->u.if_stmt.cond);

                       t->u.if_stmt.my_cond = cond;         // den kanei tipota ...
                       Function *TheFunction = get_top()->getParent();

                       // create "then" block
                       ThenBB =
                           BasicBlock::Create(TheContext, "then", TheFunction);

                       // create "after if" block
                       MergeBB =
                           BasicBlock::Create(TheContext, "ifcont", TheFunction);

                       // create "elif" block
                       if (t->u.if_stmt.st_list != NULL) 
                           ElifBB =
                               BasicBlock::Create(TheContext, "elsif", TheFunction); // wants "The Function";

                       else if (!else_ast.empty()) {
                           k = else_ast.top();
                           else_ast.pop();
                           ElseBB = BasicBlock::Create(TheContext, "else", TheFunction); 
                       }


                       if (t->u.if_stmt.st_list != NULL) 
                           Builder.CreateCondBr(cond, ThenBB, ElifBB);
                       else {
                           if (k==NULL) 	
                               Builder.CreateCondBr(cond, ThenBB, MergeBB);
                           else
                               Builder.CreateCondBr(cond, ThenBB, ElseBB);
                       }

                       // Emit then value.
                       Builder.SetInsertPoint(ThenBB);
                       push_block(ThenBB);
                       ast_sem(t->u.if_stmt.block);

                       Builder.CreateBr(MergeBB);
                       pop_block();	            

                       if (t->u.if_stmt.st_list != NULL)  {
                           Builder.SetInsertPoint(ElifBB);
                           push_block(ElifBB);
                           ast_sem(t->u.if_stmt.st_list);  //// ?

                           pop_block();
                           Builder.CreateBr(MergeBB);
                           pop_block();
                       }

                       else if (t->u.if_stmt.st_list == NULL and k != NULL) {

                           Builder.SetInsertPoint(ElseBB);
                           push_block(ElseBB);
                           ast_sem(k);  //// ?


                           Builder.CreateBr(MergeBB);
                           pop_block();


                       }

                       Builder.SetInsertPoint(MergeBB);
                       push_block(MergeBB);
                       return nullptr ;
                   }

        case SEQ: {

                      Value *l = ast_sem(t->u.op.left);

                      Value *result = nullptr;
                      if (var_array){
                          check_valid_type(t->u.op.left);
                          Value* array_dim = c32(array_sizes.top());
                          result = Builder.CreateMul(l, array_dim, "multmp");
                          t->u.op.is_number = 1;
                          array_sizes.pop();
                      }

                      Value *r = ast_sem(t->u.op.right);

                      if (var_array)
                          check_valid_type(t->u.op.right);
                      if (var_array && t->u.op.right != NULL){
                          if (result == nullptr){
                              result = Builder.CreateAdd(l,r,"addtmp");
                          }else{
                              result = Builder.CreateAdd(result,r,"addtmp");
                          }
                      }

                      if (var_array){
                          //result is a pointer only if only one child of type pointer
                          if (t->u.op.right == NULL && !t->u.op.is_number)
                              t->is_array = 1;
                          else
                              t->is_array = 0;
                      }
                      return result;
                  }
        case VAR: {
                      my_countlines = t->line_number;
                      SymbolEntry* e = lookupEntry(t->u.lvalue.id_name, LOOKUP_ALL_SCOPES, true);  
                      if (e->entryType == ENTRY_FUNCTION) {

                          strcat(msg6,(t->u.lvalue.id_name));
                          my_fatal(t->line_number, msg6);
                      }

                      Value* pos;

                      int return_pointer_var = return_pointer;

                      /* ANY Varaiables used to declare this pointer should return a value and not a pointer */
                      return_pointer = 0;

                      std::string s= t->u.lvalue.id_name;
                      BasicBlock* CurBB = get_top();
                      blockp tmp = b_start;
                      while (tmp != NULL && tmp->locals.find(s) == tmp->locals.end() ) {

                          tmp = tmp->prev; }

                      int dimensions;

                      if (e->entryType == ENTRY_PARAMETER){
                          t->general_type = e->u.eParameter.type;
                          t->is_array = (e->u.eParameter.dim == NULL) ? 0 : 1; 
                          dimensions = count_dimensions(e->u.eParameter.dim);
                      }else{
                          t->general_type = e->u.eVariable.type;                    // are you sure it an eVariable and not a eParameter???
                          t->u.lvalue.offset = e->u.eVariable.offset;
                          t->is_array = (e->u.eVariable.dim == NULL) ? 0 : 1; 
                          dimensions = count_dimensions(e->u.eVariable.dim);
                      }
                      t->u.lvalue.nesting_diff = currentScope->nestingLevel - e->nestingLevel;


                      if (t->u.lvalue.expr == NULL){
                          t-> ar = t->is_array;

                          if (e->entryType == ENTRY_PARAMETER && e->u.eParameter.mode==PASS_BY_REFERENCE) { 
                              Value* x;
                              if ((tmp->block)->getParent() == CurBB->getParent()){
                                  Value *temp = new LoadInst(tmp->locals[t->u.lvalue.id_name], "load", false, get_top());
                                  x = GetElementPtrInst::Create(NULL, temp, std::vector<Value *>{c32(0) }, "arrayIdx", CurBB);
                              }
                              else {
                                  LoadInst *tempload = Builder.CreateLoad(globals[t->u.lvalue.id_name].top(), "ld");
                                  x = GetElementPtrInst::Create(NULL, tempload, std::vector<Value *>{c32(0)}, "arrayIdx", CurBB);	
                              }
                              if (return_pointer_var){
                                  return x;
                              }
                              return new LoadInst(x, "load", false, get_top());
                          }

                          if (!t->is_array){

                              if (e->entryType == ENTRY_PARAMETER and ((tmp->block)->getParent() == CurBB->getParent())) {
                                  if (return_pointer_var)
                                      return tmp->locals[t->u.lvalue.id_name];
                                  return new LoadInst(tmp->locals[t->u.lvalue.id_name], "load" , false, CurBB);
                              }

                              if (return_pointer_var)
                                  return globals[t->u.lvalue.id_name].top() ;
                              else  {
                                  return Builder.CreateLoad(globals[t->u.lvalue.id_name].top(),"ld");
                              }

                          }
                          else{
                              Value *temp;
                              if (e->entryType == ENTRY_PARAMETER) { 
                                  if ((tmp->block)->getParent() == CurBB->getParent()){
                                      temp = new LoadInst(tmp->locals[t->u.lvalue.id_name], "load", false, get_top());
                                  }
                                  else {
                                      temp = Builder.CreateLoad(globals[t->u.lvalue.id_name].top(), "ld");
                                  }
                                  return GetElementPtrInst::Create(NULL, temp, std::vector<Value *>{c32(0) }, "arrayIdx", CurBB);
                              }
                              else  {
                                  return  GetElementPtrInst::Create(NULL, globals[t->u.lvalue.id_name].top(), std::vector<Value *>{c32(0), c32(0) }, "arrayIdx", CurBB); }	

                          }
                      }else{
                          var_array++;
                          if (dimensions == 0) {
                              my_countlines = t->line_number;
                              fatal("Variable  %s is not an array", t->u.lvalue.id_name);
                          }

                          if (e->entryType == ENTRY_PARAMETER) {
                              array_add_dimensions(e->u.eParameter.dim);
                          }else
                              array_add_dimensions(e->u.eVariable.dim);
                          int has_dimensions = 0;

                          ast temp = t->u.lvalue.expr;
                          while (temp!=NULL){
                              has_dimensions++;
                              temp = temp->u.op.right;
                          }


                          if (dimensions < has_dimensions) {
                              my_countlines = t->line_number;
                              fatal( "Variable % s has fewer dimensions as specified",t->u.lvalue.id_name);
                          }

                          int real_dim = 0;
                          Dimensions new_temp = e->u.eVariable.dim;
                          while (new_temp!=NULL){
                              real_dim++;
                              new_temp = new_temp->next;
                          }

                          if (has_dimensions == real_dim){
                              t->ar = 0; }
                          else{
                              t->ar = 1; }

                          pos = ast_sem(t->u.lvalue.expr);
                          check_valid_type(t->u.lvalue.expr);
                          var_array--;


                          GetElementPtrInst *x;
                          if (e->entryType == ENTRY_PARAMETER){
                              if  ((tmp->block)->getParent() == CurBB->getParent()) {
                                  Value *temp = new LoadInst(tmp->locals[t->u.lvalue.id_name], "load", false, get_top());
                                  x = GetElementPtrInst::Create(NULL, temp, std::vector<Value *>{ pos }, "arrayIdx", CurBB);
                              }
                              else {
                                  LoadInst *tempload = Builder.CreateLoad(globals[t->u.lvalue.id_name].top(), "ld");
                                  x = GetElementPtrInst::Create(NULL, tempload, std::vector<Value *>{pos}, "arrayIdx", CurBB);	
                              }
                              if (return_pointer_var || dimensions != has_dimensions)
                                  return x;
                              return new LoadInst(x, "load", false, get_top());

                          }

                          x = GetElementPtrInst::Create(NULL, globals[t->u.lvalue.id_name].top(), std::vector<Value *>{c32(0), pos}, "arrayIdx", CurBB);	

                          if (return_pointer_var || dimensions != has_dimensions)
                              return x;                 


                          return new LoadInst(x, "load", false, get_top());

                      }

                  }
        case STR_VAR: {
                          int len = 0;
                          for(unsigned long i = 0; i < strlen(t->u.lvalue.id_name) ; i++){
                              if ((t->u.lvalue.id_name)[i] == '\\')
                                  i++;
                              len++;
                          }

                          t->general_type = typeChar;
                          t->ar = 1;

                          ArrayType* new_type= ArrayType::get(i8, len-1);
                          std::vector<Constant *> val;

                          short next_is_special = 0;
                          unsigned long i =1;
                          while( i< strlen(t->u.lvalue.id_name) - 1){
                              if (next_is_special){
                                  char test = t->u.lvalue.id_name[i];
                                  if (test == 'n')
                                      val.push_back((Constant* )c8('\n'));
                                  else if (test == 't')
                                      val.push_back((Constant* )c8('\t'));
                                  else if (test == 'r')
                                      val.push_back((Constant* )c8('\r'));
                                  else if (test == '0')
                                      val.push_back((Constant* )c8('\0'));
                                  else if (test == '\\')
                                      val.push_back((Constant* )c8('\\'));
                                  else if (test == '\'')
                                      val.push_back((Constant* )c8('\''));
                                  else
                                      val.push_back((Constant* )c8('\"'));
                              }
                              else{
                                  if ((t->u.lvalue.id_name)[i] == '\\'){
                                      next_is_special = 1;
                                      i++;
                                      continue;
                                  }
                                  val.push_back((Constant* )c8((t->u.lvalue.id_name)[i]));
                              }
                              next_is_special = 0;
                              i++;
                          }
                          val.push_back((Constant* ) c8('\0'));

                          new_string = new GlobalVariable( *TheModule, new_type, true, GlobalValue::ExternalLinkage, ConstantArray::get(new_type, val), "str");
                          new_string->setAlignment(1);


                          if (return_pointer)
                              my_fatal(t->line_number, "Calling with a string as reference...");

                          Value *str;     

                          if (t->u.lvalue.expr == NULL){
                              Value *idxlist[] = { c32(0), c32(0) };
                              str = Builder.CreateGEP(new_string, idxlist, "tmp");
                              t->is_array = 1;
                          }else{
                              t->is_array = 0;
                              t->ar = 0;
                              var_array ++;
                              array_sizes.push(0);
                              t->u.lvalue.expr->is_array = 2;
                              check_valid_type(t);                // eixe check_valid_type(t->u.lvalue.expr) ????
                              Value *expr = ast_sem(t->u.lvalue.expr);
                              // CHECK TYPE FOR pos t->u.lvalue.expr ?????????????????????????????????????????????????????????????
                              var_array --;


                              GetElementPtrInst* x = GetElementPtrInst::Create(NULL, new_string, std::vector<Value *>{c32(0), expr }, "arrayIdx", get_top());
                              str =  new LoadInst(x, "load" , false, get_top());                                
                          }

                          return str;

                      }
        case INT: {    
                      t->general_type = typeInteger;
                      return c32(t->u.my_const.val);


                  }
        case CHAR: {
                       t->general_type = typeChar;
                       return c8(t->u.my_const.val);

                   }
        case PLUS: {
                       Value *l = ast_sem(t->u.op.left);
                       Value *r = ast_sem(t->u.op.right);
                       CHECK_SEM("+");
                       CHECK_TYPE;    

                       return Builder.CreateAdd(l, r, "addtmp");  // adds l, r and saves at "addtmp"   

                   }
        case MINUS: {
                        Value *l = ast_sem(t->u.op.left);
                        Value *r = ast_sem(t->u.op.right);
                        CHECK_SEM("-");
                        CHECK_TYPE;
                        return Builder.CreateSub(l, r, "subtmp");
                    }
        case TIMES: {
                        Value *l = ast_sem(t->u.op.left);
                        Value *r = ast_sem(t->u.op.right);
                        CHECK_SEM("*");
                        CHECK_TYPE;
                        return Builder.CreateMul(l, r, "multmp");
                    }
        case DIV: {
                      Value *l = ast_sem(t->u.op.left);
                      Value *r = ast_sem(t->u.op.right);
                      CHECK_SEM("/");
                      CHECK_TYPE;
                      return Builder.CreateSDiv(l, r, "divtmp"); 
                  }
        case MOD: {
                      Value *l = ast_sem(t->u.op.left);
                      Value *r = ast_sem(t->u.op.right);
                      CHECK_SEM("%");
                      CHECK_TYPE;

                      return Builder.CreateSRem(l, r, "modtmp");
                  }
        case EQUAL: {  
                        Value *l = ast_sem(t->u.op.left);
                        Value *r = ast_sem(t->u.op.right);                        
                        CHECK_SEM("=");
                        t->general_type = typeBoolean;

                        Value* xEqualsY = Builder.CreateICmpEQ(l, r, "tmpeq");     //1-bit integer, not so sure
                        return xEqualsY ;

                    }
        case DIFFERENT: {
                            Value *l = ast_sem(t->u.op.left);
                            Value *r = ast_sem(t->u.op.right);
                            CHECK_SEM("<>");
                            t->general_type = typeBoolean;

                            Value* xNotEqualsY = Builder.CreateICmpNE(l, r, "tmpdif");     //1-bit integer, not so sure
                            return xNotEqualsY;
                        }
        case LOWER: {
                        Value *l = ast_sem(t->u.op.left);
                        Value *r = ast_sem(t->u.op.right);                        
                        CHECK_SEM("<");
                        t->general_type = typeBoolean;
                        Value* xLowerThanY;

                        if (equalType(t->u.op.left->general_type, typeInteger)) 
                            xLowerThanY = Builder.CreateICmpSLT(l, r, "tmplo");     //1-bit integer, not so sure
                        else
                            xLowerThanY = Builder.CreateICmpULT(l, r, "tmplo");     //1-bit integer, not so sure
                        return xLowerThanY;
                    } 
        case LOWEREQ: {
                          Value *l = ast_sem(t->u.op.left);
                          Value *r = ast_sem(t->u.op.right);
                          CHECK_SEM("<=");
                          t->general_type = typeBoolean;

                          Value* xLowerEqualY;

                          if (equalType(t->u.op.left->general_type, typeInteger)) 
                              xLowerEqualY = Builder.CreateICmpSLE(l, r, "tmplo");     //1-bit integer, not so sure
                          else
                              xLowerEqualY = Builder.CreateICmpULE(l, r, "tmplo");     //1-bit integer, not so sure
                          return xLowerEqualY;
                      }
        case GREATER: {
                          Value *l = ast_sem(t->u.op.left);
                          Value *r = ast_sem(t->u.op.right); 
                          CHECK_SEM(">");
                          t->general_type = typeBoolean;

                          Value* xGreaterThanY;
                          if (equalType(t->u.op.left->general_type, typeInteger)) 
                              xGreaterThanY = Builder.CreateICmpSGT(l, r, "tmpgr");     //1-bit integer, not so sure
                          else
                              xGreaterThanY = Builder.CreateICmpUGT(l, r, "tmpgr");     //1-bit integer, not so sure

                          return xGreaterThanY;
                      }
        case GREATEREQ: {
                            Value *l = ast_sem(t->u.op.left);
                            Value *r = ast_sem(t->u.op.right);
                            CHECK_SEM(">=");
                            t->general_type = typeBoolean;

                            Value* xGreaterEqualY;
                            if (equalType(t->u.op.left->general_type, typeInteger)) 
                                xGreaterEqualY = Builder.CreateICmpSGE(l, r, "tmpgr");     //1-bit integer, not so sure
                            else
                                xGreaterEqualY = Builder.CreateICmpUGE(l, r, "tmpgr");     //1-bit integer, not so sure

                            return xGreaterEqualY;
                        }
        case AND_C: {
                        Value *l = ast_sem(t->u.op.left);
                        Value *r = ast_sem(t->u.op.right);                       
                        CHECK_BOOL(t->u.op.left->general_type);
                        CHECK_BOOL(t->u.op.right->general_type);
                        t->general_type = typeBoolean;

                        if (t->u.op.left->general_type != typeBoolean){
                            if (t->u.op.left->general_type == typeInteger)
                                l = Builder.CreateICmpNE(l, c32(0), "tmpdif");
                            else
                                l = Builder.CreateICmpNE(l, c8(0), "tmpdif");
                        }
                        if (t->u.op.right->general_type != typeBoolean){
                            if (t->u.op.right->general_type == typeInteger)
                                r = Builder.CreateICmpNE(r, c32(0), "tmpdif");
                            else
                                r = Builder.CreateICmpNE(r, c8(0), "tmpdif");
                        }

                        return Builder.CreateAnd(l, r, "andtmp");
                    }
        case OR_C: {
                       Value *l = ast_sem(t->u.op.left);
                       Value *r = ast_sem(t->u.op.right);
                       CHECK_BOOL(t->u.op.left->general_type);
                       CHECK_BOOL(t->u.op.right->general_type);
                       t->general_type = typeBoolean;

                       if (t->u.op.left->general_type != typeBoolean){
                           if (t->u.op.left->general_type == typeInteger)
                               l = Builder.CreateICmpNE(l, c32(0), "tmpdif");
                           else
                               l = Builder.CreateICmpNE(l, c8(0), "tmpdif");
                       }
                       if (t->u.op.right->general_type != typeBoolean){
                           if (t->u.op.right->general_type == typeInteger)
                               r = Builder.CreateICmpNE(r, c32(0), "tmpdif");
                           else
                               r = Builder.CreateICmpNE(r, c8(0), "tmpdif");
                       }

                       return Builder.CreateOr(l, r, "ortmp");
                   }
        case NOT_C: {
                        Value *l = ast_sem(t->u.op.left);                        
                        CHECK_BOOL(t->u.op.left->general_type);
                        t->general_type = typeBoolean;

                        if (t->u.op.left->general_type != typeBoolean){
                            if (t->u.op.left->general_type == typeInteger)
                                l = Builder.CreateICmpNE(l, c32(0), "tmpdif");
                            else
                                l = Builder.CreateICmpNE(l, c8(0), "tmpdif");
                        }

                        return Builder.CreateNot(l, "nottmp");
                    }
        case AND_E: {
                        Value *l = ast_sem(t->u.op.left);
                        Value *r = ast_sem(t->u.op.right);
                        check_valid_type(t->u.op.left);
                        check_valid_type(t->u.op.right);
                        CHECK_BOOL1(t->u.op.left->general_type, '&');
                        CHECK_BOOL1(t->u.op.right->general_type, '&' );
                        t->general_type = typeChar;

                        return Builder.CreateAnd(l, r, "andtmpexp");
                    }
        case OR_E: {
                       Value *l = ast_sem(t->u.op.left);
                       Value *r = ast_sem(t->u.op.right);
                       check_valid_type(t->u.op.left);
                       check_valid_type(t->u.op.right);
                       CHECK_BOOL1(t->u.op.left->general_type, '|');
                       CHECK_BOOL1(t->u.op.right->general_type, '|');
                       t->general_type = typeChar;

                       return Builder.CreateOr(l, r, "ortmpexp"); 
                   }
        case NOT_E: {
                        Value *l = ast_sem(t->u.op.left);
                        check_valid_type(t->u.op.left);
                        CHECK_BOOL1(t->u.op.left->general_type, '!');

                        if (!equalType(t->u.op.left->general_type, typeChar))
                            my_fatal(t->u.op.left->line_number,"type mismatch in ! operator");
                        t->general_type = typeChar;

                        return Builder.CreateNot(l,"nottmpexp");
                    }
        case SKIP: {	/* h kaluterh entolh */
                       return nullptr;
                   }
        case VARDEF: {
                         ast temp_var = t->u.op.left;
                         ast right = t->u.op.right;
                         my_Type new_T;
                         new_T = right->u.TypeStruct.T;


                         while (temp_var!=NULL){
                             /* Every variable gets the same type and dimensions */
                             int len= strlen(temp_var->u.op.left->u.lvalue.id_name);
                             char *new_name = (char*)malloc(len * sizeof(char));
                             EXIT_NO_MEM(new_name);
                             strcpy(new_name, temp_var->u.op.left->u.lvalue.id_name);
                             my_countlines = temp_var->u.op.left->line_number;
                             newVariable(temp_var->u.op.left->u.lvalue.id_name,new_T, right->u.TypeStruct.dim );
                             temp_var->u.op.left->general_type = new_T;
                             temp_var = temp_var->u.op.right;
                             Type* T;

                             if (equalType(new_T, typeChar)) 
                                 T = Type::getInt8Ty(TheContext);

                             else if (equalType(new_T, typeInteger))
                                 T = Type::getInt32Ty(TheContext);

                             //std::string Cppnew = new_name;
                             if (right->u.TypeStruct.dim == NULL) {

                                 (b_start->names).push_back(new_name);
                                 std::vector<Constant*> val;
                                 if (equalType(new_T, typeChar)) 
                                     globals[new_name].push(new GlobalVariable(*TheModule, T, false, GlobalValue::ExternalLinkage,
                                                 (Constant*)c8(0), new_name));
                                 else 	
                                     globals[new_name].push(new GlobalVariable(*TheModule, T, false, GlobalValue::ExternalLinkage,
                                                 (Constant*)c32(0), new_name));


                             }
                             else {
                                 ArrayType* new_type;
                                 Dimensions d = right->u.TypeStruct.dim;
                                 new_type= ArrayType::get(T,find_total_size(d));

                                 (b_start->names).push_back(new_name);
                                 globals[new_name].push(new GlobalVariable(*TheModule, new_type, false, GlobalValue::ExternalLinkage,
                                             ConstantAggregateZero::get(new_type), new_name));
                             }


                         }
                         return nullptr;
                     }
        case FUNCDEF: {
                          Value * r = ast_sem(t->u.func.header);   

                          BasicBlock* nB = BasicBlock::Create(TheContext, "newentry", (Function *)r, 0 );
                          Builder.SetInsertPoint(nB);

                          push_block(nB);


                          for (unsigned int i = 0; i< func_names.size();  i++) {
                              Type* myT = func_locals[func_names.at(i)];
                              AllocaInst *alloc = new AllocaInst(myT,"", nB);
                              b_start->locals[func_names.at(i)] = alloc; 
                              (b_start->names).push_back(func_names.at(i));

                          }


                          unsigned Idx = 0;
                          for (auto &Arg : ((Function*)r)->args()) {
                              new StoreInst((Value*)&Arg , (b_start->locals)[func_names.at(Idx)] , false, nB); 
                              Builder.CreateStore((Value*)&Arg , globals[func_names.at(Idx)].top()); 
                              Idx++;
                          }

                          ast_sem(t->u.func.localdef);

                          function_decl.push(b_start->names);

                          t->u.func.num_vars = currentScope->negOffset;
                          pushed_blocks = 0;
                          ast_sem(t->u.func.block);


                          if (equalType(t->u.func.header->u.func_header.T, typeChar)){
                              Value *v = c8('\0');
                              ReturnInst::Create(TheContext, v, get_top());
                          }else if (equalType(t->u.func.header->u.func_header.T, typeInteger)){
                              Value *v = c32(0);	
                              ReturnInst::Create(TheContext, v, get_top());
                          }else{
                              ReturnInst::Create(TheContext, get_top());
                          }


                          for (int i=0; i < pushed_blocks; i++)
                              pop_block(); 


                          for (unsigned int i = 0; i < (function_decl.top()).size();  i++) {
                              globals[(function_decl.top()).at(i)].pop();       
                          }


                          pop_block();
                          function_decl.pop();
                          my_remove(&Functions);

                          Builder.SetInsertPoint(get_top());
                          closeScope();	
                          return r;
                      }
        case HEADER: {
                         func_locals.clear();
                         func_names.clear();
                         my_countlines = t->line_number;
                         p = newFunction(t->u.func_header.func_name);

                         openScope();
                         my_insert(t,&Functions);
                         /* forward function ?? */
                         parameter = 1;
                         argTypes.clear();

                         ast_sem(t->u.func_header.func_decl);
                         std::vector<Type*> myargTypes;
                         Function *my;
                         myargTypes = argTypes;
                         argTypes.clear();

                         endFunctionHeader(p, t->u.func_header.T);
                         /*
                         if (!strcmp(t->u.func_header.func_name,"strcat")){    
                             fprintf(stderr,"============= HERE STARTS MAIN ==============\n");
                         } 
                         */

                         // declare a prototype for this function
                         if (!(p->u.eFunction.hasdeclared)) { 
                             Type* T;
                             if (equalType(t->u.func_header.T, typeChar))
                                 T = Type::getInt8Ty(TheContext);
                             else if (equalType(t->u.func_header.T, typeInteger)) 
                                 T = Type::getInt32Ty(TheContext); 

                             else 
                                 T = Type::getVoidTy(TheContext); 


                             FunctionType *ftype = FunctionType::get(T, myargTypes, false);
                             my = Function::Create(ftype, GlobalValue::ExternalLinkage, t->u.func_header.func_name, TheModule.get()); }
                         else my = p->u.eFunction.func;


                         unsigned Idx = 0;
                         for (auto &Arg : my->args())
                             Arg.setName(func_names[Idx++]);

                         if (my_main == NULL) my_main = my; 
                         p->u.eFunction.func = my;

                         if (t->u.func_header.from_libraries && !(p->u.eFunction.hasdeclared))
                             closeScope();
                         return my;
                     }

        case FPARDEF: {

                          ast temp_var = t->u.op.left;
                          ast right = t->u.op.right;
                          my_Type new_T;
                          new_T = right->u.TypeStruct.T;

                          PassMode way;
                          if (right->k == OTHER) 
                              way = PASS_BY_VALUE;
                          else way = PASS_BY_REFERENCE;

                          while (temp_var!=NULL){
                              newParameter(temp_var->u.op.left->u.lvalue.id_name,new_T, right->u.TypeStruct.dim, parameter, way, p );
                              temp_var->u.op.left->general_type = new_T;
                              parameter++;
                              Type* T;

                              if (equalType(new_T, typeChar)) 
                                  T = Type::getInt8Ty(TheContext);
                              else
                                  T = Type::getInt32Ty(TheContext);

                              Type* t1 = T;	

                              if (right->u.TypeStruct.dim == NULL) {                
                                  if (way == PASS_BY_REFERENCE)

                                      T = T->getPointerTo();
                                  argTypes.push_back(T);
                                  func_locals[temp_var->u.op.left->u.lvalue.id_name] = T;
                                  func_names.push_back(temp_var->u.op.left->u.lvalue.id_name);
                                  if (way == PASS_BY_REFERENCE) {
                                      GlobalVariable* n = new GlobalVariable(*TheModule, T, false, GlobalValue::ExternalLinkage, 0, temp_var->u.op.left->u.lvalue.id_name);
                                      globals[temp_var->u.op.left->u.lvalue.id_name].push(n); 

                                      PointerType* PointerTy_0 = PointerType::get(t1,0); 
                                      ConstantPointerNull* const_ptr_2 = ConstantPointerNull::get(PointerTy_0);

                                      // Global Variable Definitions
                                      n->setInitializer(const_ptr_2);
                                  }

                                  else {
                                      if (T == Type::getInt8Ty(TheContext)) 
                                          globals[temp_var->u.op.left->u.lvalue.id_name].push(new GlobalVariable(*TheModule, T, false, GlobalValue::ExternalLinkage, (Constant*)c8(0), temp_var->u.op.left->u.lvalue.id_name));
                                      else 	
                                          globals[temp_var->u.op.left->u.lvalue.id_name].push(new GlobalVariable(*TheModule, T, false, GlobalValue::ExternalLinkage, (Constant*)c32(0), temp_var->u.op.left->u.lvalue.id_name)); 
                                  }
                              }   
                              else {
                                  Type* arr_T = T->getPointerTo();
                                  argTypes.push_back(arr_T);
                                  func_locals[temp_var->u.op.left->u.lvalue.id_name] = arr_T;
                                  func_names.push_back(temp_var->u.op.left->u.lvalue.id_name);

                                  GlobalVariable* n = new GlobalVariable(*TheModule, arr_T, false, GlobalValue::ExternalLinkage, 0, temp_var->u.op.left->u.lvalue.id_name);
                                  globals[temp_var->u.op.left->u.lvalue.id_name].push(n); 

                                  PointerType* PointerTy_0 = PointerType::get(T,0); 
                                  ConstantPointerNull* const_ptr_2 = ConstantPointerNull::get(PointerTy_0);

                                  // Global Variable Definitions
                                  n->setAlignment(4);

                                  n->setInitializer(const_ptr_2);
                              }

                              temp_var = temp_var->u.op.right;                       
                          }

                          return nullptr;
                      }

        case PROCCALL: 
        case FUNCCALL: {
                           my_countlines = t->line_number;
                           SymbolEntry *e = lookupEntry(t->u.lvalue.id_name, LOOKUP_ALL_SCOPES, true);
                           args.clear();
                           int h= check_proccall(e->u.eFunction.firstArgument, t->u.lvalue.expr, t->u.lvalue.id_name);
                           if (h==0) {
                               strcat(msg2,t->u.lvalue.id_name);
                               my_fatal(t->line_number, msg2); }
                           t->general_type = e->u.eFunction.resultType;

                           Function *func = TheModule->getFunction(t->u.lvalue.id_name);
                           std::vector<Value*> myargs;


                           myargs = args;
                           args.clear();

                           return CallInst::Create(func, myargs,"",get_top());


                       }
        case EXPRLIST: {
                           Value* argument = ast_sem(t->u.op.left);
                           args.push_back(argument);
                           ast_sem(t->u.op.right);
                           return nullptr;
                       }

        case OTHER: {
                        /* pinakas ........... */
                        return nullptr;
                    }

        case REF: {
                      return nullptr;
                  }

        case CONTINUE: {
                           ast temp_cont = find_element(t->u.break_continue.loop_id,Loops);
                           if (temp_cont == NULL) {

                               strcat(msg3, t->u.break_continue.loop_id);
                               my_fatal(t->line_number,msg3); }
                           else
                               t->u.break_continue.loop = temp_cont;
                           (t->u.break_continue.loop)->u.loop.again = 1;

                           if (!inside_loops.empty()) {
                               BasicBlock *temp = inside_loops[inside_loops.size() - loop_index - 1];
                               Builder.CreateBr(temp);

                               Function *TheFunction = get_top()->getParent();
                               pop_block();

                               BasicBlock *After =
                                   BasicBlock::Create(TheContext, "after_continue", TheFunction);

                               push_block(After);

                               Builder.SetInsertPoint(After);

                           }
                           return nullptr;
                       }
        case BREAK: {
                        ast temp_brk = find_element(t->u.break_continue.loop_id,Loops);
                        if (temp_brk == NULL) {
                            strcat(msg3, t->u.break_continue.loop_id);
                            my_fatal(t->line_number,msg3); }
                        else
                            t->u.break_continue.loop = temp_brk;
                        (t->u.break_continue.loop)->u.loop.again = 1;

                        if (!after_loops.empty()) {
                            BasicBlock *temp = after_loops[after_loops.size() - loop_index - 1];
                            Builder.CreateBr(temp);

                            Function *TheFunction = get_top()->getParent();
                            pop_block();

                            BasicBlock *After =
                                BasicBlock::Create(TheContext, "after_break", TheFunction);

                            push_block(After);

                            Builder.SetInsertPoint(After);
                        }
                        return nullptr;
                    }
        case EXIT: {

                       ast temp;
                       if ((temp = find_element(NULL,Functions)) == NULL)
                           my_fatal(t->line_number,"No function name corresponding to return statemnet... should be caught by parser");
                       my_countlines = t->line_number;
                       p = lookupEntry(temp->u.func_header.func_name, LOOKUP_ALL_SCOPES, true);
                       if (!equalType(typeVoid, p->u.eFunction.resultType)) {
                           strcat(msg4, temp->u.func_header.func_name);
                           my_fatal(t->line_number, msg4); 
                       }


                       ReturnInst::Create(TheContext, get_top());
                       Function *TheFunction = get_top()->getParent();
                       pop_block();  

                       BasicBlock *After =
                           BasicBlock::Create(TheContext, "after_exit", TheFunction);

                       push_block(After);

                       Builder.SetInsertPoint(get_top());
                       return nullptr;
                   }
        case RETURN: {
                         Value *v = ast_sem(t->u.op.right);
                         ast temp;
                         if ((temp = find_element(NULL,Functions)) == NULL)
                             my_fatal(t->line_number,"No function name corresponding to return statement... should be caught by parser"); 
                         my_countlines = t->line_number; 
                         p = lookupEntry(temp->u.func_header.func_name, LOOKUP_ALL_SCOPES, true);
                         if (!equalType(t->u.op.right->general_type, p->u.eFunction.resultType)) {
                             strcat(msg4, temp->u.func_header.func_name);
                             my_fatal(t->line_number, msg4); 
                         } 
                         if (t->u.op.right->ar)
                             my_fatal(t->line_number, "a function cannot return an array"); 
                         ReturnInst::Create(TheContext, v, get_top());


                         Function *TheFunction = get_top()->getParent();   
                         pop_block();

                         BasicBlock *After =
                             BasicBlock::Create(TheContext, "after_return", TheFunction);

                         push_block(After);

                         Builder.SetInsertPoint(After);
                         return nullptr;
                     }
                     ;

    }

    // Should never reach this point
    return nullptr;
}


void llvm_compile_and_dump (ast t, int optimize) { 

    TheModule = make_unique<Module>("dana program", TheContext);
    // Initialize the module and the optimization passes.
    TheFPM = make_unique<legacy::FunctionPassManager>(TheModule.get());
    if (optimize){
        TheFPM->add(createPromoteMemoryToRegisterPass());
        TheFPM->add(createInstructionCombiningPass());
        TheFPM->add(createReassociatePass());
        TheFPM->add(createGVNPass());
        TheFPM->add(createCFGSimplificationPass());
        TheFPM->doInitialization();
    }

    ast_sem(t);

    bool bad = verifyModule(*TheModule, &errs());   //check the module for errors
    if (bad) {
        TheModule->print(outs(), nullptr);
        return;
    }

    // Print out the IR.
    TheModule->print(outs(), nullptr);  //Print the module to the output stream

    TheFPM->run(*my_main);
    return;
}




