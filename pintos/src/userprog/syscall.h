#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/synch.h"

#define CLOSE_ALL -1
#define ERROR -1

#define NOT_LOADED 0
#define LOAD_SUCCESS 1
#define LOAD_FAIL 2

struct child {
int pid;
int load;
bool wait;
bool exit;
int status;
struct lock wait_lock;
struct list_elem elem;
};

struct child* add_child_process(int pid);

struct child* get_child_process(int pid);

void remove_child_process(struct child *cp);

void remove_child_processes(void);


void syscall_init (void);

#endif /* userprog/syscall.h */
