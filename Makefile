 # ---------------------------------------------------------------------
 # File: makefile
 # 
 # Author: Alexandros Petridis
 # Date: September 2021
 #
 # Description: Simple makefile for local or raspberry pi build system
 # ---------------------------------------------------------------------
# Source, Executable, Includes, Library Defines
INCL   = covidTrace.h 
SRC    = main.c
OBJ    = $(SRC:.c=.o)
LIBS   = -pthread
EXE    = covidExe

# Compiler, Linker Defines
CC      = arm-linux-gnueabihf-gcc # gcc for local or arm-linux-gnueabihf-gcc for pi
CFLAGS  = -Wall -O3
LIBPATH = -L.
LDFLAGS = -o $(EXE) $(LIBPATH) $(LIBS)
RM      = rm -f


# Compile and Assemble C Source Files into Object Files
%.o: %.c
	@echo "Compile and Assemble C source file into Object file"
	$(CC) -c $(CFLAGS) $*.c

# Link all Object Files with external Libraries into Binaries
$(EXE): $(OBJ)
	@echo "Link all object files with external Libs into Binaries"
	$(CC) $(LDFLAGS) $(OBJ)

# Objects depend on these Libraries
$(OBJ): $(INCL)

# Execute executable
exec:
	@echo "Execute..."
	./$(EXE)

# Clean Up Objects, Exectuables,
clean:
	@echo "Cleaning up..."
	$(RM) $(OBJ) $(EXE) log_file.bin nearContacts.bin BTnearme_call.bin
