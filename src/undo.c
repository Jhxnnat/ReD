#include <string.h>
#include <stdio.h>
#include "undo.h"

void init_ustack(Ustack *ustack, int init_cap) {
	ustack->size = 0;
	ustack->capacity = 8;
	
	ustack->undo = malloc(init_cap * sizeof(Undo));
	if (ustack->undo == NULL) {
		printf("[init_ustack]: buy more memory\n");
		exit(1);
	}
}

void free_ustack(Ustack *ustack) {
	//TODO: free the Undo list
	free(ustack->undo);
}

void ustack_push(Ustack *ustack, int *codepoint_buff, size_t codepoint_buff_size) {
	if (ustack->size >= ustack->capacity) {
		ustack->size = ustack->capacity;
		ustack->capacity *= 2;
		Undo *temp = realloc(ustack->undo, ustack->capacity * sizeof(Undo));
		if (temp == NULL) {
			printf("[ustack_push]: error realloc");
			exit(1);
		} else {
			ustack->undo = temp;
		}
	}

	Undo *undo = &ustack->undo[ustack->size];
	undo->buff = malloc(codepoint_buff_size * sizeof(int)); //TODO: should check if has been  prev allocated
															//so we use realloc instead
	if (undo->buff == NULL) {
		printf("[ustack_push]: buy more memory\n");
		exit(1);
	}
	undo->size = codepoint_buff_size;
	for (size_t i = 0; i < codepoint_buff_size; ++i) {
		undo->buff[i] = codepoint_buff[i];
	}
	ustack->size++;
}

bool ustack_undo(Ustack *ustack) {
	if (ustack->size < 1) return false;
	ustack->size--;
	//TODO: perform move the info to Redo Stack
	return true;
}

