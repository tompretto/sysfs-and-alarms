//		user.c
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>//for "exit(0)";
#include <time.h>
#include <fcntl.h>
#include <string.h>

#define NUM_ITERATIONS 300 //300=5 minutes worth if an interation equals 1 second
#define MAX_ALARMS 10
	int 	ready;
//hw5 additions
#define PFILE_NAME '/proc/myClock'
#define SIG_TEST 44 
	int foofd;
	int numread;
	char buf[25];


void receiveData(int ALARM_NUMBER, siginfo_t *info, void *unused) {
	void showMyClock2Time();
	printf("\n\n/aAlert!!! it has been %i seconds!\n  ALERT!!!  ->/sys/kernel/myClock2/timeClock = ", info->si_int);
	showMyClock2Time(0);
}
//end of hw5 additions


main(int argc, char* argv[])
{	
	
	int count;//counter for nunber of loops to run
	int alarm_timer_period;//seconds to set the alarm timer to run for
	void 	timeout();
	void showtime();
	void showMyClock2Time();
	int	i, n, k, t;
	struct	itimerval	myvalue,myovalue;
	struct timeval mytv;//my time value structure

//additions for hw 5
/* setup the signal handler for SIG_TEST 
 	 * SA_SIGINFO -> we want the signal handler function with 3 arguments
 	 */
	struct sigaction sig[MAX_ALARMS];
for(i=0;i<MAX_ALARMS;i++){
	sig[i].sa_sigaction = receiveData;
	sig[i].sa_flags = SA_SIGINFO;
	sigaction(SIG_TEST, &sig[i], NULL);
}
	if ((argc < 2) || (argc > 11)) {
		printf("usage: alarm_user ([N])*\n");
		printf("	N is the number of seconds to set the alarm for\n");
		printf("	you may have up to ten distinct alarm times per command line argument.\n");
		exit(0);
	}

	/* use sysfs file to pass timeout delay to kobject_mcm module */
	foofd = open("/sys/kernel/myClock2/myClock2", O_RDWR);
	if(foofd < 0) {
		perror("error openin /sys/kernel/myClock2/myClock2");
		return -1;
	}


printf("Tom's  timer program.  Running %d iterations.\n",NUM_ITERATIONS);

    	gettimeofday(&mytv, NULL);
	printf("Start time:  %s\n", ctime(&mytv.tv_sec) );

//create the user-sent alarms
for(i=1;i < argc;i++)
{
		sprintf(buf, "%i", atoi(argv[i]));

		if (write(foofd, buf, strlen(buf) + 1) < 0) {
			perror("fwrite"); 
			return -1;
		}
		if ((numread=read(foofd, &buf[0], sizeof(buf))) < 0) {
			perror("read");
			return -1;
		}
		printf("PID: %d, Value Read from foo: %s\n",getpid(),buf);

}
//end of additions for hw6

for(k=0; k<NUM_ITERATIONS; k++){
	//printf("Setting signal handler\n");
	if ((signal(SIGALRM, timeout)) == SIG_ERR) {
		printf("Error in setting signal handler\n");
		exit(0);
	}

	//printf("Clearing timer\n");
	timerclear(&myvalue.it_interval);
	timerclear(&myvalue.it_value);
	timerclear(&myovalue.it_interval);
	timerclear(&myovalue.it_value);
	if (( n = setitimer(ITIMER_REAL,&myvalue,&myovalue)) < 0 ) {
		printf("Error in clearing timer\n");
		exit(0);
	}

	//printf("Setting timer\n");
	myvalue.it_value.tv_sec = 1; /* timeout interval in seconds */
		/* set tv_usec for microsecond timeout */
	if (( n = setitimer(ITIMER_REAL,&myvalue,&myovalue)) < 0 ) {
		printf("Error in clearing timer\n");
		exit(0);
	}

	ready=0;
	
//	printf("Waiting for signal...\n");
//	for(;;);
		while(!ready);
		//got the signal, now go print out 
		printf("Second timer: after %d second(s), /proc/myClock=",k);
		showtime(k);
		ready=0;//wait for next alarm
		printf("Second timer: /sys/kernel/myClock2/timeClock=");
		showMyClock2Time(0);
		printf("\n");
	}
	printf("Start time: %s\n", ctime(&mytv.tv_sec) );
	gettimeofday(&mytv, NULL);
	printf("End time:  %s\n", ctime(&mytv.tv_sec) );
	
}
void timeout(int	alarmno)
{
	ready=1;//We have received the signal!
	//printf("Executed timeout!\n");
	//exit(0);
}

void showtime(int iteration){
	char proc_time[40];
    	char* date;//full current date needing to be tokenized
    	char* year;
    	char* month;
    	char* weekday;//Mon, Tues, ...
    	char* day_of_month;//01, 02, ...
	char* am_pm;

    	int current_time;
    	int hours;
    	int minutes;
    	int seconds;

    	int myclock_fd;
	struct timeval mytv;//my time value structure

    	memset(proc_time, 0, sizeof(proc_time));

//ropen the proc file
  	myclock_fd = open("/proc/myClock", O_RDONLY);
	if(myclock_fd < 0) {
	        perror("Bad read file open");
	        exit(-1);
	}

    	read(myclock_fd, proc_time, sizeof(proc_time));

    	gettimeofday(&mytv, NULL);
    	date = ctime(&mytv.tv_sec);

    	weekday = strtok(date, " ");
    	month = strtok(NULL, " ");
    	day_of_month = strtok(NULL, " ");
    	strtok(NULL, " ");
    	year = strtok(NULL, " ");

    	current_time = atoi(proc_time);

	hours = (current_time-25200)/3600%24;
	if(hours>12){hours-=12;}
	minutes = current_time/60%60;
	seconds = current_time%60;
	am_pm = (hours<=12)?("am"):("pm");

    	printf("%s %s, %s, %02d:%02d:%02d %s %s", weekday,month, day_of_month, hours, minutes, seconds, am_pm, year);

	
    	close(myclock_fd);
}

void showMyClock2Time(int iteration){
	char sysTimeClock_time[40];
    	char* date;//full current date needing to be tokenized
    	char* year;
    	char* month;
    	char* weekday;//Mon, Tues, ...
    	char* day_of_month;//01, 02, ...
	char* am_pm;

    	int current_time;
    	int hours;
    	int minutes;
    	int seconds;

    	int myclock_fd;
	struct timeval mytv;//my time value structure

    	memset(sysTimeClock_time, 0, sizeof(sysTimeClock_time));

//ropen the proc file
  	myclock_fd = open("/sys/kernel/myClock2/read_TimeClock", O_RDONLY);
	if(myclock_fd < 0) {
	        perror("Bad read file open");
	        exit(-1);
	}

    	read(myclock_fd, sysTimeClock_time, sizeof(sysTimeClock_time));

    	gettimeofday(&mytv, NULL);
    	date = ctime(&mytv.tv_sec);

    	weekday = strtok(date, " ");
    	month = strtok(NULL, " ");
    	day_of_month = strtok(NULL, " ");
    	strtok(NULL, " ");
    	year = strtok(NULL, " ");

    	current_time = atoi(sysTimeClock_time);

	hours = (current_time-25200)/3600%24;
	if(hours>12){hours-=12;}
	minutes = current_time/60%60;
	seconds = current_time%60;
	am_pm = (hours<=12)?("am"):("pm");

    	printf("%s %s, %s, %02d:%02d:%02d %s %s", weekday,month, day_of_month, hours, minutes, seconds, am_pm, year);

	
    	close(myclock_fd);
}

// read user input
void get_input(char *buf) {
    int input = scanf("%s", buf);
    if(input == EOF)
	printf("ERROR: Get user Input failed\n");
}

