#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/fixed-point.h"
#include "threads/synch.h"

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    int nice;                           /* Nice value of the thread*/
    fp_t recent_cpu;                    /* Recent CPU value for the thread*/
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

    struct semaphore t_sema;            /* semaphore for putting thread to sleep and wake them up */
    struct list_elem t_elem;            /* list element for waiting queue */
    int64_t wakeup_time;                /* 64bit integer - record the wakeup time */
    int total_fd_id;
    struct list file_list;

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
    //int exit_status;

#endif

/*priority data*/

  //The current priority of a thread considering priorities donated by other threads
  int practical_priority;
  //Lock that this thread is waiting on, It's null if the list is empty.
  struct lock *resource_waiting;
  // List of locks held by the thread and waited by others.
  struct list locks_held;

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
    // Wait/Exec sys calls
    struct list child_list;
    tid_t parent;
    // Struct to store info about the child process
    struct child *c;
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

//functions to ensure priority scheduling, by sorting ready list.
void update_ready_list (void);
//The below function compares practical priority, hence not used for mlfqs
bool compare_list_element_priority (const struct list_elem *first_entry, const struct list_elem *second_entry, void *aux);
bool compare_list_element_priority_mlfqs (const struct list_elem *first_entry, const struct list_elem *second_entry, void *aux);
int highest_priority_in_list (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);

//functions to compute practical priority based on donated priority
int thread_get_practical_priority (struct thread *t);
void thread_set_practical_priority (struct thread *current_thread, int max_priority);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

void calculate_thread_priority(struct thread *t, void *aux);
void calculate_priority_for_each_thread(void);
void calculate_load_avg(void);
void increment_recent_cpu(void);
void calculate_recent_cpu(struct thread *t, void *aux);
void calculate_recent_cpu_for_each_thread(void);
void yield_max_priority_thread(void);

bool thread_alive(int pid);
#endif /* threads/thread.h */
