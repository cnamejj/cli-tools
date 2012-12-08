PROGS = quipi

# ---

CC = gcc
LD = gcc
CFLAGS = -g -Wall -pedantic
LDFLAGS = 

# ---

default:
	@echo "You can make any of the following programs: $(PROGS)"

# ---

.PHONY: all
all: $(PROGS)

# ---

%.o : %.c %.h Makefile
	$(CC) -c -o $@ $(CFLAGS) $<

$(PROGS) : % : %.o
	$(CC) -o $@ $(@).o $(LDFLAGS)

# ---

clean:
	rm $(PROGS) *.o
