PROGS = quipi send-udp-message capture-udp-port lock-n-run actor
SOLOPROGS = iso8859-collapse extended-letters-collapse
SOBJS = parse_command_options.o cli_strerror.o allocate_plan_data.o get_destination_ip.o \
	parse_destination_value.o convert_to_mode.o gsub_string.o get_groupname.o \
	get_username.o switch_run_user.o switch_run_group.o int_to_str.o \
	build_syscall_errmsg.o hexdigits_to_string.o xml2-util.o
LIBS = libCLISUB.a
UBIQ_H = err_ref.h net-task-data.h cli-sub.h

# ---

CC = gcc
LD = gcc
CFLAGS = -g -Wall -pedantic -I/usr/include/libxml2
LDFLAGS = -L . -lCLISUB -L/usr/lib/x86_64-linux-gnu -lxml2
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

$(SOLOPROGS) : % : %.o Makefile $(UBIQ_H) $(LIBS)
	$(CC) -o $@ $(@).o $(LDFLAGS)

$(PROGS) : % : %.o %.h Makefile $(UBIQ_H) $(LIBS)
	$(CC) -o $@ $(@).o $(LDFLAGS)

libCLISUB.a : $(SOBJS)
	$(ARCOMM) $@ $(SOBJS)

# ---

clean:
	rm $(PROGS) $(SOLOPROGS) $(LIBS) *.o
