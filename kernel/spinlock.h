#define NSEM 20

// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};

struct semaphore{
	struct spinlock lock;
	int count;
  int valid;
  uint64 address;

};
extern struct semaphore sem[NSEM];
extern struct spinlock tblock;

extern char * abc[NSEM];

