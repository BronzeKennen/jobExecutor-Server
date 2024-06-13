CC = gcc
ODIR = build
SDIR = src
CFLAGS = -Wall -Werror
LINKS = -lpthread

C_OBJS = jobCommander.o 
COBJS = $(patsubst %,$(ODIR)/%,$(C_OBJS))

S_OBJS=  controller.o worker.o buffer.o jobExecutorServer.o
SOBJS = $(patsubst %,$(ODIR)/%,$(S_OBJS))


all: bin/jobCommander bin/jobExecutorServer

bin/jobCommander: $(COBJS) | bin
	$(CC) $(CFLAGS) $(COBJS) $(LINKS) -o $@

bin/jobExecutorServer: $(SOBJS) | bin
	$(CC) $(CFLAGS) $(SOBJS) $(LINKS) -o $@

$(ODIR)/%.o: $(SDIR)/%.c | $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

bin:
	mkdir -p bin

$(ODIR):
	mkdir -p $(ODIR)

clean:
	rm -rf $(ODIR) bin

