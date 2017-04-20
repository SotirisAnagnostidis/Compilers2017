#ifndef __LEXER_H__
#define __LEXER_H__

	typedef struct list{
		struct list *next;
		char *name;
	} List;

	typedef struct node {
		struct node* next;
		int beggining;
		char has_begin;
		int statements;
		char pseudo_begin;
		char is_def;
		List* definitions;
	} Node;

	typedef Node* stack;

	void add_to_list(char *name);
	void push(int spaces);
#endif
