PROGS = test-pull-actor-rule
SOLOPROGS = test-gsub_string test-hexdigits_to_string test-string_to_hexdigits parse-actor-config \
	test-called-as-cgi test-get-cgi-data test-decode-url-string test-is-ip4-address \
	test-is-ip6-address test-parse-url test-clock-calls test-pull-problems test-is-res-char \
        test-is-res-char2 show-net-inf test-ssl-client test-connect test-sfi test-sff \
        test-s2n-client
SOBJS = test-ssl-init-routines.o
LIBS = libTESTCLI.a
UBIQ_H = ../err_ref.h ../net-task-data.h ../cli-sub.h

# ---

DEBUG_DEFS =

OS=$(shell uname)

LD_OPTS_Linux = -L/usr/lib/x86_64-linux-gnu -lrt -lssl
CFLAGS_Linux = -DUSE_CLOCK_GETTIME

CC = gcc $(DEBUG_DEFS) -Wall -g
LD = gcc
CFLAGS = -g -Wall -pedantic -I/usr/include/libxml2 -I/usr/local/src/s2n -I/usr/local/src/s2n/api -I/usr/local/ssl/include/openssl $(CFLAGS_$(OS))
#LDFLAGS = -L /extra/tmp/src/openssl-1.0.1g -lsslGGG -lcryptoGGG -L .. -lCLISUB -lxml2 -lm $(LD_OPTS_$(OS)) -ldl
LDFLAGS = -L/usr/local/ssl/lib -lssl -lcrypto -L .. -lCLISUB -L . -lTESTCLI -L /usr/local/src/s2n/lib -ls2n -lxml2 -lm -lcrypto -ldl -lpthread $(LD_OPTS_$(OS)) -lcrypto
#LDFLAGS = -lssl -lcrypto -L .. -lCLISUB -L . -lTESTCLI -lxml2 -lm $(LD_OPTS_$(OS)) /usr/local/src/debug/bug_malloc.o  /usr/local/src/trace/trace_table.o /usr/local/src/debug/dbg_hack_info.o
ARCOMM = ar rlc

# ---

default:
	@echo "You can make any of the following programs: $(PROGS) $(SOLOPROGS)"

# ---

.PHONY: all libs

all : $(PROGS) $(SOLOPROGS)

libs: $(LIBS)

# ---

%.o : %.c Makefile %.c $(UBIQ_H) %.h
	$(CC) -c -o $@ $(CFLAGS) $<

$(SOBJS) : % : Makefile $(UBIQ_H)

$(SOLOPROGS) : % : %.o Makefile $(UBIQ_H) $(LIBS) ../libCLISUB.a
	$(CC) -o $@ $(@).o $(LDFLAGS)

$(PROGS) : % : %.o %.h Makefile $(UBIQ_H) $(LIBS) ../libCLISUB.a
	$(CC) -o $@ $(@).o $(LDFLAGS)

libTESTCLI.a : $(SOBJS)
	$(ARCOMM) $@ $(SOBJS)

# ---

clean:
	rm $(PROGS) $(SOLOPROGS) *.o *.a
