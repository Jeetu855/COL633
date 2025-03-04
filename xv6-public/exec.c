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
    cprintf("Operation execute failed: permission denied\n");
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

 
  // if(strcmp(file, "chmod") == 0){
  //   cprintf(2, "Operation chmod failed: cannot modify chmod itself.\n");
  //   return -1;
  // }


  // if((ip = namei(file)) == 0){
  //   cprintf("\nOperation chmod failed: file %s not found.\n", file);
  //   return -1;
  // }


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

// int sys_chmod(void)
// {
//   char *path;
//   int mode;
//   struct inode *ip;

//   if (argstr(0, &path) < 0 || argint(1, &mode) < 0)
//     return -1;
//   if (mode < 0 || mode > 7)
//     return -1;

//   begin_op();
//   if ((ip = namei(path)) == 0) {
//     end_op();
//     return -1;
//   }
//   ilock(ip);
//   cprintf("Value is %d\n", mode);
//   ip->minor = mode;  // store permission bits in minor
//   iupdate(ip);
//   iunlockput(ip);
//   end_op();
//   return 0;
// }
