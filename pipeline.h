//Header file

#include <cmath>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <assert.h>
#include <iomanip>

#include "registers.h"

//Pipeline stage status enum
enum Status {
	FULL = 1, 
	EMPTY, 
	VALID, //3
	INVALID //4
};

enum Ready {
	READY = 1, 
	NOTREADY
};

class Pipeline
{
public:
	rmtBlock *RMT;

	ROBQueue *ROB;

	int rob_size, iq_size, width;

	Pipeline (int, int, int);
	void Destructor();
	void initialize();
	int Simulate();
	int Advance_Cycle();

	decodeRegister *DE;
	renameRegister *RN;
	regReadRegister *RR;
	dispatchRegister *DI;
	issueQueue *IQ;
	writeBackRegister *WB;
	executeList **EX;

	inst *Instruction;
	int currentCycle;
	int *retireDest1;
	int *retireDest2;
	int *wbDest;

	int flip;
	int stall;
	int totalCount;

	//Pipeline stage functions
	void Retire();
	void Writeback();
	void Execute();
	void Issue();
	void Dispatch();
	void RegRead();
	void Rename();
	void Decode();
	void Fetch();

	void printStage(inst *);
	void printWB(inst *);
};
