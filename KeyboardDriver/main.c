#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#define KB_STATUS 0x64
#define KB_DATA 0x60
#define NOT_READY 256

static int scroll;
static int num;
static int caps;

const int COUNTER_VALUE = 1;
static struct semaphore sema;

unsigned char kbd_read_status(void){
    return inb(KB_STATUS);
}

//read data
unsigned  kbd_read_data(void){
   while((kbd_read_status() & 1 ) == 0);
   return inb(0x60);            //read data from 0x60
}

//write data to 0x60
void kbd_write_data(unsigned char data){
   while((kbd_read_status() & 2 ) == 2 );
   outb(data , 0x60);           // write data to 0x60
}


int update_leds(unsigned char led_status_word) {

    disable_irq(1) ;
    printk(KERN_INFO "PID: %d || START.\n", current -> pid);
    // send ’Set LEDs’ command
    printk(KERN_INFO "PID: %d || SENDING 0xED COMMAND.\n", current -> pid);
    kbd_write_data(0xED);

    // wait for ACK
    if (kbd_read_data() != 0xFA){
          enable_irq(1);
          return -1;
    }
    printk(KERN_INFO "PID: %d || RECEIVED ACK.\n", current -> pid);

    printk(KERN_INFO "PID: %d || SLEEP.\n", current -> pid);
    //msleep(500);

    printk(KERN_INFO "PID: %d || WAKE UP.\n", current -> pid);
    printk(KERN_INFO "PID: %d || SENDING KEYBOARD DATA.\n", current -> pid);
    // now send LED states
    kbd_write_data(led_status_word);

    // wait for ACK
    if (kbd_read_data() != 0xFA){
            enable_irq(1);
        return -1;
    }
    printk(KERN_INFO "PID: %d || RECEIVED ANOTHER ACK.\n", current -> pid);

    // success
    enable_irq(1);
    printk(KERN_INFO "PID: %d || EXIT.\n", current -> pid);
    return 0;
}

void set_led_state(int led, int state){

    down(&sema);

    if(led == 0){
        scroll = (state == 0)? 0:1;
    }else if(led == 1){
        num = (state == 0)? 0:1;
    }else if(led == 2){
        caps = (state == 0)? 0:1;
    }

    unsigned char led_status_word = '0';
    led_status_word = led_status_word | (scroll << 0);
    led_status_word = led_status_word | (num << 1);
    led_status_word = led_status_word | (caps << 2);

    update_leds(led_status_word);

    up(&sema);
}


int get_led_state(int led){
    if(led == 0) return scroll;
    else if(led == 1) return num;
    else if(led == 2) return caps;
}


///************* start of part two *************
static ssize_t scroll_show(struct kobject *kobj, struct kobj_attribute *attr,
            char *buf)
{
    return sprintf(buf, "%d\n", scroll);
}

static ssize_t scroll_store(struct kobject *kobj, struct kobj_attribute *attr,
             const char *buf, size_t count)
{
    int ret;
    ret = kstrtoint(buf, 10, &scroll);
    set_led_state(0, scroll);
    if (ret < 0)
        return ret;

    return count;
}

//************************************************************************************
static ssize_t num_show(struct kobject *kobj, struct kobj_attribute *attr,
            char *buf)
{
    return sprintf(buf, "%d\n", num);
}

static ssize_t num_store(struct kobject *kobj, struct kobj_attribute *attr,
             const char *buf, size_t count)
{
    int ret;
    ret = kstrtoint(buf, 10, &num);
    set_led_state(1, num);
    if (ret < 0)
        return ret;

    return count;
}

//************************************************************************************
static ssize_t caps_show(struct kobject *kobj, struct kobj_attribute *attr,
            char *buf)
{
    return sprintf(buf, "%d\n", caps);
}

static ssize_t caps_store(struct kobject *kobj, struct kobj_attribute *attr,
             const char *buf, size_t count)
{
    int ret;

    ret = kstrtoint(buf, 10, &caps);
    set_led_state(2, caps);
  if (ret < 0)
        return ret;

    return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute scroll_attribute =
    __ATTR(scroll, 0664, scroll_show, scroll_store);

static struct kobj_attribute num_attribute =
    __ATTR(num, 0664, num_show, num_store);

static struct kobj_attribute caps_attribute =
    __ATTR(caps, 0664, caps_show, caps_store);

static struct attribute *attrs[] = {
    &scroll_attribute.attr,
    &num_attribute.attr,
    &caps_attribute.attr,
    NULL,   /* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static struct kobject *example_kobj;

static int __init example_init(void){
    int retval;
    sema_init(&sema, COUNTER_VALUE);

    example_kobj = kobject_create_and_add("kobject_example", kernel_kobj);
    if (!example_kobj)
        return -ENOMEM;

    retval = sysfs_create_group(example_kobj, &attr_group);
    if (retval)
        kobject_put(example_kobj);

    return retval;
}

static void __exit example_exit(void)
{
    kobject_put(example_kobj);
}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL");
