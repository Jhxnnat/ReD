#ifndef UNDO_H
#define UNDO_H

typedef struct {
	size_t begin;
	size_t end;
	const char *buffer;
	size_t cursor_pos;
} Undo;

//TODO: implement undo/redo behaviour

#endif UNDO_H //UNDO_H
