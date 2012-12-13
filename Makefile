PROGS = quipi send-udp-message
SOLOPROGS = 
SOBJS = parse_command_options.o

# ---

CC = gcc
LD = gcc
CFLAGS = -g -Wall -pedantic
LDFLAGS = 
ARCOMM = ar rlc

# ---

default:
	@echo "You can make any of the following programs: $(PROGS)"

# ---

.PHONY: all
all: $(PROGS) $(SOLOPROGS)

# ---

%.o : %.c Makefile
	$(CC) -c -o $@ $(CFLAGS) $<

$(SOBJS) : % : Makefile parse_opt.h

$(SOLOPROGS) : % : %.o Makefile parse_opt.h
	$(CC) -o $@ $(@).o $(LDFLAGS)

$(PROGS) : % : %.o %.h Makefile parse_opt.h
	$(CC) -o $@ $(@).o $(LDFLAGS)

libCLISUB.a : $(SOBJS)
	$(ARCOMM) $@ $(SOBJS)

# ---

clean:
	rm $(PROGS) *.o
