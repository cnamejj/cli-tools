#include <stdlib.h>
#include <arpa/inet.h>
#include "cli-sub.h"
#include "net-task-data.h"

/* --- */

struct task_details *allocate_plan_data()

{
    struct task_details *plan = 0;

    plan = (struct task_details *) malloc( (sizeof *plan));

    if( plan)
    {
        plan->use_ip = 0;
        plan->target_port = 0;
        plan->found_family = 0;
        plan->logmode = 0;
        plan->debug = 0;
        plan->show_help = 0;
        plan->msg_in_hex = 0;
        plan->openflags = LOG_OPEN_FLAGS;
        plan->target_host = 0;
        plan->message = 0;
        plan->logfile = 0;
        plan->runuser = 0;
        plan->rungroup = 0;
        plan->err_msg = 0;
        plan->dest4.sin_family = AF_INET;
        plan->dest4.sin_port = 0;
        plan->dest4.sin_addr.s_addr = 0;
        plan->dest6.sin6_family = AF_INET6;
        plan->dest6.sin6_port = 0;
        plan->dest6.sin6_flowinfo = 0;
        plan->dest6.sin6_addr = in6addr_loopback;
        plan->dest6.sin6_scope_id = SCOPE_LINK;
    }

    return( plan);
}
