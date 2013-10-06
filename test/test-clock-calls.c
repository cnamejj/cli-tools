#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>

int main( int narg, char **opts)

{
    int rc, nclocks, off;
    struct timespec *clock = 0;
    clockid_t clid, clist[] = { CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_MONOTONIC_RAW, CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID };

    clock = (struct timespec *) malloc( sizeof *clock);
    if( !clock)
    {
        fprintf( stderr, "**Err** Call to malloc() choked.\n");
        exit( 1);
    }

    clock->tv_sec = 0;
    clock->tv_nsec = 0;

    nclocks = (sizeof clist) / (sizeof clist[ 0]);

    for( off = 0; off < nclocks; off++)
    {
        clid = clist[ off];

        rc = clock_getres( clid, clock);
        if( rc)
        {
            fprintf( stderr, "**Err** Can't get clock resolution, rc=%d\n", errno);
            exit( 1);
        }

        printf( "\nClock %d: resolution %ld.%09ld seconds\n", clid, clock->tv_sec, clock->tv_nsec);

        rc = clock_gettime( clid, clock);
        if( rc)
        {
            fprintf( stderr, "**Err** Can't get clock time, rc=%d\n", errno);
            exit( 1);
        }
        printf( "Clock %d: time %ld.%09ld seconds\n", clid, clock->tv_sec, clock->tv_nsec);
    }

    exit( 0);
}
