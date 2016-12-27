/********** time - get process time in seconds **********/

#include "report.h"
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

#define TIME(x)	((float)x.tv_sec + ((float)x.tv_usec/1000000))

float 
process_time()
{
	float           system_time, user_time;
	struct rusage   rusa;

	getrusage(RUSAGE_SELF, &rusa);
	system_time = TIME(rusa.ru_stime);
	user_time = TIME(rusa.ru_utime);

	return (system_time + user_time);
}


float 
real_time()
{
	//int           gettimeofday();
	struct timeval  start_tp;
	struct timezone start_tpz;
	float           now;

	(void) gettimeofday(&start_tp, &start_tpz);


	now =
		(1000000 * (long) (start_tp.tv_sec) +
		 (long) (start_tp.tv_usec)) / 1000000.0;

	return (now);

}				/* end of realtime  */


char*
Ascii_Time()
{

	long            clock;

	clock = time(0);
	return ctime(&clock);
}
