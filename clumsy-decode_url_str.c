#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "cli-sub.h"

char *decode_url_str( char *encoded)

{
   unsigned long int conv;
   char *result = 0, *cin = 0, *cout = 0, conv_buff[ 3] = { '\0', '\0', '\0' };

   /* --- */

   if( encoded)
   {
      result = (char *) malloc( strlen( encoded) + 1);

      if( result)
      {
         for( cout = result, cin = encoded; *cin && result; cin++)
         {
            if( *cin == PERCENT_CH)
            {
               cin++;
               if( !*cin || *cin == PERCENT_CH) *cout++ = PERCENT_CH;
               else
               {
                  conv_buff[ 0] = *cin++;
                  conv_buff[ 1] = *cin;

                  if( !isxdigit( conv_buff[ 0]) || !isxdigit( conv_buff[ 1]))
                  {
                     free( result);
                     result = 0;
                  }

                  else
                  {
                     errno = 0;
                     conv = strtoul( conv_buff, 0, HEX_BASE);
                     if( !errno) *cout++ = conv;
                     else 
                     {
                        free( result);
                        result = 0;
                     }
                  }
               }
            }
            else if( *cin == PLUS_CH) *cout++ = BLANK_CH;
            else *cout++ = *cin;
         }

         *cout = EOS_CH;
      }
   }

   /* --- */

   return( result);
}
