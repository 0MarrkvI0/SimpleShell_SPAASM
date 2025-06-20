CC = gcc
CFLAGS = -Wall -g
SOURCES = main.c shell.c client_utils.c server_utils.c
OBJECTS = $(SOURCES:.c=.o)
EXEC = endpoint

# Default target
all: $(EXEC)

# Linking the object files to create the executable
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)

# Rule to compile .c files to .o files
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Clean up generated files
clean:
	rm -f $(OBJECTS) $(EXEC)
