#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>


#define FIFO_SIZE 32  

static struct kfifo test;
static struct task_struct *producer_thread;
static struct task_struct *consumer_thread;

static const unsigned char expected_result[FIFO_SIZE] = {
    6, 7, 8, 9, 10,
};

static int producer(void *data)
{
    unsigned char i;

    for (i = 1; i <= 10; i++) {
        kfifo_put(&test, i);
        pr_info("Producer inserted value: %d\n", i);
        msleep(1000);
    }

    kthread_stop(producer_thread);
    return 0;
}

static int consumer(void *data)
{
    unsigned char j;
    unsigned char buf[10];
    unsigned int ret;
    
    for (j = 1; j <= 5; j++) {
        ret = kfifo_out(&test, buf, 1);
        if (ret) {
            pr_info("Consumer removed value: %d\n", j);
        } else {
            pr_info("Consumer failed to remove value from kfifo\n");
        }
        msleep(2000);
    }

    kthread_stop(consumer_thread);
    return 0;
}

static int __init example_init(void)
{
    int ret;
    pr_info("kfifo Example Init\n");

    ret = kfifo_alloc(&test, FIFO_SIZE, GFP_KERNEL);
    if (ret) {
        pr_info("error kfifo_alloc\n");
        return ret;
    }

    producer_thread = kthread_run(producer, NULL, "producer_thread");
    if (IS_ERR(producer_thread)) {
        printk(KERN_ERR "Failed to create producer thread\n");
        return PTR_ERR(producer_thread);
    }

    consumer_thread = kthread_run(consumer, NULL, "consumer_thread");
    if (IS_ERR(consumer_thread)) {
        printk(KERN_ERR "Failed to create consumer thread\n");
        return PTR_ERR(consumer_thread);
    }

    return 0;
}

static void __exit example_exit(void)
{
    unsigned char i, j;

    /* check the correctness of all values in the fifo */
    j = 0;
    while (kfifo_get(&test, &i)) {
        pr_info("kfifo item = %d\n", i);
        if (i != expected_result[j++]) {
            pr_warn("value mismatch: test failed\n");
            goto error_EIO;
        }
    }
    pr_info("test passed\n");
    kfifo_free(&test);
    pr_info("kfifo Example Exit\n");

error_EIO:
    kfifo_free(&test);
}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL");