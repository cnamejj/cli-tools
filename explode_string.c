#include "cli-sub.h"
#include "err_ref.h"
#include <string.h>
#include <stdlib.h>

/* --- */

struct string_parts *explode_string( int *rc, char *string, char *delim)

{
   int dlen, count, ii, err = ERR_BAD_PARMS;
   char *st, *buff = 0;
   struct string_parts *rez = 0;

   if( string && delim) if( *string && *delim) err = RC_NORMAL;

   if( err == RC_NORMAL)
   {
      buff = strdup( string);
      if( !buff) err = ERR_MALLOC_FAILED;
      else
      {
         rez = (struct string_parts *) malloc( sizeof *rez);
         if( !rez) err = ERR_MALLOC_FAILED;
      }
   }

   if( err == RC_NORMAL)
   {
      rez->np = 0;
      rez->list = 0;

      dlen = strlen( delim);
      count = 1;

      for( st = buff; *st; )
      {
         if( strncmp( st, delim, dlen)) st++;
         else
         {
            count++;
            *st = '\0';
            st += dlen;
         }
      }

      rez->list = (char **) malloc( count * (sizeof *(rez->list)));
      if( !rez->list) err = ERR_MALLOC_FAILED;
      else rez->np = count;
   }

   if( err == RC_NORMAL)
   {
      for( ii = 0; ii < count; ii++)
        rez->list[ ii] = 0;

      for( ii = 0, st = buff; ii < count && err == RC_NORMAL; ii++)
      {
         rez->list[ ii] = strdup( st);
         if( !rez->list[ ii]) err = ERR_MALLOC_FAILED;
         else st += strlen( st) + dlen;;
      }

      if( buff) free( buff);
   }

   if( rc) *rc = err;

   return rez;
}
