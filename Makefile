# Makefile for disktool
# Windows disk management tool with simple GUI

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude -D_WIN32_WINNT=0x0600 -mwindows
LDFLAGS = -lcomctl32 -lcomdlg32 -lgdi32 -lole32 -luuid

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
TARGET = $(BUILDDIR)/disktool.exe

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# Default target
all: $(TARGET)

# Create build directory
$(BUILDDIR):
	if not exist $(BUILDDIR) mkdir $(BUILDDIR)

# Link object files to create executable
$(TARGET): $(BUILDDIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo Build complete: $(TARGET)

# Compile C source files to object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	if exist $(BUILDDIR) rmdir /s /q $(BUILDDIR)
	@echo Clean complete

# Run the tool
run: $(TARGET)
	$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: all

# Help
help:
	@echo Available targets:
	@echo   all     - Build the project (default)
	@echo   clean   - Remove build files
	@echo   run     - Run the compiled program
	@echo   debug   - Build with debug symbols
	@echo   help    - Show this help

.PHONY: all clean run debug help