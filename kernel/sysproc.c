#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// get uid of specified process.
uint64
sys_getuid(void)
{
  int pid;

  argint(0, &pid);
  return getuid(pid);
}

// set uid of specified process, as long as
// the password offered is correct.
uint64
sys_setuid(void)
{
  int pid, uid;
  char buf[MAXPWD], tar[MAXPWD];

  argint(0, &pid);
  argint(1, &uid);
  if(argstr(2, buf, MAXPWD) < 0)
    return -1;

  strncpy(tar, PWD, MAXPWD);
  if(strncmp(buf, tar, MAXPWD) != 0)
    return -1;

  if(setuid(pid, uid) < 0)
    return -1;
  return 0;
}

// get env value associated with given key
// in the current process.
uint64
sys_getenv(void)
{
  int pid;
  char key[MAXENVK], *val;
  uint64 dst;

  argint(0, &pid);
  if(argstr(1, key, MAXENVK) < 0)
    return -1;
  argaddr(2, &dst);

  if((val = getenv(pid, key)) == 0)
    return 0;

  if(copyout(myproc()->pagetable, dst, val, MAXENVV) < 0)
    return -1;
  return 0;
}

// set env value associated with given key
// in the current process.
uint64
sys_setenv(void)
{
  int pid;
  char key[MAXENVK], val[MAXENVV];

  argint(0, &pid);

  if(argstr(1, key, MAXENVK) < 0)
    return -1;

  if(argstr(2, val, MAXENVV) < 0)
    return -1;

  if(setenv(pid, key, val) < 0)
    return -1;
  return 0;
}

// list environment variables.
uint64
sys_env(void)
{
  envdump();
  return 0;
}
