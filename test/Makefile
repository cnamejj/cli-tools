PROGS = test-pull-actor-rule
SOLOPROGS = test-gsub_string test-hexdigits_to_string test-string_to_hexdigits parse-actor-config \
	test-called-as-cgi test-get-cgi-data test-decode-url-string test-is-ip4-address \
	test-is-ip6-address test-parse-url test-clock-calls test-pull-problems test-is-res-char \
        test-is-res-char2 show-net-inf test-ssl-client test-connect test-sfi test-sff test-svg
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
CFLAGS = -g -Wall -pedantic -I/usr/include/libxml2 $(CFLAGS_$(OS))
#LDFLAGS = -L /extra/tmp/src/openssl-1.0.1g -lsslGGG -lcryptoGGG -L .. -lCLISUB -lxml2 -lm $(LD_OPTS_$(OS)) -ldl
#LDFLAGS = -lssl -lcrypto -L .. -lCLISUB -L . -lTESTCLI -lxml2 -lm $(LD_OPTS_$(OS))
LDFLAGS = -lssl -lcrypto -L .. -lCLISUB -L . -lTESTCLI -lxml2 -lm $(LD_OPTS_$(OS))
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
