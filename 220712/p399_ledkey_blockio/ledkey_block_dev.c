#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>

#include <linux/fs.h>          
#include <linux/errno.h>       
#include <linux/types.h>       
#include <linux/fcntl.h>       
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wait.h>

#define   LEDKEY_DEV_NAME            "ledkey_block"
#define   LEDKEY_DEV_MAJOR            240      
#define DEBUG 0
#define IMX_GPIO_NR(bank, nr)       (((bank) - 1) * 32 + (nr))

DECLARE_WAIT_QUEUE_HEAD(WaitQueue_Read);
static int sw_irq[8] = {0};
static long sw_no = 0;
static int led[] = {
	IMX_GPIO_NR(1, 16),   //16
	IMX_GPIO_NR(1, 17),	  //17
	IMX_GPIO_NR(1, 18),   //18
	IMX_GPIO_NR(1, 19),   //19
};

static int key[] = {
	IMX_GPIO_NR(1, 20),
	IMX_GPIO_NR(1, 21),
	IMX_GPIO_NR(4, 8),
	IMX_GPIO_NR(4, 9),
  	IMX_GPIO_NR(4, 5),
  	IMX_GPIO_NR(7, 13),
  	IMX_GPIO_NR(1, 7),
 	IMX_GPIO_NR(1, 8),
};
static int led_init(void)
{
	int ret = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(led); i++) {
		ret = gpio_request(led[i], "gpio led");
		if(ret<0){
			printk("#### FAILED Request gpio %d. error : %d \n", led[i], ret);
		} 
		else
			gpio_direction_output(led[i], 0);  //0:led off
	}
	return ret;
}
#if 0
static int key_init(void)
{
	int ret = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(key); i++) {
		ret = gpio_request(key[i], "gpio key");
		if(ret<0){
			printk("#### FAILED Request gpio %d. error : %d \n", key[i], ret);
		} 
		else
			gpio_direction_input(key[i]);  
	}
	return ret;
}
#endif
irqreturn_t sw_isr(int irq, void* unused) // irqreturn 재정의 -> irqreturn_t
{
	int i;	
	for (i = 0; i < ARRAY_SIZE(key); i++){
		if(irq == sw_irq[i]) {
			sw_no = i+1;
			break;
		}
	}
	printk("IRQ : %d, %ld\n", irq, sw_no);
	wake_up_interruptible(&WaitQueue_Read);   // 프로세스sleep 상태 해제
	return IRQ_HANDLED;
}
static int key_irq_init(void)
{
	int ret = 0;
	int i;
	char* irq_name[8] = {
		"irq sw1",
		"irq sw2",
		"irq sw3",
		"irq sw4",
		"irq sw5",
		"irq sw6",
		"irq sw7",
		"irq sw8",
	};
	for(i = 0; i < ARRAY_SIZE(key); i++) {
		sw_irq[i] = gpio_to_irq(key[i]);
	} // gpio를 irq 번호로 변경
	for(i = 0; i < ARRAY_SIZE(key); i++) {
		ret = request_irq(sw_irq[i], sw_isr, IRQF_TRIGGER_RISING, irq_name[i], NULL);
		/* 할당된 irq, 인터럽트 핸들러 함수 이름, 인터럽트 발생 트리거 설정, irq 이름 설정, 매개변수가 전달될 주소??*/
		if(ret) {
			printk("### Failed request_irq %d. error : %d\n", sw_irq[i], ret);
		}
	}
	return ret;
}
static void led_exit(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(led); i++){
		gpio_free(led[i]);
	}
}
#if 0
static void key_exit(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(key); i++){
		gpio_free(key[i]);
	}
}
#endif
static void key_irq_exit(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(key); i++){
		/**/
		free_irq(sw_irq[i], NULL);
	}
}

static void led_write(char data)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(led); i++){
		gpio_set_value(led[i], (data >> i ) & 0x01);
	}
#if DEBUG
	printk("#### %s, data = %d\n", __FUNCTION__, data);
#endif
}
#if 0
static void key_read(char * key_data)
{
	int i;
	char data=0;
//	char temp;
	for(i=0;i<ARRAY_SIZE(key);i++)
	{
		if(gpio_get_value(key[i]))
		{
			data = i+1;
			break;
		}
//		temp = gpio_get_value(key[i]) << i;
//		data |= temp;
	}
#if DEBUG
	printk("#### %s, data = %d\n", __FUNCTION__, data);
#endif
	*key_data = data;
	return;
}
#endif
static int ledkey_open (struct inode *inode, struct file *filp)
{
    int num0 = MAJOR(inode->i_rdev); 
    int num1 = MINOR(inode->i_rdev); 
    printk( "call open -> major : %d\n", num0 );
    printk( "call open -> minor : %d\n", num1 );

    return 0;
}

static ssize_t ledkey_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
//    printk( "call read -> buf : %08X, count : %08X \n", (unsigned int)buf, count );
	char kbuf;
	int ret;
	/*app.c 에서 사용자가 설정하는 옵션 값은
	 filp 구조체의 f_flags 변수에 저장된다.*/
	if(!(filp->f_flags & O_NONBLOCK))
	{
		if(!sw_no) 
			//interruptible_sleep_on(&WaitQueue_Read); // 대기 큐 주소
			wait_event_interruptible(WaitQueue_Read, sw_no);
			//wait_event_interruptible_timeout(WaitQueue_Read, 10); // 10 * 1/Hz
		/*wiat_event_interruptible 함수의 경우 sw_no 값이 0이면 프로세스가 sleep 상태가
		  되기 때문에 wakeup 함수가 필요하다. sw_no 값에 0이 아닌 다른 값이 들어오면
		  프로세스가 동작한다. 즉 두 번째 매개변수의 상태가 참일 경우에만 프로세스가
		  동작하며 참이 아닐 경우에는 sleep 상태가 된다.
		  timeout 사용 시에는 별도의 wakeup 함수가 필요 없다.*/
	}
	/*block mode 이면 아래 실행*/
	/*대기 큐 이용해서 해당 프로세스를 sleep 상태로 만든다.
	 인터럽트 서비스 루틴함수에서 sleep 상태를 해제한다.*/
	kbuf = (char)sw_no;
	ret = copy_to_user(buf, &kbuf, count);
	sw_no = 0;
    return count;
}

static ssize_t ledkey_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
//    printk( "call write -> buf : %08X, count : %08X \n", (unsigned int)buf, count );
	char kbuf;
	int ret;
//	get_user(kbuf, buf);
	ret = copy_from_user(&kbuf, buf, count);
	led_write(kbuf);
    return count;
//	return -EFAULT;
}

static long ledkey_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{

    printk( "call ioctl -> cmd : %08X, arg : %08X \n", cmd, (unsigned int)arg );
    return 0x53;
}

static int ledkey_release (struct inode *inode, struct file *filp)
{
    printk( "call release \n" );
    return 0;
}

struct file_operations ledkey_fops =
{
    .owner    = THIS_MODULE,
    .read     = ledkey_read,     
    .write    = ledkey_write,    
	.unlocked_ioctl = ledkey_ioctl,
    .open     = ledkey_open,     
    .release  = ledkey_release,  
};

static int ledkey_init(void)
{
    int result;

    printk( "call ledkey_init \n" );    

    result = register_chrdev( LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME, &ledkey_fops);
    if (result < 0) return result;

	led_init();
//	key_init();
	result = key_irq_init();
	if(result < 0) return result;
    return 0;
}

static void ledkey_exit(void)
{
    printk( "call ledkey_exit \n" );    
    unregister_chrdev( LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME );
	led_exit();
//	key_exit();
	key_irq_exit();
}

module_init(ledkey_init);
module_exit(ledkey_exit);

MODULE_LICENSE("Dual BSD/GPL");
