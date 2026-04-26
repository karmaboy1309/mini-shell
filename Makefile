# ──────────────────────────────────────────────────────────────────
#  Makefile — Mini Unix Shell (mysh)
# ──────────────────────────────────────────────────────────────────

CC       = gcc
CFLAGS   = -Wall -Wextra -Wpedantic -std=c99 -D_POSIX_C_SOURCE=200809L
LDFLAGS  =

# Directories
SRCDIR   = src
INCDIR   = include
OBJDIR   = obj
BINDIR   = bin

# Source / object lists
SRCS     = $(wildcard $(SRCDIR)/*.c)
OBJS     = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
TARGET   = $(BINDIR)/mysh

# ── Default target ──────────────────────────────────────────────
.PHONY: all clean debug

all: $(TARGET)

# ── Link ────────────────────────────────────────────────────────
$(TARGET): $(OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# ── Compile ─────────────────────────────────────────────────────
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

# ── Create directories ──────────────────────────────────────────
$(BINDIR) $(OBJDIR):
	mkdir -p $@

# ── Debug build (with symbols, sanitizer) ───────────────────────
debug: CFLAGS += -g -O0 -fsanitize=address,undefined
debug: LDFLAGS += -fsanitize=address,undefined
debug: all

# ── Clean ───────────────────────────────────────────────────────
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# ── Quick build (gcc *.c -o mysh) ──────────────────────────────
.PHONY: quick
quick:
	$(CC) $(CFLAGS) -I$(INCDIR) $(SRCS) -o mysh
