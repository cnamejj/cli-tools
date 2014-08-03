#include "http-fetch.h"

struct chart_options *alloc_chart_options() 

{
    struct chart_options *chopt = 0;

    chopt = (struct chart_options *) malloc( sizeof *chopt );
    if( chopt )
    {
        chopt->xmin_soft = CH_OPT_NO_VALUE;
        chopt->xmin_hard = CH_OPT_NO_VALUE;
        chopt->xmax_soft = CH_OPT_NO_VALUE;
        chopt->xmax_hard = CH_OPT_NO_VALUE;
        chopt->ymin_soft = CH_OPT_NO_VALUE;
        chopt->ymin_hard = CH_OPT_NO_VALUE;
        chopt->ymax_soft = CH_OPT_NO_VALUE;
        chopt->ymax_hard = CH_OPT_NO_VALUE;
        chopt->mstone = 0;
    }

    return( chopt );
}
