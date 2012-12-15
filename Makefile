PROGS = quipi send-udp-message
SOLOPROGS = dumb
SOBJS = parse_command_options.o
LIBS = libCLISUB.a

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

$(SOBJS) : % : Makefile parse_opt.h

$(SOLOPROGS) : % : %.o Makefile parse_opt.h $(LIBS)
	$(CC) -o $@ $(@).o $(LDFLAGS)

$(PROGS) : % : %.o %.h Makefile parse_opt.h $(LIBS)
	$(CC) -o $@ $(@).o $(LDFLAGS)

libCLISUB.a : $(SOBJS)
	$(ARCOMM) $@ $(SOBJS)

libs : $(LIBS)

# ---

clean:
	rm $(PROGS) $(SOLOPROGS) $(LIBS) *.o
