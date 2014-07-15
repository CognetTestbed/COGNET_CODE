/*
 *   National Semiconductor SC1x00 CPU watchdog driver
 *   Copyright (c) Inprimis Technologies 2002
 *
 *   by Mark Grosberg <markg@inprimis.com>
 *   and Rolando Goldman <rolandog@inprimis.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version
 *   2 of the License, or (at your option) any later version.
 *   
 *   09/10/2003
 *   - Module parameter support   
 *   - Modern processor support
 *   added by Erich Titl erich.titl@think.ch
 *   
 *   09/01/2005
 *   - port to 2.6 kernel
 *   added by Punky Tse punkytse@yahoo.com
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/spinlock.h>
#include <linux/sysctl.h>
#include <linux/pci.h>

/* 
 * Since the SC1100 is an x86 clone, we don't even bother with 
 * allowing other architectures to compile us.
 */
#ifndef CONFIG_X86
# error Sorry this driver is only for x86.
#endif

//#include <asm/system.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/processor.h>

/*
#define DEBUG_WD1100 
*/

static int proc_wd_timeout(ctl_table   *ctl, 
                           int          write,
                           void        *buffer,
                           size_t      *lenp,
                           loff_t      *ppos);
static int proc_wd_graceful(ctl_table   *ctl, 
                            int          write,
                            void        *buffer,
                            size_t      *lenp,
                            loff_t      *ppos);
                           
/* Register definitions */

#define SC1100_F5_VENDOR_ID  0x100B
#define SC1100_F5_DEVICE_ID  0x0515   

#define CPU_WDTO_REG    0x00 /* watchdog time out, 16 bit register */
#define CPU_WDCNFG_REG  0x02 /* watchdog config , 16 bit register */
#define CPU_WDSTS_REG   0x04 /* watchdog status , 8 bit register */

/* Default timeout: 4 seconds (changeable via sysctl) */
static unsigned int    sysctl_wd_timeout  = 4;
static unsigned int    sysctl_wd_graceful = 1;

static int             in_use    = 0;
static unsigned short  cpu_base;
static struct semaphore wd_semaphore;

/**************************************************************************/

#define GCB_LOCATION 0x1234 // just a dummy value

int gcb = GCB_LOCATION;
module_param(gcb, int, 0);
MODULE_PARM_DESC (gcb, "The GCB location (default is to look into the scratch pad)");

module_param(sysctl_wd_graceful, int, 0);
MODULE_PARM_DESC (sysctl_wd_graceful, "Graceful shutdown on close");

module_param(sysctl_wd_timeout, int, 0);
MODULE_PARM_DESC (sysctl_wd_timeout, "Watchdog timeout value in seconds");

/**************************************************************************/

/* XXX To-do: DEV_WATCHDOG must be in include/linux/sysctl.h */
enum
{ DEV_WATCHDOG = 8 };

enum
{
   DEV_WD_TIMEOUT   = 1,
   DEV_WD_GRACEFUL  = 2
};

static struct ctl_table_header *wd_table_header;

static ctl_table wd_table[] = {
   {
    .procname = "timeout",
    .data = &sysctl_wd_timeout, .maxlen = sizeof(int), .mode = 0644, .proc_handler = &proc_wd_timeout,
   },
   
   {
    .procname = "graceful",
    .data = &sysctl_wd_graceful, .maxlen = sizeof(int), .mode = 0644, .proc_handler = &proc_wd_graceful,
   },

   {0}
};

static ctl_table wd_dir_table[] = {
   {.procname = "watchdog", .maxlen = 0, .mode = 0555, .child = wd_table,},
   {0}
};

static ctl_table wd_root_table[] = {
   {.procname = "dev", .maxlen = 0, .mode = 0555, .child = wd_dir_table,},
   {0}
};

static int proc_wd_timeout(ctl_table   *ctl, 
                           int          write,
                           void        *buffer,
                           size_t      *lenp,
                           loff_t      *ppos)
{
  int   rc;
  
  rc = proc_dointvec(ctl, write, buffer, lenp, ppos);
  if (write && (rc == 0))
  {
     /* Clamp to limits. */
     if (sysctl_wd_timeout < 1) 
       sysctl_wd_timeout = 1;
     else if (sysctl_wd_timeout > 65535)
       sysctl_wd_timeout = 65535;
  }
  
  return (rc);
}

static int proc_wd_graceful(ctl_table   *ctl, 
                            int          write,
                            void        *buffer,
                            size_t      *lenp,
                            loff_t      *ppos)
{
  int   rc;
  
  rc = proc_dointvec(ctl, write, buffer, lenp, ppos);
  if (write && (rc == 0))
  {
     /* Clamp to true/false. */
     if (sysctl_wd_graceful) 
       sysctl_wd_graceful = 1;
  }
  
  return (rc);
}                           

/**************************************************************************/

static __inline__ void reset_wd(void)
{
  outw(sysctl_wd_timeout * 8, cpu_base + CPU_WDTO_REG);
}

static int reboot_reason(void)
{
   unsigned static char sr;
   static int    fetched = 0;
 
   if (!fetched)
   {
     
     sr = inb(cpu_base + CPU_WDSTS_REG);
     outb(sr | 1, cpu_base + CPU_WDSTS_REG);
   
     fetched = 1;
   }
   
   return ((sr & 0x01) ? 1 : 0);
}

static struct watchdog_info wd_info =
{
   0,   /* Options          */
   0,   /* Firmware version */
   "NSC SC1x00 WD"
};

static int wd_ioctl(struct inode *inode, 
                    struct file  *file, 
                    unsigned int  cmd,
                    unsigned long arg)
{
   int  i;
   
   switch (cmd)
   {
      default:
        return (-ENOTTY);
        
      case WDIOC_GETSUPPORT:
        i = access_ok(VERIFY_WRITE, (void *)arg, sizeof(struct watchdog_info));
        if (i)
          return (i);
        else
          return copy_to_user((struct watchdog_info *)arg, 
                              &wd_info, 
                              sizeof(wd_info));
        break;
        
      case WDIOC_KEEPALIVE:
        reset_wd();
        return (0);
        
      case WDIOC_GETBOOTSTATUS:
        i = reboot_reason();
        return (put_user(i, (int *)arg));
        
      case WDIOC_GETSTATUS:
        i = inw(cpu_base + CPU_WDTO_REG) / 8;
        return (put_user(i, (int *)arg));
   }       
}                    

static int wd_open(struct inode *inode, 
                   struct file  *file)
{

  int i;

  if (down_trylock(&wd_semaphore))
	return -EBUSY;

  /*
   * Configure the chip to do a reset if the timer goes to 0.
   * Set the clock divisor to 4096.
   */
     
  i = inw(cpu_base + CPU_WDCNFG_REG);
  // keep the high order 7 bits of the config register
  // clear bit 8 and write 0xfc to the low order 8 bits
  outw((i & 0xfe00) | 0x00fc, cpu_base + CPU_WDCNFG_REG);
  
  /* Start the watchdog: It won't run until we write the TO reg. */
  reset_wd();

  return nonseekable_open(inode, file);
}

static int wd_release(struct inode *inode,
                       struct file  *file)
{
  in_use = 0;
  
  /*
   * If graceful shutdown is not set, then don't bother to stop the
   * watchdog timer. This handles the scenario where the user process
   * that is poking the watchdog gets terminated due to some error
   * (say a SEGV or some VM condition). 
   *
   * In that case, the kernel would happily close the descriptor for
   * us and leave us in a state where we aren't watching the dog...
   *
   * To work around this, the "graceful" sysctl prevents reset of the
   * watchdog on close.
   */
#ifdef DEBUG_WD1100
  printk(KERN_INFO "wd1100.c: releasing wd1100 sysctl_wd_graceful = %x\n",sysctl_wd_graceful);
#endif
  if (sysctl_wd_graceful)
    outw(0, cpu_base + CPU_WDCNFG_REG);
  
  up(&wd_semaphore);
  
  return (0);
}           

static ssize_t wd_write(struct file *file,
                        const char  *data,
                        size_t       len,
                        loff_t      *ppos)
{
  if (len > 0)
    reset_wd();   
  
  return (len);
}

static struct file_operations wd_fops=
{
   owner:      THIS_MODULE,
   write:      wd_write,
   unlocked_ioctl:      wd_ioctl,
   open:       wd_open,
   release:    wd_release,
};
 
static struct miscdevice sc1x00wd_miscdev=
{
   WATCHDOG_MINOR,
   "watchdog",
   &wd_fops
};

static int __init wd_init(void)
{
   int              ret;
   struct pci_dev  *dev;
   unsigned int     cw;

   if (strcmp(boot_cpu_data.x86_vendor_id, "Geode by NSC") != 0)
   {
      printk(KERN_ERR "wd1100.c: This is not an SC1100 processor!\n");
      return (0);
   }

   switch (boot_cpu_data.x86_model)
   {
	case 4: // those are the models we know
	case 9:
		break;
	default:
		printk(KERN_WARNING "wd1100.c: unknown model value %d, trying anyway\n",boot_cpu_data.x86_model);
		break;
   }

   if (gcb == GCB_LOCATION) // this is the assigned dummy value 
   {
#ifdef DEBUG_WD1100
   	printk(KERN_INFO "wd1100.c: fetching the value for gcb from scratch pad\n");
#endif
   	/* get the CONFIG BLOCK ADDRESS from scratch pad register */ 
   	dev = pci_get_device(SC1100_F5_VENDOR_ID,SC1100_F5_DEVICE_ID,0);
   	if (dev == NULL)
   	{
     		printk(KERN_ERR "wd1100.c: Can not find bridge device.\n");
     		return (0);
   	}

   	// read a double word at offset 0x64 into cw
   	pci_read_config_dword(dev, 0x64, &cw);
   	cpu_base = (unsigned short )cw;
   }
   else // it is not the default value, just use it
   {
#ifdef DEBUG_WD1100
   	printk(KERN_INFO "wd1100.c: using the assigned value for gcb \n");
#endif
   	cpu_base = (unsigned short) gcb;
   }
  
#ifdef DEBUG_WD1100
   printk(KERN_INFO "wd1100.c: CPU base = 0x%X\n", (unsigned int )cpu_base);
#endif

   printk(KERN_INFO "SC1x00 Watchdog driver by Inprimis Technolgies.\n");
   printk(KERN_INFO "wd1100.c: a few hacks by erich.titl@think.ch\n");
   /*
    * We must call reboot_reason() to reset the flag in the WD.
    *
    * Even though it is available as an ioctl(), we call it during
    * module initialization to perform the clear. You can take out
    * the printk(), but don't take out the call to reboot_reason().
    */
   if (reboot_reason())
      printk(KERN_INFO "Last reboot was by watchdog!\n");
    
   sema_init(&wd_semaphore, 1);
        
   ret = misc_register(&sc1x00wd_miscdev);
   if (ret)
     printk(KERN_ERR "wd1100.c: Can't register device.\n");
   else
   {
     wd_table_header = register_sysctl_table(wd_root_table); 
      if (wd_table_header == NULL)
        printk(KERN_ERR "wd1100.c: Can't register sysctl.\n");
   }

   return 0;
}   

static void __exit wd_exit(void)
{
   if (wd_table_header != NULL)
      unregister_sysctl_table(wd_table_header);
   
   misc_deregister(&sc1x00wd_miscdev);
}

module_init(wd_init);
module_exit(wd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Grosberg <markg@inprimis.com> and Rolando Goldman <rolandog@inprimis.com>");
MODULE_DESCRIPTION("National Semiconductor SC1x00 CPU watchdog driver");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
