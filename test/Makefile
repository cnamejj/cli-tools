PROGS = test-pull-actor-rule
SOLOPROGS = test-gsub_string test-hexdigits_to_string test-string_to_hexdigits parse-actor-config \
	test-called-as-cgi test-get-cgi-data test-decode-url-string test-is-ip4-address \
	test-is-ip6-address test-parse-url test-clock-calls test-pull-problems test-is-res-char \
        test-is-res-char2 show-net-inf test-ssl-client test-connect
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
LDFLAGS = -lssl -lcrypto -L .. -lCLISUB -lxml2 -lm $(LD_OPTS_$(OS))
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
