#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"

static void syscall_handler (struct intr_frame *);
static void halt(void);
static void exit(int status);
static int wait(tid_t pid);
static unsigned int write(int fd, const void *buffer, unsigned int count);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f)
{
  if(f->esp == NULL && f->esp > PHYS_BASE)
    exit(-1);

  int syscall_number = *(int*)f->esp;
  printf ("System Call! Number: %d \n", syscall_number);
  switch(syscall_number) {
    case SYS_HALT:
    {
        halt();
        break;
    }
    case SYS_EXIT:
    {
        printf("Calling exit\n");
        int status = *((int*)f->esp + 1);
        exit(status);
        break;
    }
    case SYS_EXEC:
    {
        // Call to exec function
        break;
    }
    case SYS_WAIT:
    {
        printf("Calling Wait\n");
        tid_t pid = (unsigned int)(*((int*)f->esp + 1));
        f->eax = wait(pid);
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
        printf("Calling write\n");
        int fd = *((int*)f->esp + 1);
        void *buffer = (void*)(*((int*)f->esp + 2));
        unsigned int size = *((unsigned int*)f->esp + 3);
        f->eax = write(fd, buffer, size);
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
  thread_exit ();
}

static void
halt(void) {
  shutdown_power_off();
}

static void
exit(int status) {
    struct thread * current_thread = thread_current();
    current_thread->exit_status = status;
    printf ("%s: exit(%d)\n", current_thread->name, status);
    thread_exit();
}

static int
wait(tid_t pid) {
    return process_wait (pid);
}

static unsigned int
write(int fd, const void *buffer, unsigned int count) {
    if(fd == 1) {
         putbuf(buffer, count);
         return count;
     }
     return count;
}
