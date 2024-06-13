CC = gcc
ODIR = build
SDIR = src
CFLAGS = -Wall -Werror

C_OBJS = jobCommander.o 
COBJS = $(patsubst %,$(ODIR)/%,$(C_OBJS))

S_OBJS= jobExecutorServer.o controller.o worker.o buffer.o
SOBJS = $(patsubst %,$(ODIR)/%,$(S_OBJS))

all: bin/jobCommander bin/jobExecutorServer

bin/jobCommander: $(COBJS) | bin
	gcc -Wall -Werror $(COBJS) -o $@

bin/jobExecutorServer: $(SOBJS) | bin
	gcc -Wall -Werror $(SOBJS) -o $@

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c $(INC) -o $@ $< $(CFLAGS) 	

