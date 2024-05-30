CC = gcc
ODIR = build
SDIR = src
CFLAGS = -Wall -Werror

C_OBJS = jobCommander.o  
COBJS = $(patsubst %,$(ODIR)/%,$(C_OBJS))

S_OBJS= jobExecutorServer.o
SOBJS = $(patsubst %,$(ODIR)/%,$(S_OBJS))

all: bin/jobCommander bin/jobExecutorServer

bin/jobCommander: $(ODIR)/jobCommander.o | bin
	gcc -Wall -Werror $(ODIR)/jobCommander.o -o bin/jobCommander

bin/jobExecutorServer: $(ODIR)/jobExecutorServer.o | bin
	gcc -Wall -Werror $(ODIR)/$(S_OBJS) -o bin/jobExecutorServer

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c $(INC) -o $@ $< $(CFLAGS) 	

