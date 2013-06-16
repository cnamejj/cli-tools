#include "cli-sub.h"

/* --- */

mode_t convert_to_mode( int dec_mode)

{
    int subset;
    mode_t mode;

    mode = 0;

    subset = dec_mode % 10;
    if( subset & SUBSET_EXEC) mode |= S_IXOTH;
    if( subset & SUBSET_WRITE) mode |= S_IWOTH;
    if( subset & SUBSET_READ) mode |= S_IROTH;

    subset = (dec_mode / 10) % 10;
    if( subset & SUBSET_EXEC) mode |= S_IXGRP;
    if( subset & SUBSET_WRITE) mode |= S_IWGRP;
    if( subset & SUBSET_READ) mode |= S_IRGRP;
    
    subset = (dec_mode / 100) % 10;
    if( subset & SUBSET_EXEC) mode |= S_IXUSR;
    if( subset & SUBSET_WRITE) mode |= S_IWUSR;
    if( subset & SUBSET_READ) mode |= S_IRUSR;
    
    subset = (dec_mode / 1000) % 10;
    if( subset & SUBSET_EXEC) mode |= S_ISVTX;
    if( subset & SUBSET_WRITE) mode |= S_ISGID;
    if( subset & SUBSET_READ) mode |= S_ISUID;

    return( mode);
}
