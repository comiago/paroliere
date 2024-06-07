# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Directory sorgente e directory di destinazione per i file oggetto ed eseguibili
SRCDIR = .
OBJDIR = obj
BINDIR = bin

# Header files
HDRS = macros.h types.h utils.h

# Executable files
EXE1 = server
EXE2 = client

# Object files
OBJS1 = $(OBJDIR)/$(EXE1).o
OBJS2 = $(OBJDIR)/$(EXE2).o

# Source files
SRCS1 = $(SRCDIR)/$(EXE1).c
SRCS2 = $(SRCDIR)/$(EXE2).c
SRCS_UTILS = $(SRCDIR)/utils.c

# Targets
all: $(EXE1) $(EXE2)

$(EXE1): $(OBJS1) $(SRCS_UTILS) $(HDRS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$(EXE1) $(OBJS1) $(SRCS_UTILS)

$(EXE2): $(OBJS2) $(SRCS_UTILS) $(HDRS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$(EXE2) $(OBJS2) $(SRCS_UTILS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regola per creare le directory di destinazione
$(shell mkdir -p $(OBJDIR) $(BINDIR))

clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: clean
