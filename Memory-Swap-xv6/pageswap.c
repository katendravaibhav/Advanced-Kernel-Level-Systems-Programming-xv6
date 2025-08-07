#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

#define SWAP_SLOT_TOTAL 800
#define SWAP_BLOCKS_PER_PAGE 8
#define SWAP_BLOCK_START 2

struct run {
  struct run *next;
};

struct swap_slot {
  int page_perm;
  int is_free;
};

struct {
  struct spinlock lock;
  struct swap_slot slots[SWAP_SLOT_TOTAL];
} swap_area;
#ifdef BETA
int beta = BETA;
#else
int beta = 10;
#endif
int threshold = 100;
int limit = 100;
int npages_to_swap = 4;
#ifdef ALPHA
int alpha = ALPHA;
#else
int alpha = 25;
#endif



void
swapInit(void)
{
  initlock(&swap_area.lock, "swap_area");
  acquire(&swap_area.lock);
  int idx = 0;
  do {
    swap_area.slots[idx].is_free = 1;
    swap_area.slots[idx].page_perm = 0;
    idx++;
  } while(idx < SWAP_SLOT_TOTAL);
  release(&swap_area.lock);
  cprintf("Swap area initialized with %d slots\n", SWAP_SLOT_TOTAL);
}

int
findFreeSlot(void)
{
  int result = -1;
  acquire(&swap_area.lock);
  for(int i = 0; i < SWAP_SLOT_TOTAL; i++) {
    if(swap_area.slots[i].is_free) {
      swap_area.slots[i].is_free = 0;
      result = i;
      break;
    }
  }
  release(&swap_area.lock);
  return result;
}

void
freeSlot(int slot_index)
{
  if(slot_index >= 0 && slot_index < SWAP_SLOT_TOTAL) {
    acquire(&swap_area.lock);
    swap_area.slots[slot_index].is_free = 1;
    swap_area.slots[slot_index].page_perm = 0;
    release(&swap_area.lock);
  }
}

int
duplicateSwapSlot(int parent_slot)
{
  if(parent_slot < 0 || parent_slot >= SWAP_SLOT_TOTAL)
    return -1;
  if(swap_area.slots[parent_slot].is_free)
    return -1;

  int child_slot = findFreeSlot();
  if(child_slot < 0) {
    checkAndSwap();
    child_slot = findFreeSlot();
    if(child_slot < 0) {
      checkAndSwap();
      child_slot = findFreeSlot();
      if(child_slot < 0)
        return -1;
    }
  }

  acquire(&swap_area.lock);
  int perm = swap_area.slots[parent_slot].page_perm;
  swap_area.slots[child_slot].page_perm = perm;
  release(&swap_area.lock);

  uint parent_blk = SWAP_BLOCK_START + parent_slot * SWAP_BLOCKS_PER_PAGE;
  uint child_blk = SWAP_BLOCK_START + child_slot * SWAP_BLOCKS_PER_PAGE;

  int b = 0;
  while(b < SWAP_BLOCKS_PER_PAGE) {
    struct buf *src = bread(0, parent_blk + b);
    struct buf *dst = bread(0, child_blk + b);
    memmove(dst->data, src->data, BSIZE);
    bwrite(dst);
    brelse(src);
    brelse(dst);
    b++;
  }
  return child_slot;
}

int
countFreePages(void)
{
  int total = 0;
  struct run *r;
  extern struct {
    struct spinlock lock;
    int use_lock;
    struct run *freelist;
  } kmem;
  if(kmem.use_lock) acquire(&kmem.lock);
  r = kmem.freelist;
  while(r != 0) {
    total++;
    r = r->next;
  }
  if(kmem.use_lock) release(&kmem.lock);
  return total;
}

int
swapAndPageOut(pde_t *pgdir, uint va, uint pa)
{
  int slot_index = findFreeSlot();
  if(slot_index < 0)
    return -1;

  uint blockno = SWAP_BLOCK_START + slot_index * SWAP_BLOCKS_PER_PAGE;
  pte_t *pte = walkpgdir(pgdir, (void*)va, 0);
  if(!pte)
    return -1;
  if(!(*pte & PTE_P))
    return -1;

  acquire(&swap_area.lock);
  swap_area.slots[slot_index].page_perm = *pte & 0xFFF;
  release(&swap_area.lock);

  for(int i = 0; i < SWAP_BLOCKS_PER_PAGE; i++) {
    struct buf *b = bread(0, blockno + i);
    char *src = (char*)(P2V(pa)) + i*BSIZE;
    memmove(b->data, src, BSIZE);
    bwrite(b);
    brelse(b);
  }
  *pte = (slot_index << 12) | ((*pte) & ~PTE_P & 0xFFF);
  lcr3(V2P(pgdir));
  return 0;
}

int
swapPageIn(pde_t *pgdir, void *va)
{
  uint page_addr = PGROUNDDOWN((uint)va);
  pte_t *pte = walkpgdir(pgdir, (void*)page_addr, 0);
  if(!pte)
    return -1;
  if(*pte & PTE_P)
    return 0;

  int slot_index = PTE_ADDR(*pte) >> 12;
  if(slot_index < 0 || slot_index >= SWAP_SLOT_TOTAL)
    return -1;
  if(swap_area.slots[slot_index].is_free)
    return -1;

  char *mem = kalloc();
  if(mem == 0) {
    checkAndSwap();
    mem = kalloc();
    if(mem == 0)
      return -1;
  }

  uint blockno = SWAP_BLOCK_START + slot_index * SWAP_BLOCKS_PER_PAGE;
  int i = 0;
  while(i < SWAP_BLOCKS_PER_PAGE) {
    struct buf *b = bread(0, blockno + i);
    memmove(mem + i*BSIZE, b->data, BSIZE);
    brelse(b);
    i++;
  }

  uint perm;
  acquire(&swap_area.lock);
  perm = swap_area.slots[slot_index].page_perm;
  release(&swap_area.lock);

  perm |= PTE_P;
  int mapping = mapPages(pgdir, (void*)page_addr, PGSIZE, V2P(mem), perm);
  if(mapping < 0) {
    kfree(mem);
    return -1;
  }
  freeSlot(slot_index);

  struct proc *p = myproc();
  if(p)
    p->rss++;

  return 0;
}

struct proc*
findVictimProc(void)
{
  struct proc *victim = 0;
  int maxrss = 0;
  extern struct {
    struct spinlock lock;
    struct proc proc[NPROC];
  } ptable;
  acquire(&ptable.lock);
  struct proc *p = ptable.proc;
  while(p < &ptable.proc[NPROC]) {
    if(p->state != UNUSED && p->pid >= 1) {
      if(p->rss > maxrss || (p->rss == maxrss && victim && p->pid < victim->pid)) {
        maxrss = p->rss;
        victim = p;
      }
    }
    p++;
  }
  release(&ptable.lock);
  return victim;
}

uint
findVictimPage(pde_t *pgdir, uint *va_out)
{
  uint addr = 0;
  while(addr < KERNBASE) {
    pte_t *pte = walkpgdir(pgdir, (void*)addr, 0);
    if(pte && (*pte & PTE_P) && (*pte & PTE_U) && !(*pte & PTE_A)) {
      *va_out = addr;
      return PTE_ADDR(*pte);
    }
    addr += PGSIZE;
  }
  addr = 0;
  while(addr < KERNBASE) {
    pte_t *pte = walkpgdir(pgdir, (void*)addr, 0);
    if(pte && (*pte & PTE_P) && (*pte & PTE_U))
      *pte &= ~PTE_A;
    addr += PGSIZE;
  }
  lcr3(V2P(pgdir));
  addr = 0;
  while(addr < KERNBASE) {
    pte_t *pte = walkpgdir(pgdir, (void*)addr, 0);
    if(pte && (*pte & PTE_P) && (*pte & PTE_U)) {
      *va_out = addr;
      return PTE_ADDR(*pte);
    }
    addr += PGSIZE;
  }
  return 0;
}

void
swapOutPages(void)
{
  struct proc *victim = findVictimProc();
  if(victim == 0)
    return;
  int swapped = 0;
  int attempts = 0;
  while(swapped < npages_to_swap && attempts < npages_to_swap * 2) {
    uint va;
    uint pa = findVictimPage(victim->pgdir, &va);
    if(pa == 0)
      break;
    int out = swapAndPageOut(victim->pgdir, va, pa);
    if(out == 0) {
      victim->rss--;
      kfree((char*)P2V(pa));
      swapped++;
    }
    attempts++;
  }
}

void
checkAndSwap(void)
{
  int free_pages = countFreePages();
  if(free_pages > threshold)
    return;

  cprintf("Current Threshold = %d, Swapping %d pages\n", threshold, npages_to_swap);

  swapOutPages();

  threshold -= (threshold * beta) / 100;
  if(threshold < 1)
    threshold = 1;

  npages_to_swap += (npages_to_swap * alpha) / 100;
  if(npages_to_swap > limit)
    npages_to_swap = limit;
}

void
swapAndClean(struct proc *p)
{
  if(p == 0 || p->pgdir == 0)
    return;
  uint i = 0;
  while(i < KERNBASE) {
    pte_t *pte = walkpgdir(p->pgdir, (void*)i, 0);
    if(pte && !(*pte & PTE_P) && (*pte != 0)) {
      int slot_index = PTE_ADDR(*pte) >> 12;
      if(slot_index >= 0 && slot_index < SWAP_SLOT_TOTAL)
        freeSlot(slot_index);
    }
    i += PGSIZE;
  }
}
