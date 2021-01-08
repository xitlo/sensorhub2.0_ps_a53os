/** ===================================================== **
 *Author : Momenta founderHAN
 *Created: 2021-1-5
 *Version: 1.0
 ** ===================================================== **/

/** ===================================================== **
 * HEADER
 ** ===================================================== **/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/timekeeping.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <asm/io.h>

/** ===================================================== **
 * MACRO
 ** ===================================================== **/
/* version */
#define VERSION           "v1.3-21"

/* 设备节点名称 */
#define DEVICE_NAME       "timesync"
/* 设备号个数 */
#define DEVID_COUNT       1
/* 驱动个数 */
#define DRIVE_COUNT       1
/* 主设备号 */
#define MAJOR_AX
/* 次设备号 */
#define MINOR_AX          20

/** ===================================================== **
 * STRUCT DEFINE
 ** ===================================================== **/
struct time_s {
    unsigned int uiTimeNsec;
    unsigned int uiTimeSecL;
};

struct ps_timer_s {
    unsigned int uiTimeNsec;
    unsigned int uiTimeSecL;
    unsigned int uiTimeSecH;
    unsigned int uiIrqCycle;
};

struct sync_data_s {
    struct timespec64  begin;           //起始系统时间
    struct timespec64  end;             //结束系统时间
    struct timespec64  realtime;        //真实时间-读取自ps-timer寄存器
    unsigned long      realtime_reg_v;  //真实时间-ps-timer寄存器读值
    int                diff_real_b_ns;  //真实时间与起始系统时间差值，用于精度判断
    int                handle_e_b_ns;   //结束系统时间与起始系统时间差值，用于操作时间计算
}

struct timesync_char_dev{
    struct device_node *nd;             //设备树的设备节点
    unsigned int       time_reg_addr;   //time寄存器地址
    unsigned int       time_period_ms;  //定时器周期，单位ms
    struct ps_timer_s  *ps_timer_ptr;
    struct work_struct sync_work;
    struct timer_list  timer;           //定时器
    spinlock_t         lock;            //自旋锁变量

    struct sync_data_s set;
    struct sync_data_s read;
};

/** ===================================================== **
 * VARIABLE
 ** ===================================================== **/
static struct timesync_char_dev timesync_dev = {0};

/** ===================================================== **
 * MODULE PARAM
 ** ===================================================== **/
int iFlagDebuglog = 0;
module_param(iFlagDebuglog, int, 0644);
MODULE_PARM_DESC(iFlagDebuglog, "timesync debug log");

static ssize_t show_debug_log(struct device *dev, struct device_attribute *attr, char *buf) {
    return snprintf(buf, PAGE_SIZE, "debug log: %d\n", iFlagDebuglog);
}

static ssize_t store_debug_log(struct device *dev,struct device_attribute *attr,const char *buf, size_t count) {
    if(kstrtoint(buf, 10, &iFlagDebuglog) < 0)
        return -EINVAL;
    printk(">>>> debug log: %d\n", iFlagDebuglog);
    return count;
}

static DEVICE_ATTR(timesync_debug_log, S_IRUGO | S_IWUSR, show_debug_log, store_debug_log);

/** ===================================================== **
 * FUNCTIONS
 ** ===================================================== **/
static void synctime_process(struct work_struct *work)
{
    struct timesync_char_dev *dev = container_of(work, struct timesync_char_dev, sync_work);
    struct timespec64 ts1, ts2, ts_set;
    unsigned long ulCurPsTime;
    struct time_s *time_ptr = (struct time_s *)&ulCurPsTime;
    int ret;

	spin_lock_irq(&dev->lock);
    ktime_get_real_ts64(&ts1);
    // ulCurPsTime = *(unsigned long *)dev->ps_timer_ptr;
    // ts_set.tv_nsec = ulCurPsTime & 0x00000000FFFFFFFF;
    // ts_set.tv_sec  = ulCurPsTime >> 32;
    ulCurPsTime = atomic64_read((atomic64_t *)dev->ps_timer_ptr);
    ts_set.tv_nsec = time_ptr->uiTimeNsec;
    ts_set.tv_sec  = time_ptr->uiTimeSecL;
    ret = do_settimeofday64(&ts_set);
    ktime_get_real_ts64(&ts2);
	spin_unlock_irq(&dev->lock);

    if ( 0 < iFlagDebuglog ) {
        printk("&&timer expire[%d]! sys1/ps/diff, sys2/hdt: %lld.%09ld/%lld.%09ld/%lld, %lld.%09ld/%lld\n",
            ret,
            ts1.tv_sec, ts1.tv_nsec,
            ts_set.tv_sec, ts_set.tv_nsec,
            (ts_set.tv_sec - ts1.tv_sec)*1000000000 + ts_set.tv_nsec - ts1.tv_nsec,
            ts2.tv_sec, ts2.tv_nsec,
            (ts2.tv_sec - ts1.tv_sec)*1000000000 + ts2.tv_nsec - ts1.tv_nsec);
    }
}

void timer_function(struct timer_list *timer)
{
    if( 0 < timesync_dev.time_period_ms)
        mod_timer(&timesync_dev.timer, jiffies + msecs_to_jiffies(timesync_dev.time_period_ms));

    schedule_work(&timesync_dev.sync_work);
}

static int timesync_open(struct inode *inode_p, struct file *file_p)
{
    file_p->private_data = &timesync_dev;
    return 0;
}

static ssize_t timesync_write(struct file *file_p, const char __user *buf, size_t len, loff_t *loff_t_p)
{
    int retvalue;
    unsigned int databuf;
    struct timesync_char_dev *dev = file_p->private_data;

    retvalue = copy_from_user((void *)&databuf, buf, len);
    if(retvalue < 0) {
        printk("timesync copy_from_user failed\n");
        return -EFAULT;
    }

    if( sizeof(unsigned int) != len ) {
        printk("length err, %ld/%ld\n", len, sizeof(unsigned int));
        return -EFAULT;
    }

    dev->time_period_ms = databuf;
    printk("timesync period: %d\n", dev->time_period_ms);

    if( 0 == dev->time_period_ms)
        del_timer_sync(&dev->timer);
    else {
        mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->time_period_ms));
        schedule_work(&dev->sync_work);
    }

    return 0;
}

static ssize_t timesync_read(struct file *file_p, char __user *buf, size_t len, loff_t *loff_t_p)
{
    return 0;
}

static int timesync_release(struct inode *inode_p, struct file *file_p)
{
    return 0;
}

/* file_operations结构体声明, 是上面open、write实现函数与系统调用函数对应的关键 */
static struct file_operations timesync_fops = {
    .owner   = THIS_MODULE,
    .open    = timesync_open,
    .read    = timesync_read,
    .write   = timesync_write,
    .release = timesync_release,
};

/* MISC设备结构体 */
static struct miscdevice timesync_miscdev = {
    .minor = MINOR_AX,
    .name = DEVICE_NAME,
    /* file_operations结构体 */
    .fops = &timesync_fops,
};

/* probe函数实现, 驱动和设备匹配时会被调用 */
static int timesync_probe(struct platform_device *dev)
{
    int ret = 0;

    printk("%s, %s\n", __func__, VERSION);

    /* 获取设备节点 */
    timesync_dev.nd = of_find_node_by_path("/timesync");
    if(timesync_dev.nd == NULL) {
        printk("timesync node nost find\n");
        return -EINVAL;
    }

    if(0 > of_property_read_u32(timesync_dev.nd, "timereg", &timesync_dev.time_reg_addr)) {
        printk("can not get timereg\n");
        return -EINVAL;
    }

    timesync_dev.ps_timer_ptr = (struct ps_timer_s *)ioremap_wc(timesync_dev.time_reg_addr, sizeof(struct ps_timer_s));
    printk("timereg/remap: 0x%08x/0x%lx\n", timesync_dev.time_reg_addr, (unsigned long)timesync_dev.ps_timer_ptr);

    /* 注册misc设备 */
    ret = misc_register(&timesync_miscdev);
    if(ret < 0) {
        printk("misc device register failed\n");
        return -EFAULT;
    }

    INIT_WORK(&timesync_dev.sync_work, synctime_process);

    /* 设置定时器回掉函数&初始化定时器 */
    timer_setup(&timesync_dev.timer, timer_function, 0);

    if ( 0 != device_create_file(&dev->dev, &dev_attr_timesync_debug_log) ) {
		printk(KERN_DEBUG "Failed to create device file debug log \n");
	}else{
		printk(KERN_DEBUG "Device file debug log created successfully\n");
	}

    return 0;
}

static int timesync_remove(struct platform_device *dev)
{
    device_remove_file(&dev->dev, &dev_attr_timesync_debug_log);

    /* 删除定时器 */
    del_timer_sync(&timesync_dev.timer);

    iounmap(timesync_dev.ps_timer_ptr);

    /* 注销misc设备 */
    misc_deregister(&timesync_miscdev);

    printk("timesync_remove ok\n");
    return 0;
}

/* 初始化of_match_table */
static const struct of_device_id timesync_of_match[] = {
    /* compatible字段和设备树中保持一致 */
    { .compatible = "momenta-timesync" },
    {/* Sentinel */}
};

/* 声明并初始化platform驱动 */
static struct platform_driver timesync_driver = {
    .driver = {
        /* name字段需要保留 */
        .name = "timesync",
        /* 用of_match_table代替name匹配 */
        .of_match_table = timesync_of_match,
    },
    .probe  = timesync_probe,
    .remove = timesync_remove,
};

/* 驱动入口函数 */
static int __init timesync_drv_init(void)
{
    /* 在入口函数中调用platform_driver_register, 注册platform驱动 */
    return platform_driver_register(&timesync_driver);
}

/* 驱动出口函数 */
static void __exit timesync_drv_exit(void)
{
    /* 在出口函数中调用platform_driver_register, 卸载platform驱动 */
    platform_driver_unregister(&timesync_driver);
}

/* 标记加载、卸载函数 */
module_init(timesync_drv_init);
module_exit(timesync_drv_exit);

/* 驱动描述信息 */
MODULE_AUTHOR("Momneta");
MODULE_ALIAS("timesync");
MODULE_DESCRIPTION("MISC timesync driver");
MODULE_VERSION("v1.0");
MODULE_LICENSE("GPL");
