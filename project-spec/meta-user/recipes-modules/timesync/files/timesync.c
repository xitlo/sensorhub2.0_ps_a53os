/** ===================================================== **
 *Author : Momenta founderHAN
 *Created: 2021-1-5
 *Version: 1.0
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
#include <linux/security.h>
#include <linux/timekeeping.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <asm/io.h>

/* version */
#define VERSION           "v1.3-12"

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

#define PS_RTC_ADDR       0xFFA60000

struct ps_rtc_s {
	unsigned int uiSetTimeWrite;
	unsigned int uiSetTimeRead;
	unsigned int uiCalibWrite;
	unsigned int uiCalibRead;
    unsigned int uiCurrentTime;
	unsigned int auiReserved[16];
};

struct ps_timer_s {
	unsigned int uiTimeSecL;
	unsigned int uiTimeSecH;
	unsigned int uiIrqCycle;
	unsigned int uiTimeNsec;
};

/* 把驱动代码中会用到的数据打包进设备结构体 */
struct timesync_char_dev{
    struct device_node *nd;             //设备树的设备节点
    unsigned int       time_reg_addr;   //time寄存器地址
    unsigned int       time_period_ms;  //定时器周期，单位ms
    unsigned long      remap_addr_timer;
    unsigned long      remap_addr_rtc;
    struct ps_timer_s  *ps_timer_ptr;
    struct ps_rtc_s    *ps_rtc_ptr;
    struct timer_list  timer;           //定时器
};

/* 声明设备结构体 */
static struct timesync_char_dev timesync_char = {0};

#if 0
static long synctime_process(struct xdma_dev *lro)
{
    struct timespec ts1, ts2;
    unsigned int time_s, time_ns;
    unsigned long long ullTimeDiff;
    struct timesync_regs *reg = (struct timesync_regs*)
            (lro->bar[lro->user_bar_idx] + USER_TIMESYNC);

    /* need assure pcie and pc are same in second, and not exceed threshold */
    do {
        getnstimeofday(&ts1);

        /*read pcie time*/
        time_s  = read_register(&reg->time_s);
        time_ns = read_register(&reg->time_ns);
        if ( read_register(&reg->time_s) != time_s ) {
            time_ns = read_register(&reg->time_ns);
        }

        /* read system time */
        getnstimeofday(&ts2);

        /* calc elapse time of read time */
        ullTimeDiff = (unsigned long long)(ts2.tv_sec-ts1.tv_sec)*1000000000 + ts2.tv_nsec - ts1.tv_nsec;
    } while ( (time_s != ts2.tv_sec) || (ts2.tv_nsec > XDMA_SYNC_TIME_NS_MAX) || (time_ns + ullTimeDiff > XDMA_SYNC_TIME_NS_MAX) );

    /* set sync registers */
    write_register(ts2.tv_sec, &reg->time_s);
    write_register(ts2.tv_nsec | (1<<31), &reg->time_ns);

    if ( iFlagSyncTimeLog ) {
        struct timespec ts3;
        getnstimeofday(&ts3);
        printk("&&&&%s: sync d/pcie/pc/diff: %llu/%u.%09u/%lu.%09lu/%lldns\n", __func__, ullTimeDiff,
             time_s, time_ns, ts2.tv_sec, ts2.tv_nsec, (long long)(ts2.tv_sec-time_s)*1000000000 + ts2.tv_nsec - time_ns);
        printk("&&&&%s: run: %lu.%09lu->%lu.%09lu=%lldns\n", __func__,
             ts1.tv_sec, ts1.tv_nsec, ts3.tv_sec, ts3.tv_nsec, (long long)(ts3.tv_sec-ts1.tv_sec)*1000000000 + ts3.tv_nsec - ts1.tv_nsec);
    }

    /* Clear time sync status */
    write_register(0UL, &reg->time_ns);

    return 0;
}
#endif

void timer_function(struct timer_list *timer)
{
    // struct timespec ts1, ts2, ts_set;
    struct timespec64 ts1, ts2, ts_set;

    // unsigned int time_s, time_ns;
    // unsigned long long ullTimeDiff;

    if( 0 < timesync_char.time_period_ms)
        mod_timer(&timesync_char.timer, jiffies + msecs_to_jiffies(timesync_char.time_period_ms));

    // synctime_process(lro);
    // if ( iFlagSyncTimeLog ) {
    // }

    ktime_get_real_ts64(&ts1);
    ts_set.tv_nsec = timesync_char.ps_timer_ptr->uiTimeNsec*1000;
    ts_set.tv_sec  = timesync_char.ps_timer_ptr->uiTimeSecL;
    if ( 0 != security_settime64(&ts_set, NULL) ) {
        printk("security_settime64 err!!\n");
    }
    do_settimeofday64(&ts_set);
    ktime_get_real_ts64(&ts2);
    //atomic64_read();

    // printk("&&timer expire! sys1/ps/diff_ns, sys2/diff: %d.%09d/%d.%09d/%d, %d.%09d/%d, %d\n",
    //        ts1.tv_sec, ts1.tv_nsec,
    //        ts_ps.uiTimeSecL, ts_ps.uiTimeNsec*1000,
    //        (ts_ps.uiTimeSecL - ts1.tv_sec)*1000000000 + ts_ps.uiTimeNsec*1000 - ts1.tv_nsec,
    //        ts2.tv_sec, ts2.tv_nsec,
    //        (ts2.tv_sec - ts1.tv_sec)*1000000000 + ts2.tv_nsec - ts1.tv_nsec,
    //        ts_rtc.uiCurrentTime);
    printk("&&timer expire! sys1/ps/diff_ns, sys2/diff: %lld.%09ld/%lld.%09ld/%lld, %lld.%09ld/%lld\n",
           ts1.tv_sec, ts1.tv_nsec,
           ts_set.tv_sec, ts_set.tv_nsec,
           (ts_set.tv_sec - ts1.tv_sec)*1000000000 + ts_set.tv_nsec - ts1.tv_nsec,
           ts2.tv_sec, ts2.tv_nsec,
           (ts2.tv_sec - ts1.tv_sec)*1000000000 + ts2.tv_nsec - ts1.tv_nsec);
}


/* open函数实现, 对应到Linux系统调用函数的open函数 */
static int timesync_open(struct inode *inode_p, struct file *file_p)
{
    /* 设置私有数据 */
    file_p->private_data = &timesync_char;

    return 0;
}

/* write函数实现, 对应到Linux系统调用函数的write函数 */
static ssize_t timesync_write(struct file *file_p, const char __user *buf, size_t len, loff_t *loff_t_p)
{
    int retvalue;
    unsigned int databuf;
    /* 获取私有数据 */
    //struct timesync_char_dev *dev = file_p->private_data;

    /* 获取用户数据 */
    retvalue = copy_from_user((void *)&databuf, buf, len);
    if(retvalue < 0)
    {
        printk("timesync copy_from_user failed\n");
        return -EFAULT;
    }

    if( sizeof(unsigned int) != len )
    {
        printk("length err, %ld/%ld\n", len, sizeof(unsigned int));
        return -EFAULT;
    }

    timesync_char.time_period_ms = databuf;
    printk("timesync period: %d\n", timesync_char.time_period_ms);

    if( 0 == timesync_char.time_period_ms)
        del_timer_sync(&timesync_char.timer);
    else
        mod_timer(&timesync_char.timer, jiffies + msecs_to_jiffies(timesync_char.time_period_ms));

    return 0;
}

/* write函数实现, 对应到Linux系统调用函数的write函数 */
static ssize_t timesync_read(struct file *file_p, char __user *buf, size_t len, loff_t *loff_t_p)
{
    return 0;
}

/* release函数实现, 对应到Linux系统调用函数的close函数 */
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
    timesync_char.nd = of_find_node_by_path("/timesync");
    if(timesync_char.nd == NULL)
    {
        printk("timesync node nost find\n");
        return -EINVAL;
    }

    if(0 > of_property_read_u32(timesync_char.nd, "timereg", &timesync_char.time_reg_addr))
    {
        printk("can not get timereg\n");
        return -EINVAL;
    }

    timesync_char.remap_addr_timer = (unsigned long)ioremap_wc(timesync_char.time_reg_addr, sizeof(struct ps_timer_s));
    printk("timereg/remap: 0x%08x/0x%lx\n", timesync_char.time_reg_addr, timesync_char.remap_addr_timer);
    timesync_char.ps_timer_ptr = (struct ps_timer_s *)timesync_char.remap_addr_timer;

    timesync_char.remap_addr_rtc = (unsigned long)ioremap_wc(PS_RTC_ADDR, sizeof(struct ps_rtc_s));
    printk("rtcreg/remap: 0x%08x/0x%lx\n", PS_RTC_ADDR, timesync_char.remap_addr_rtc);
    timesync_char.ps_rtc_ptr = (struct ps_rtc_s *)timesync_char.remap_addr_rtc;

    /* 注册misc设备 */
    ret = misc_register(&timesync_miscdev);
    if(ret < 0) {
        printk("misc device register failed\n");
        return -EFAULT;
    }

    /* 设置定时器回掉函数&初始化定时器 */
    timer_setup(&timesync_char.timer, timer_function, 0);

    return 0;
}

static int timesync_remove(struct platform_device *dev)
{
    /* 删除定时器 */
    del_timer_sync(&timesync_char.timer);

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
