PROGS = test-pull-actor-rule
SOLOPROGS = test-gsub_string test-hexdigits_to_string test-string_to_hexdigits parse-actor-config \
	test-called-as-cgi test-get-cgi-data test-decode-url-string test-is-ip4-address \
	test-is-ip6-address test-parse-url test-clock-calls test-pull-problems test-is-res-char \
        test-is-res-char2 show-net-inf
UBIQ_H = ../err_ref.h ../net-task-data.h ../cli-sub.h

# ---

CC = gcc
LD = gcc
CFLAGS = -g -Wall -pedantic -I.. -I/usr/include/libxml2
LDFLAGS = -L .. -lCLISUB -L/usr/lib/x86_64-linux-gnu -lxml2 -lrt
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
