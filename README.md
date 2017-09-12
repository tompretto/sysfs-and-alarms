# sysfs-and-alarms
Add module to sys file system: /sys/kernel/myclock2/myclock2
Kernel module which adds multiple alarms functionality.

Thomas Pretto 
CSc 615, Homework #5: 
Adding sysfs access & Alarms for HW#4 Clock. 
SFSU S2012 –  
Professor Marguerite Murphy 
 
Problem Description: 
For this assignment, we added further functionality to our previous clock and calendar homework:  Homework #4.  In the previous assignment, we added a read-only /proc filesystem entry which, when read from, returned two simple values separated by a comma:  The time in milliseconds since Midnight Jan 1, 1970, and the number of milliseconds since the last second which had just passed; with a single solitary space between the comma and the number of milliseconds.  For Homework #5, we further added a sysfs entry into the folder /sys/kernel using kernel objects (kobjects) and an internal array that allowed the user to set-up up to ten separate alarms. 
The sysfs is a virtual filesystem representation of the system’s device topology which allows for easy traversal or enumeration of all of a systems devices, including all busses and interconnections.  This tree of devices is enabled through kobjects (kernel objects), which are structs found in <linux/kobject.h>.   The kobject struct is likened to an Object class.  It contains a basic template for the creation of a hierarchy of objects.    
struct kobject { 
const char*name;//name of this kobject 
struct list_headentry; 
struct kobject*parent;//kobjects parent 
struct kset*kset; 
struct kobj_type*ktype; 
struct sysfs_dirent*sd;//struct containing inode structure representing kobject in sysfs. 
struct krefkref;// 
unsigned intstate_initialized:1; 
unsigned intstate_in_sysfs:1; 
unsigned intstate_add_uevent_sent:1; 
unsigned intstate_remove_uevent_sent:1; 
unsigned intuevent_suppress:1; 
} 
 
The sysfs entry created for this assignment was a virtual directory called myClock2 (/sys/kernel/myclock2), which contained two attributes:  the first was a replication of homework 4’s /proc filesystem entry, in the sysfs, while the second was a writeable attribute which acts as an interface for our new kernel timer.  The first node, renamed real_TimeClock (name changed due to an ambiguity between the original name “myClock”, and the new “myClock2” entry) returned the time in the format seconds, milliseconds exactly as the /proc filesystem entry.  The second attribute is the new and exciting part of this project.  The sysfs entry myClock2 is writable.  Not only is the virtual entry writable, but writing any integer number to this file starts a kernel timer for the number of seconds specified, at the end of which time a real-time alarm is sent to user-space, along with a copy of the value of the original timer assignment.   
The user space component of this program loops for 300 seconds (0-299), printing the time every second.  As alarm signals are sent from the kernel-space to the user-space, the signals are caught by the user-space program and the amount of time that the alarm had been set for is printed.  As the user-space program counts from 0-299, it is off by one second.  This is the one second of difference that is seen between the timer alarm messages and the second counts. 
