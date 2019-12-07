#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "filesys/filesys.h"
#define MAX_ARGS 3
#define USER_VADDR_BOTTOM ((void *) 0x08048000)
#include "devices/shutdown.h"

void file_lock_acquire();
void file_lock_release();
static void syscall_handler (struct intr_frame *);
void halt(void);
void exit(int status);
void * check_addr(const void*);
int wait(tid_t pid);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned int count);
void seek (int fd, unsigned position);
void close(int fd);
unsigned tell (int fd);
struct file_doc* retrieve_file(int fd);
void get_arg (struct intr_frame *f, int *arg, int n);
void check_valid_buffer (void* buffer, unsigned size);
int user_to_kernel_ptr(const void *vaddr);
void check_valid_ptr (const void *vaddr);
void check_valid_string(const void*str);

//static struct lock file_lock;

void 
file_lock_acquire(){
	lock_acquire(&file_lock);
}

void
file_lock_release(){
	lock_release(&file_lock);
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f)
{
  //if(f->esp == NULL && f->esp > PHYS_BASE)
    //exit(-1);
  int arg[MAX_ARGS];
  //check_valid_ptr((const void*)f->esp);
  int syscall_number = user_to_kernel_ptr((const void *)f->esp);
  lock_init(&file_lock);
  //printf ("System Call! Number: %d \n", syscall_number);
  switch(*(int *)syscall_number) {
    case SYS_HALT:
    {
        halt();
        break;
    }
    case SYS_EXIT:
    {
        //printf("Calling exit\n");
        //check_addr((int*)f->esp + 1);
        //int status = *((int*)f->esp + 1);
        get_arg(f, &arg[0], 1);
        exit(arg[0]);
        break;
    }
    case SYS_EXEC:
    {
        get_arg(f, &arg[0], 1);
        check_valid_string((const void*) arg[0]);
        arg[0] = user_to_kernel_ptr((const void*) arg[0]);
        f->eax = exec((const char*)arg[0]);
        break;
    }
    case SYS_WAIT:
    {
        //printf("Calling Wait\n");
        //check_addr((int*)f->esp + 1);
        //tid_t pid = (unsigned int)(*((int*)f->esp + 1));
        //f->eax = wait(pid);
        get_arg(f, &arg[0], 1);
        f->eax = wait(arg[0]);
        break;
    }
    case SYS_CREATE:
    {
    	get_arg(f, &arg[0],2);
    	arg[0] = user_to_kernel_ptr((const void*) arg[0]);
    	f->eax = create((const char*)arg[0], (unsigned)arg[1]);

        // Call to create function
        break;
    }
    case SYS_REMOVE:
    {
        // Call to remove function
        get_arg(f, &arg[0],1);
        arg[0] = user_to_kernel_ptr((const void*) arg[0]);
        f->eax = remove((const char*)arg[0]);
        break;
    }
    case SYS_OPEN:
    {
        // Call to open function
        get_arg(f, &arg[0],1);
        arg[0] = user_to_kernel_ptr((const void*) arg[0]);
        f->eax = open((const char*)arg[0]);
        break;
    }
    case SYS_FILESIZE:
    {
        // Call to filesize function
        get_arg(f, &arg[0],1);
        int * p = f->esp;
        // arg[0] = user_to_kernel_ptr((const void*) (p+1));
        f->eax = filesize((int) arg[0]);
        break;
    }
    case SYS_READ:
    {
        // Call to read function
        get_arg(f, &arg[0],3);
        check_valid_buffer((void*) arg[1], (unsigned int) arg[2]);
        arg[1] = user_to_kernel_ptr((const void*)arg[1]);
        f->eax = read((int) arg[0], (void*) arg[1], (unsigned) arg[2]);
        break;
    }
    case SYS_WRITE:
    {
        //printf("Calling write\n");
        /*check_addr((int*)f->esp + 1);
        int fd = *((int*)f->esp + 1);
        check_addr((int*)f->esp + 2);
        void *buffer = (void*)(*((int*)f->esp + 2));
        check_addr((int*)f->esp + 3);
        unsigned int size = *((unsigned int*)f->esp + 3);
        f->eax = write(fd, buffer, size); */
        get_arg(f, &arg[0], 3);
        check_valid_buffer((void*) arg[1], (unsigned int) arg[2]);
        arg[1] = user_to_kernel_ptr((const void*)arg[1]);
        f->eax = write(arg[0], (const void*) arg[1], (unsigned int)arg[2]);
         break;
    }
    case SYS_SEEK:
    {
        // Call to seek function
        get_arg(f, &arg[0], 2);
        seek((int) arg[0], (unsigned) arg[1]);
        break;
    }
    case SYS_TELL:
    {
        // Call to tell function
        get_arg(f, &arg[0], 1);
        f->eax = tell((int) arg[0]);
        break;
    }
    case SYS_CLOSE:
    {
        // Call to close function
        get_arg(f, &arg[0], 1);
        close((int) arg[0]);
        break;
    }
  }
}

void
halt(void) {
  shutdown_power_off();
}

void
exit(int status) {
    struct thread * current_thread = thread_current();
    if(thread_alive(current_thread->parent) && current_thread->c)
      current_thread->c->status = status;
    printf ("%s: exit(%d)\n", current_thread->name, status);
    thread_exit();
}

pid_t
exec(const char *cmd_line)
{
  pid_t pid = process_execute(cmd_line);
  struct child *cp = get_child_process(pid);
  if(cp == NULL) return ERROR;

  while(cp->load == NOT_LOADED)
    sema_down(&cp->load_sema);

  if(cp->load == LOAD_FAIL)
  {
    remove_child_process(cp);
    return ERROR;
  }
  return pid;
}

int
wait(tid_t pid) {
    return process_wait (pid);
}

bool 
create(const char *file, unsigned initial_size){
	file_lock_acquire();
	bool is_create = filesys_create(file, initial_size);
	file_lock_release();
	return is_create;
}

int
write(int fd, const void *buffer, unsigned int count) {
	if(fd == 1) {
         putbuf(buffer, count);
         return count;
    }
    struct file_doc* fd_doc = retrieve_file(fd);
	if (fd_doc == NULL){
	 	return -1;
	}

	file_lock_acquire();
	int write_len = file_write(fd_doc->p, buffer, count);
	file_lock_release();
    return write_len;
}

bool
remove(const char *file){
	file_lock_acquire();
	bool is_remove = filesys_remove(file);
	file_lock_release();
	return is_remove;
}

int
open (const char *file){

	file_lock_acquire();
	struct file *return_file = filesys_open(file);
	file_lock_release();
	if (return_file == NULL)
		return -1;

	struct file_doc *fd_doc = malloc(sizeof(struct file_doc));
	fd_doc->p = return_file;

	int id = ++thread_current()->total_fd_id;
	fd_doc->id = id;
	list_insert(list_tail(&thread_current()->file_list), &fd_doc->e);
	// list_push_front(&thread_current()->file_list, &fd_doc->e);
	return id;
}
int filesize (int fd){
	struct file_doc* fd_doc = retrieve_file(fd);
	if (fd_doc == NULL){
		return -1;
	}
	file_lock_acquire();
	int size = file_length(fd_doc->p);
	file_lock_release();
	return size;
}

int
read (int fd, void *buffer, unsigned size){
	if (fd == 0){
		return input_getc();
	}
	struct file_doc* fd_doc = retrieve_file(fd);
	if (fd_doc == NULL){
		return -1;
	}
	file_lock_acquire();
	int read_len = file_read(fd_doc->p, buffer, size);
	file_lock_release();
	return read_len;

}
void 
seek(int fd, unsigned position){
	struct file_doc* fd_doc = retrieve_file(fd);
	if (fd_doc == NULL){
		return -1;
	}
	file_lock_acquire();
	file_seek(fd_doc->p, position);
	file_lock_release();
	return;
}
unsigned 
tell (int fd) {
	struct file_doc* fd_doc = retrieve_file(fd);
	if (fd_doc == NULL){
		return -1;
	}
	file_lock_acquire();
	unsigned index = file_tell(fd_doc->p);
	file_lock_release();
	return index;
}
void close (int fd){
	struct file_doc* fd_doc = retrieve_file(fd);
	if (fd_doc == NULL){
		return -1;
	}
	file_lock_acquire();
	file_close(fd_doc->p);
	list_remove(&fd_doc->e);
	free(fd_doc);
	file_lock_release();
	return;
}

struct file_doc* retrieve_file(int fd){
	struct list_elem *e;
  	for (e = list_begin(&thread_current()->file_list); e != list_end(&thread_current()->file_list); e = list_next(e)) {
  		struct file_doc *cur = list_entry(e, struct file_doc, e);
  		if (cur->id == fd) {
  			return cur;
  		} 
  	}
  	return NULL;
}


void check_valid_ptr (const void *vaddr)
{
  if (!is_user_vaddr(vaddr) || vaddr < USER_VADDR_BOTTOM)
    {
      exit(ERROR);
    }
}

int user_to_kernel_ptr(const void *vaddr)
{
  check_valid_ptr(vaddr);
  void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
  if (!ptr)
    {
      exit(ERROR);
    }
  return (int) ptr;
}

void * check_addr(const void *vaddr)
{
    if(!is_user_vaddr(vaddr)) {
        exit(-1);
        return 0;
     }

     void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
     if(!ptr) {
        exit(-1);
        return 0;
     }
     return ptr;
}
/*
struct child* add_child_process(int pid)
{
  struct child *c = (struct child*)malloc(sizeof(struct child));
  c->pid = pid;
  c->load = NOT_LOADED;
  c->wait = false;
  c->exit = false;
  lock_init(&c->wait_lock);
  list_push_back(&thread_current()->child_list, &c->elem);

  return c;
}
*/
struct child* get_child_process(int pid)
{

  struct thread *t = thread_current();
  struct list_elem *e;

  for(e = list_begin(&t->child_list); e!= list_end(&t->child_list); e = list_next(e))
  {
     struct child *cp = list_entry(e, struct child, elem);
     if(pid == cp->pid) return cp;
  }
  return NULL;
}

void remove_child_process(struct child *cp)
{
  list_remove(&cp->elem);
  free(cp);
}

void remove_child_processes(void)
{
  struct thread*t = thread_current();
  struct list_elem *next, *e = list_begin(&t->child_list);

  while(e!= list_end(&t->child_list))
  {

     next = list_next(e);
     struct child *cp = list_entry(e, struct child, elem);
     list_remove(&cp->elem);
     free(cp);
     e = next;

  }
}

void get_arg (struct intr_frame *f, int *arg, int n)
{
  int i;
  int *ptr;
  for (i = 0; i < n; i++)
    {
      ptr = (int *) f->esp + i + 1;
      check_valid_ptr((const void *) ptr);
      arg[i] = *ptr;
    }
}

void check_valid_buffer (void* buffer, unsigned size)
{
  unsigned i;
  char* local_buffer = (char *) buffer;
  for (i = 0; i < size; i++)
    {
      check_valid_ptr((const void*) local_buffer);
      local_buffer++;
    }
}

void check_valid_string(const void*str)
{
  while(*(char*) user_to_kernel_ptr(str) != 0)
    str = (char*) str +1;
}
