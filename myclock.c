/*********************************************************************************
**
**	Modified by TKP, 4/27/2012
**		SFSU S2012
**		CSc615 homework #6:  Multi-Alarm Clock module with SYSFS access
**
**	For this assignement, I added multiple-alarm functionality to homework 5
**********************************************************************************
**
**	Modified by TKP, 4/15/2012
**		SFSU S2012
**		CSc615 homework #5:  Clock module with SYSFS access
**
**	For this assignement, I added a file (/sys/kernel/myClock2/myClock2)
**					 to the sys file system
**		using demo code provided by mcm:
** 
**
 * Very loosely based on Linux sample kobject code:
 * 	/usr/src/linux/samples/kobject_example.c
 * Timer demo code:
 * 	www.ibm.com/developerworks/linux/library/l-timers-oistindex.html?ca=drs-
 * And signal demo code:
 * 	http://people.ee.ethz.ch/~arkeller/linux/kernel_user_space_howto.html
 *
 * mcm, 4/11/12
 *
 * To compile:  make -C /usr/src/linux M=`pwd`
 * Contents of makefile:  obj-m := kobject_mcm.o 
 * Use insmod/rmmod to install and remove this module
 * Look in /var/log/messages for the module output (note that you must
 * 	'emerge syslog-ng' and 'rc-update add syslog-ng default' for output
 * 	logging to occur!
 * 
 * See kernel_user.c for a demo user program using this module!
 *
 *
**
*****************************************************************************
**
**
**	Modified by TKP, 4/1/2012
**		SFSU S2012
**		CSc615 homework #4:  clock Module.
**	
**	For this assignement, I added a file (/proc/myclock) 
**					to the proc file system
**
**
**
*****************************************************************************
**
**	Modified by TKP, 3/5/2012
**		SFSU S2012
**		CSc615 homework #3:  Hello World Module.
**		mod.c
**	
**	I have changed many of the initials "mcm" to the initials "tkp"
**	in order to correctly reflect myself as the user who has modified
**	the messages in /var/log/messages and in the /proc/testing file
**
**
** Module demo based on Kernel Projects for Linux, Lab #4 and lkmpg2.6
** To compile:  make -C /usr/src/linux M=`pwd`
** Contents of Makefile:  obj-m := mod_demo2.o
** Use insmod/rmmod to install and remove this module
** To invoke the module read routine use 'cat /proc/testing'
** Look in /var/log/messages for the module output (note that
**   you must 'emerge syslog-ng' and 'rc-update add syslog-ng default'
**   in order for output logging to occur)
**
**	MCM, 10/8/2005
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/moduleparam.h>
#include <linux/time.h> // This is needed to support CURRENT_TIME

//includes added for HW 5, from the ibm code demo
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <asm/siginfo.h>
#include <linux/timer.h>
#include <linux/sched.h>

#define	PFILE_NAME "myClock"
#define SFOLDER_NAME "myClock2" //
#define	SFILE_NAME "myClock2"   //filename same as folder for this example...
#define SIG_TEST 44

//for hw #6
#define MAX_NUM_ALARMS 10


MODULE_LICENSE("Dual BSD/GPL");

//Global Variables

//from original demo code
int n;
struct itimerval val,val2;
struct proc_dir_entry *tkp_mod_proc_file;
//struct timeval mytv;

//from hw5 ibm demo code
static int myClock2;//realtime clock 
static int read_TimeClock;//proc and sysfs clock entry
static struct timer_list my_timer[MAX_NUM_ALARMS];
struct siginfo info;
static	struct task_struct *t[MAX_NUM_ALARMS];

//for hw #6-Multi-alarm array structure
int alm_array[MAX_NUM_ALARMS+1];//for ease of shifting array~> bypass error checking
int num_alarms=0;//the number of current alarms enqueued

//now for some added hw6 code:  handler for the alarm array
int array_push(int alarm){
	int i,k=0;
	int position=0;
	int inserted=0;//a flag to tell if anything has been inserted
	if(num_alarms==0){//head_insert
		alm_array[num_alarms++]=(int)alarm;
	}
	else{
		
		for(i=0;(i<num_alarms)&&(num_alarms < MAX_NUM_ALARMS);i++){
			if(alarm > alm_array[i]){
				if(inserted==0){
					position=i;
				}
				k= alm_array[i];
				alm_array[i]=alarm;
				alarm=k;
				inserted=1;
			}
				
		}
		alm_array[i]=alarm;//extend array
		++num_alarms;
	}
	return position;
}

int pop(int value){
	int i;
	int retval=-1;
	if(num_alarms >0)
		retval=alm_array[--num_alarms];
	/*for(i=0;i< num_alarms;i++){
		alm_array[i]=alm_array[i+1];
	}*/
	alm_array[num_alarms]=-1;
	return retval;
}

//this code portion is
//for homework 5, from ibm demo code
void my_timer_callback(unsigned long data)
{
	int	ret;

	printk("kobject_tkp_myClock2: my_timer_callback called, send signal to pid %d\n",t[num_alarms-1]->pid);

//ret=pop(0);
	memset(&info,0,sizeof(struct siginfo));
	info.si_signo=SIG_TEST;
	info.si_code=SI_QUEUE;
	info.si_int=alm_array[--num_alarms];//
	ret=send_sig_info(SIG_TEST,&info,t[num_alarms]);
	if(ret < 0) {
		printk("kobject_tkp_myClock2: error sending signal\n");
	}
	//pop(1);
}

static ssize_t myClock2_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	int	ret;
	int num;
	printk("kobject_tkp_myClock2 (myClock2_show): pid= %d, myClock2 = %d\n",current->pid, myClock2);
	t[num_alarms] = current;
	//ret=array_push(myClock2);
	num=myClock2; 
	//alm_array[num_alarms]=num;
	array_push(num);
	//alm_array[num_alarms]=num;
	printk("kobject_tkp_myClock2: starting timer to fire in %d seconds\n",myClock2);
	ret = mod_timer(&my_timer[num_alarms-1],jiffies+msecs_to_jiffies(myClock2*1000));
	if (ret) printk ("kobject_tkp_myClock2:error in setting timer\n");

	return sprintf(buf, "%d\n", myClock2);

}

static ssize_t myClock2_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	sscanf(buf, "%du", &myClock2);
	printk("kobject_tkp_myClock2 (myClock2_store): pid= %d, myClock2 = %d\n",current->pid, myClock2);
	return count;
}

//our new kobjects attributes
static struct kobj_attribute clock_attribute =
	__ATTR(myClock2, 0666,  myClock2_show, myClock2_store);//changed 0666-> S_IRUGO "read only"




/*
 * More complex function where we determine which variable is being accessed by
 * looking at the attribute for the "baz" and "read_TimeClock" files.
 */
static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	if (strcmp(attr->attr.name, "read_TimeClock") == 0)
		printk(KERN_ALERT "TKP_mod(myreader), %d,%d.\n",(int)CURRENT_TIME.tv_sec,(int)CURRENT_TIME.tv_nsec);//->/var/log/messages
	return sprintf(buf, "%d, %d\n",(int)CURRENT_TIME.tv_sec,(int)CURRENT_TIME.tv_nsec);// ->/proc/myclock
	//return sprintf(buf, "%d\n", var);
}

static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	int var,ret=0;
	sscanf(buf, "%du", &var);
	if (strcmp(attr->attr.name, "read_TimeClock") == 0){
		ret = sprintf(buf, "%d, %d\n",(int)CURRENT_TIME.tv_sec,(int)CURRENT_TIME.tv_nsec);// ->/proc/myclock
		printk("b_store,read_TimeClock: var=%d\n",var);	}

	return ret;//count;
}

static struct kobj_attribute timer_attribute =
	__ATTR(read_TimeClock, 0644, b_show, b_store);//CHANGED 0666 to S_IWUSR "WRITE ONLY"

/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&clock_attribute.attr,
	&timer_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
	.attrs = attrs,
};

//the example kobject
static struct kobject *myClock2_kobj;

/*
************************************************************************
*/
//this code portion is from the original demos and hw4
int myreader(char *buffer,
	char **buffer_location,
	off_t offset, int buffer_length, int *eof, void *data)
{
	int	ret;
	printk(KERN_ALERT "TKP_mod(myreader), %d,%d.\n",(int)CURRENT_TIME.tv_sec,(int)CURRENT_TIME.tv_nsec);//->/var/log/messages
/* see lkmp2.6 for details on the logic here */
	if (offset > 0) {
		ret = 0;
	} else {
		ret = sprintf(buffer, "%d, %d\n",(int)CURRENT_TIME.tv_sec,(int)CURRENT_TIME.tv_nsec);// ->/proc/myclock
	}
	return (ret);
}

//now all projects merged
static int demo_init(void)
{
//for hw5:
	int i, retval;

//original demo and hw4 code:
	printk(KERN_ALERT "TKP_myClock2.ko initializing at %d, %d.\n",(int)CURRENT_TIME.tv_sec,(int)CURRENT_TIME.tv_nsec);
	tkp_mod_proc_file = create_proc_entry(PFILE_NAME, 0644, NULL);
	if (tkp_mod_proc_file == NULL) {
//		remove_proc_entry(PFILE_NAME,&proc_root); &proc_root obsolete!
		remove_proc_entry(PFILE_NAME,NULL);
		printk(KERN_ALERT "Error: create proc entry\n");
		return -ENOMEM;
	}
	tkp_mod_proc_file->read_proc = myreader;	
//	tkp_mod_proc_file->owner = THIS_MODULE;	owner obsolete!
	tkp_mod_proc_file->mode	= S_IFREG | S_IRUGO; /* copied from lkmp2.6 */
	tkp_mod_proc_file->uid=0;
	tkp_mod_proc_file->gid=0;
	tkp_mod_proc_file->size=80;
	printk(KERN_ALERT "/proc/myClock2 created for TKP_MOD.\n");

	printk(KERN_ALERT "workqueue created for TKP_MOD.\n");
	

//code for hw5 sysfs access:added loop for hw6
for(i=0;i<MAX_NUM_ALARMS;i++){
	setup_timer(&my_timer[i], my_timer_callback,0);
}
	
	/*
	 * Create a simple kobject with the name of "myClock2",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	myClock2_kobj = kobject_create_and_add(SFILE_NAME, kernel_kobj);
	if (!myClock2_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(myClock2_kobj, &attr_group);
	if (retval)
		kobject_put(myClock2_kobj);

	printk("kobject_mcm: Initialized timer & kobject\n");
	return retval;

//hw6
	for(i=0;i<=MAX_NUM_ALARMS;i++){//the array is one byte larger than necessary as a buffer during array shuffling
		alm_array[i]=-1;
	}
}




static void demo_exit(void)
{
//hw5 variable
int	ret, i;

//original demo and hw4 code:
	printk(KERN_ALERT "tkp:Lab#4 - TIMERS, mod.ko is being removed.\n");
//	remove_proc_entry(PFILE_NAME,&proc_root); &proc_root obsolete
	remove_proc_entry(PFILE_NAME,NULL);
	printk(KERN_ALERT "/proc/myclock removed\n");


	//code added for hw5:
	for(i=0;i<MAX_NUM_ALARMS;i++){
		ret = del_timer(&my_timer[i]);
		if (ret) printk("Kobject_mcm: The timer is still in use\n");
	}
	printk("Kobject_mcm: uninstalling\n");
	kobject_put(myClock2_kobj);
}
module_init(demo_init);
module_exit(demo_exit);
