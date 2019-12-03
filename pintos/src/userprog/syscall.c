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

#define MAX_ARGS 3
#define USER_VADDR_BOTTOM ((void *) 0x08048000)
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);
void halt(void);
void exit(int status);
void * check_addr(const void*);
int wait(tid_t pid);
int write(int fd, const void *buffer, unsigned int count);
void get_arg (struct intr_frame *f, int *arg, int n);
void check_valid_buffer (void* buffer, unsigned size);
int user_to_kernel_ptr(const void *vaddr);
void check_valid_ptr (const void *vaddr);

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
  check_valid_ptr((const void*)f->esp);
  int syscall_number = *(int*)f->esp;
  //printf ("System Call! Number: %d \n", syscall_number);
  switch(syscall_number) {
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
        get_arg(f, &arg[0], 2);
        f->eax = wait(arg[0]);
        break;
    }
    case SYS_CREATE:
    {
        // Call to create function
        break;
    }
    case SYS_REMOVE:
    {
        // Call to remove function
        break;
    }
    case SYS_OPEN:
    {
        // Call to open function
        break;
    }
    case SYS_FILESIZE:
    {
        // Call to filesize function
        break;
    }
    case SYS_READ:
    {
        // Call to read function
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
        break;
    }
    case SYS_TELL:
    {
        // Call to tell function
        break;
    }
    case SYS_CLOSE:
    {
        // Call to close function
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
    //current_thread->exit_status = status;
    if(thread_alive(current_thread->parent))
      current_thread->c->status = status;
    printf ("%s: exit(%d)\n", current_thread->name, status);
    thread_exit();
}

pid_t
exec(const char *cmd_line)
{
  pid_t pid = process_execute(cmd_line);
  struct child *cp = get_child_process(pid);
  ASSERT(cp);
  while(cp->load == NOT_LOADED)
   barrier();
  if(cp->load == LOAD_FAIL)
   return ERROR;
  
  return pid;
}

int
wait(tid_t pid) {
    return process_wait (pid);
}

int
write(int fd, const void *buffer, unsigned int count) {
    if(fd == 1) {
         putbuf(buffer, count);
         return count;
     }
     return count;
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
  // TO DO: Need to check if all bytes within range are correct
  // for strings + buffers
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
