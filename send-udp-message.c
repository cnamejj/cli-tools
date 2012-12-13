#include <stdlib.h>
#include "parse_opt.h"
#include "send-udp-message.h"

/* --- */

int main( int narg, char **opts)

{
   int rc = 0;
   struct option_set opset[] = {
     { OP_DEST_BOTH, OP_TYPE_CHAR, 0, FL_DEST_BOTH, 0, DEF_DEST_BOTH, 0 },
     { OP_MESSAGE, OP_TYPE_CHAR, 0, FL_MESSAGE, 0, DEF_MESSAGE, 0 },
     { OP_PORT, OP_TYPE_INT, 0, FL_PORT, 0, DEF_PORT, 0 },
     { OP_HOST, OP_TYPE_CHAR, 0, FL_HOST, 0, DEF_HOST, 0 },
     { OP_IPV4, OP_TYPE_INT, 0, FL_IPV4, 0, DEF_IPV4, 0 },
     { OP_IPV6, OP_TYPE_INT, 0, FL_IPV6, 0, DEF_IPV6, 0 },
   };

   exit( rc );
}
