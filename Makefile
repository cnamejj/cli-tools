PROGS = quipi send-udp-message capture-udp-port
SOLOPROGS = dumb
SOBJS = parse_command_options.o cli_strerror.o allocate_plan_data.o get_destination_ip.o \
	parse_destination_value.o
LIBS = libCLISUB.a
UBIQ_H = parse_opt.h err_ref.h net-task-data.h

# ---

CC = gcc
LD = gcc
CFLAGS = -g -Wall -pedantic
LDFLAGS = -L . -lCLISUB
ARCOMM = ar rlc

# ---

default:
	@echo "You can make any of the following programs: $(PROGS) $(SOLOPROGS)"

# ---

.PHONY: all
all: $(PROGS) $(SOLOPROGS) $(LIBS)

# ---

%.o : %.c Makefile %.c
	$(CC) -c -o $@ $(CFLAGS) $<

$(SOBJS) : % : Makefile $(UBIQ_H)

$(SOLOPROGS) : % : %.o Makefile $(UBIQ_H) $(LIBS)
	$(CC) -o $@ $(@).o $(LDFLAGS)

$(PROGS) : % : %.o %.h Makefile $(UBIQ_H) $(LIBS)
	$(CC) -o $@ $(@).o $(LDFLAGS)

libCLISUB.a : $(SOBJS)
	$(ARCOMM) $@ $(SOBJS)

libs : $(LIBS)

# ---

clean:
	rm $(PROGS) $(SOLOPROGS) $(LIBS) *.o
