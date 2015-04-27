/*
 *
 * (C) 2015 Canonical Ltd., Chris J Arges <christopherarges@gmail.com>
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define AUTHOR      "Chris J Arges <chris.j.arges@canonical.com>"
#define LICENSE     "GPLv2"

struct task_struct *ts0 = NULL;
struct task_struct *ts1 = NULL;
unsigned int cpu0 = 0;
unsigned int cpu1 = 1;

/* dummy callback function */
void smp_callback(void *data) {
	unsigned int caller_cpu = *(unsigned int *)data;
	unsigned int cpu = smp_processor_id();
	printk("callback: %u <-- (%u)\n", caller_cpu, cpu);
}

int smp_thread(void *data) {
	unsigned int to_cpu = *(unsigned int *)data;
	unsigned int from_cpu = smp_processor_id();

	while(1) {
		preempt_disable();
		smp_call_function_single(to_cpu, &smp_callback, &from_cpu, 0);
		preempt_enable();
		printk("calling: (%u) --> %u\n", from_cpu, to_cpu);
	}
	return 0;
}

static int __init smpracer_init(void)
{

	ts0 = kthread_create_on_node(&smp_thread, &cpu1, 0, "test0");
	ts1 = kthread_create_on_node(&smp_thread, &cpu0, 0, "test1");
	kthread_bind(ts0,0);
	kthread_bind(ts1,1);
	wake_up_process(ts0);
	wake_up_process(ts1);

	printk("smpracer: loaded\n");
	return 0;
}

static void __exit smpracer_cleanup(void)
{
	if (ts0)
		kthread_stop(ts0);
	if (ts1)
		kthread_stop(ts1);
	printk("smpracer: exiting\n");
}

module_init(smpracer_init);
module_exit(smpracer_cleanup);

MODULE_AUTHOR(AUTHOR);
MODULE_LICENSE(LICENSE);
