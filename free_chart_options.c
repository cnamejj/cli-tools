#include "http-fetch.h"

void free_chart_options( struct chart_options *chopt)

{
    struct milestone *walk, *curr;

    if( chopt )
    {
        if( chopt->data_line_color) free( chopt->data_line_color);
        chopt->data_line_color = 0;

        for( walk = chopt->mstone; walk; )
        {
            curr = walk;
            walk = curr->next;

            curr->next = 0;
            if( curr->label) free( curr->label);
            curr->label = 0;
            free( curr);
	}
        chopt->mstone = 0;

        free( chopt);
    }

    return;
}
