PROGS = 
SOLOPROGS = test-gsub_string test-hexdigits_to_string test-string_to_hexdigits
UBIQ_H = ../err_ref.h ../net-task-data.h ../cli-sub.h

# ---

CC = gcc
LD = gcc
CFLAGS = -g -Wall -pedantic -I..
LDFLAGS = -L .. -lCLISUB
ARCOMM = ar rlc
LIBS = ../libCLISUB.a

# ---

default:
	@echo "You can make any of the following programs: $(PROGS) $(SOLOPROGS)"

# ---

.PHONY: all libs

all : $(PROGS) $(SOLOPROGS)

# ---

%.o : %.c Makefile %.c $(UBIQ_H) %.h
	$(CC) -c -o $@ $(CFLAGS) $<

$(SOBJS) : % : Makefile $(UBIQ_H)

$(SOLOPROGS) : % : %.o Makefile $(UBIQ_H) $(LIBS)
	$(CC) -o $@ $(@).o $(LDFLAGS)

$(PROGS) : % : %.o %.h Makefile $(UBIQ_H) $(LIBS)
	$(CC) -o $@ $(@).o $(LDFLAGS)

# ---

clean:
	rm $(PROGS) $(SOLOPROGS) *.o
