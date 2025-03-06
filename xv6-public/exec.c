#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"

// history----------------
#include "history_struct.h"
int gethistory(char*);
#include "syscall.h"

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
      s1++;
      s2++;
  }
  return *(unsigned char *)s1 - *(unsigned char *)s2;
}
// -----------------------------

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;
  struct proc *curproc = myproc();

  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    cprintf("exec: fail\n");
    return -1;
  }
  ilock(ip);

  // asignment 1---------------------------------------
  if (!(ip->mode & 4)) {
    iunlockput(ip);
    end_op();
    cprintf("Operation execute failed\n");
    return -1;
  }
  // -----------------------------

  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
  sp = sz;

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));

  // Commit to the user image.
  oldpgdir = curproc->pgdir;
  curproc->pgdir = pgdir;
  curproc->sz = sz;
  // -------------------
  curproc->check = 1;
  // --------------------
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;
  switchuvm(curproc);
  freevm(oldpgdir);
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}

int
chmod_actual(char *file, uint mode)
{
  struct inode *ip;

 
  if(strcmp(file, "chmod") == 0){
    cprintf(2, "Operation chmod failed: cannot modify chmod itself.\n");
    return -1;
  }


  if(mode < 0 || mode > 7){
    cprintf("\nOperation chmod failed: invalid minor %d. Allowed range is 0-7.\n", mode);
    
    return -1;
  }
  begin_op();
  if ((ip = namei(file)) == 0) {
    end_op();
    return -1;
  }
  ilock(ip);
  cprintf("Value is %d\n", mode);
  ip->mode = mode;  // store permission bits in minor
  iupdate(ip);
  iunlockput(ip);
  end_op();
  return 0;
}


// history -----------

int
gethistory_actual(char *user_buffer)
{
  int count;
  int bytes;

  acquire(&hist_lock);
  count = hist_count;  
  bytes = count * sizeof(struct history_struct);

  
  if(copyout(myproc()->pgdir, (uint)user_buffer, (char *)hist_arr, bytes) < 0) {
       release(&hist_lock);
       return -1;
  }
  release(&hist_lock);
  return count;
}
// --------------------


// block unblock -----------------
int block_actual(int syscall_id) {
  if (syscall_id == SYS_fork || syscall_id == SYS_exit)
      return -1; 

  struct proc *curproc = myproc();
  if(syscall_id < 0 || syscall_id >= NELEM(curproc->blocked_syscalls))
      return -1;

  curproc->blocked_syscalls[syscall_id] = 1;
  return 0;
}


int 
unblock_actual(int syscall_id)
{
  struct proc *curproc = myproc();

  // Validate syscall ID range  
  if(syscall_id < 0 || syscall_id >= NELEM(curproc->blocked_syscalls))
    return -1;

  curproc->blocked_syscalls[syscall_id] = 0;
  return 0;
}


// -------------------------------