#ifndef _CONFIG_H
#define _CONFIG_H

/* Algorithm Strategy Options */
#define ADAPT
#define ROLLBACK
#define DONT_PARTITION
#define PRE_PARTITION 
#define PARALLEL

#ifdef PARALLEL
#define DONT_PARTITION
#endif

#endif
