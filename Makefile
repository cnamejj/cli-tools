PROGS = quipi
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
all: $(PROGS)

# ---

%.o : %.c
	$(CC) -c -o $@ $(CFLAGS) $<

$(SOBJS) : % : Makefile parse_opt.h

$(PROGS) : % : %.o %.h Makefile parse_opt.h
	$(CC) -o $@ $(@).o $(LDFLAGS)

libCLISUB.a : $(SOBJS)
	$(ARCOMM) $@ $(SOBJS)

# ---

clean:
	rm $(PROGS) *.o
