CC = g++
OPT = -O3
OPT = -g
WARN = -Wall
//ERR = -Werror

CFLAGS = $(OPT) $(WARN) $(ERR) $(INC) $(LIB)

SIM_SRC = main.cc pipeline.cc registers.cc

SIM_OBJ = main.o pipeline.o registers.o

all: sim
	@echo "Compilation Done ---> nothing else to make :) "

sim: $(SIM_OBJ)
	$(CC) -o sim $(CFLAGS) $(SIM_OBJ) -lm
	@echo "----------------------------------------------------------"
	@echo "-----------FALL15-506 Multilevel Cache Protocol SIMULATOR (SMP_CACHE)-----------"
	@echo "----------------------------------------------------------"
 
.cc.o:
	$(CC) $(CFLAGS)  -c $*.cc

clean:
	rm -f *.o sim

clobber:
	rm -f *.o


