PROGS = quipi send-udp-message capture-udp-port lock-n-run actor http-fetch svg-it
SOLOPROGS = iso8859-collapse extended-letters-collapse

SOBJS = parse_command_options.o cli_strerror.o allocate_plan_data.o get_destination_ip.o \
	parse_destination_value.o convert_to_mode.o gsub_string.o get_groupname.o \
	get_username.o switch_run_user.o switch_run_group.o int_to_str.o \
	build_syscall_errmsg.o hexdigits_to_string.o xml2-util.o called_as_cgi.o \
        get_cgi_data.o chunk_routines.o decode_url_str.o word_chain_routines.o \
	is_option_set.o is_ipv4_address.o is_ipv6_address.o url_breakout_routines.o \
	print_option_settings.o dup_memory.o get_scaled_number.o sys_call_fail_msg.o \
	errmsg_with_string.o get_matching_interface.o is_reserved_uri_char.o \
	parse_http_status.o construct_entry_form.o connect_host.o ssl_init_routines.o \
	wait_until_sock_ready.o string-routines.o explode_string.o remove_empty_strings.o \
	dup_value_chain.o free_value_chain.o free_chart_options.o

HTSOBJS = handle_ssl_error.o setup_ssl_env.o ssl_handshake.o stash_ssl_err_info.o \
	verify_ssl_callback.o bio_ssl_callback.o calc_xfrates.o calc_standard_moments.o \
	packet-svg-graphs.o alloc_chart_options.o

SVGOBJS = svg-routines.o packet-svg-graphs.o

LIBS = libCLISUB.a
UBIQ_H = err_ref.h net-task-data.h cli-sub.h
HTS_H = http-fetch.h
SVG_H = svg-tools.h

# ---

#DEBUG_DEFS = -DDEBUG_MALLOC
DEBUG_DEFS =

OS=$(shell uname)

LD_OPTS_Linux = -L/usr/lib/x86_64-linux-gnu -lrt
CFLAGS_Linux = -DUSE_CLOCK_GETTIME

CC = gcc $(DEBUG_DEFS) -Wall -g
LD = gcc
#CFLAGS = -g -Wall -pedantic -I/usr/include/libxml2 $(CFLAGS_$(OS))
CFLAGS = -g -Wall -pedantic -I/usr/include/libxml2 $(CFLAGS_$(OS)) -I/usr/local/src/debug
#CFLAGS = -g -Wall -pedantic -I/usr/include/libxml2 $(CFLAGS_$(OS)) -I/usr/local/src/debug -DDEBUG_MALLOC
#LDFLAGS = -L /extra/tmp/src/openssl-1.0.1g -lsslGGG -lcryptoGGG -L . -lCLISUB -lxml2 -lm $(LD_OPTS_$(OS)) -ldl -lsslGGG -lcryptoGGG
LDFLAGS = -L . -lCLISUB -lxml2 -lm $(LD_OPTS_$(OS)) -lssl -lcrypto
#LDFLAGS = -L . -lCLISUB -lxml2 -lm $(LD_OPTS_$(OS)) -lssl -lcrypto /usr/local/src/debug/bug_malloc.o /usr/local/src/debug/dbg_hack_info.o /usr/local/src/trace/trace_table.o
ARCOMM = ar rlc

# ---

default:
	@echo "You can make any of the following programs: $(PROGS) $(SOLOPROGS)"

# ---

.PHONY: all libs

all : $(PROGS) $(SOLOPROGS) $(LIBS)

libs : $(LIBS)

# ---

%.o : %.c Makefile %.c $(UBIQ_H) %.h
	$(CC) -c -o $@ $(CFLAGS) $<

$(SOBJS) : % : Makefile $(UBIQ_H)

$(HTSOBJS) : % : Makefile $(UBIQ_H) $(HTS_H)

$(SVGOBJS) : % : Makefile $(UBIQ_H) $(SVG_H)

$(SOLOPROGS) : % : %.o Makefile $(UBIQ_H) $(LIBS)
	$(CC) -o $@ $(@).o $(LDFLAGS)

$(PROGS) : % : %.o %.h Makefile $(UBIQ_H) $(LIBS)
	$(CC) -o $@ $(@).o $(LDFLAGS)

libCLISUB.a : $(SOBJS) $(HTSOBJS) $(SVGOBJS)
	$(ARCOMM) $@ $(SOBJS) $(HTSOBJS) $(SVGOBJS)

# ---

clean:
	rm $(PROGS) $(SOLOPROGS) $(LIBS) *.o
