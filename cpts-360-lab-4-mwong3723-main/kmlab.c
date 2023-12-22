#define LINUX

#include <linux/module.h>
#include <linux/kernel.h>
#include "kmlab_given.h"
// Include headers as needed ...
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wong"); // Change with your lastname
MODULE_DESCRIPTION("CPTS360 Lab 4");

#define DEBUG 1
#define PROCSDIR "kmlab"
#define PROCFILE "status"

#define PROCFSMAXSIZE 1024

// Global variables as needed ...
static struct proc_dir_entry *procDir, *procFile;
LIST_HEAD(processList);
static struct timer_list procTimer;
static DEFINE_SPINLOCK(spinLock);
struct process_entry {
   pid_t pid;
   long unsigned cpuTime;
   struct list_head list;
};

static ssize_t procfile_read(struct file *file, char __user *buffer, size_t count, loff_t *offset) {
   struct process_entry *position;
   char *tempStr;
   unsigned long flags;
   size_t listCount = 0, size = 0, total = 0;
   if (*offset) {
      return 0;
   }
   spin_lock_irqsave(&spinLock, flags); // count our items that are in our list to give memeory
   list_for_each_entry(position, &processList, list) {
      listCount++;
   }
   spin_unlock_irqrestore(&spinLock, flags);

   size = listCount * 32;
   tempStr = kmalloc(size, GFP_KERNEL);
   
   spin_lock_irqsave(&spinLock, flags);
   list_for_each_entry(position, &processList, list) {
      total += scnprintf(tempStr + total, size - total, // go over our list to add our PID and CPU times so we can count total bytes for later
      "PID%d: %lu\n", position->pid, position->cpuTime); 
      if (total>= size) {
         break;
      } 
    }
   spin_unlock_irqrestore(&spinLock, flags);

   if(total < count){ // cannot copy anymore data
      total = total;
   }
   else{ // can copy more
      total = count;
   }
   if (copy_to_user(buffer, tempStr, total)) { // verify coipy
      return -EFAULT;
   }
   *offset += total;
   return total;
}


static ssize_t procfile_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset) {
   int status, pid;
   char tempStr[8] = "";
   struct process_entry *pMem;
   unsigned long flags;
   if (copy_from_user(tempStr, buffer, count)) { // gcopy new pid 
      return -EFAULT;
   }
   if ((status = kstrtoint(tempStr, 10, &pid))) { // convert from string to int for our pid
      return status;
   }
   spin_lock_irqsave(&spinLock, flags);
   pMem = (struct process_entry *)kzalloc(sizeof(struct process_entry), GFP_KERNEL);
   pMem->pid = pid;
   INIT_LIST_HEAD(&pMem->list);
   list_add_tail(&pMem->list, &processList);
   spin_unlock_irqrestore(&spinLock, flags);
   return count;
}

//https://stackoverflow.com/questions/61295277/passing-argument-4-of-proc-create-from-incompatible-pointer-type
// had to  proc_ops over file_operations aparaently 
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};


static void work_function(struct work_struct *curWork) {
    unsigned long flags;
    struct process_entry *position, *temp;

    spin_lock_irqsave(&spinLock, flags);
    list_for_each_entry_safe(position, temp, &processList, list) {
      if (get_cpu_use(position->pid, &position->cpuTime)) {// go over our CPU time for our ruinning proccocess then delete the finsihed
         list_del(&position->list);
         kfree(position);
      }
    }
    spin_unlock_irqrestore(&spinLock, flags);
}


DECLARE_WORK(workQueue, work_function); 


void timer_callback(struct timer_list *timeList) {
    if (!list_empty(&processList)) { // if queue exitsts
        schedule_work(&workQueue);
    }
    mod_timer(&procTimer, jiffies + msecs_to_jiffies(5000)); // create our periodic timer
}

// kmlab_init - Called when module is loaded
int __init kmlab_init(void)
{
   #ifdef DEBUG
   pr_info("KMLAB MODULE LOADING\n");
   #endif
   // Insert your code here ...
   INIT_LIST_HEAD(&processList);
   timer_setup(&procTimer, timer_callback, 0);
   mod_timer(&procTimer, jiffies + msecs_to_jiffies(5000));  // start timer ready set go!!!
   procDir = proc_mkdir(PROCSDIR, NULL); // create our directory
   if (procDir == NULL) {
      pr_alert("Error: unable to initialize /proc/%s\n", PROCSDIR);
      return -ENOMEM;
   }
   procFile = proc_create(PROCFILE, 0666, procDir, &proc_file_fops); // create the stauts of our directory when we start
   if (procFile == NULL) {
      pr_alert("Error: unable to create /proc/%s/%s\n", PROCSDIR, PROCFILE);
      proc_remove(procDir);
      return -ENOMEM;
   }


   pr_info("KMLAB MODULE LOADED\n");
   return 0;   
}

// kmlab_exit - Called when module is unloaded
void __exit kmlab_exit(void)
{
   #ifdef DEBUG
   pr_info("KMLAB MODULE UNLOADING\n");
   #endif
   // Insert your code here ...
   // We want to free all our resources used
   struct process_entry *position, *temp;
   list_for_each_entry_safe(position, temp, &processList, list) { // free our linked list we made
      list_del(&position->list);
      kfree(position);
   }
   del_timer(&procTimer); // free our timer and entries and queue
   proc_remove(procDir);
   proc_remove(procFile);
   flush_scheduled_work();

   pr_info("KMLAB MODULE UNLOADED\n");
}

// Register init and exit funtions
module_init(kmlab_init);
module_exit(kmlab_exit);