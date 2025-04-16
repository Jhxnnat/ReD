#ifndef UNDO_H
#define UNDO_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
	size_t size;
	int *buff;

	/*size_t start;*/
	/*size_t end; */
	size_t cursor;
	size_t cursor_column;
	size_t cursor_line;
} Undo;

typedef struct {
	size_t size;
	size_t capacity;
	Undo *undo;
} Ustack;

void init_ustack(Ustack *ustack, int init_cap);
void free_ustack(Ustack *ustack);
void ustack_push(Ustack *ustack, int *codepoint_buff, size_t codepoint_buff_size);
bool ustack_undo(Ustack *ustack);

#endif // !UNDO_H
