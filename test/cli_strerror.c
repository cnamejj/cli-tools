
#define ALLOC_ERR_TEXT
#include "err_ref.h"


char *cli_strerror( int error)

{
   int wh, nerr;
   char *rez = 0;
   static char unknown_err[] = "Unknown error code";

   nerr = (sizeof err_msg_list) / (sizeof *err_msg_list);

   for( wh = 0; wh < nerr && !rez; wh++)
     if( err_msg_list[ wh].err == error) rez = err_msg_list[ wh].emsg;

   if( !rez) rez = unknown_err;

   return rez;
}
