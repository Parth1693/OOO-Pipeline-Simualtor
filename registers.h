//Register definitions

//Instruction structure
typedef struct {
	int seqNo;
	unsigned int pc;
	int type;

	//Original values
	int dest;
	int rs1;
	int rs2;

	int destReg;
	int srcReg1;
	int srcReg2;

	int src1Ready;
	int src2Ready;

	int renamed1;
	int renamed2;

	int RMTtag1;
	int RMTtag2;

	int startFE;
	int durationFE;
	int startDE;
	int durationDE;
	int startRN;
	int durationRN;
	int startRR;
	int durationRR;
	int startDI;
	int durationDI;
	int startIS;
	int durationIS;
	int startEX;
	int durationEX;
	int startWB;
	int durationWB;
	int startRT;
	int durationRT;

	int robTag;

	int counter;

} inst;

//Block in RMT.
//RMT size is 67 blocks.
typedef struct mapTableBlock {
	int RMTValid;
	int tag;
} rmtBlock;

//Block in Issue Queue.
typedef struct issueQueueBlock {
	int valid;
	long int age;
	
	inst Instr;
} iqBlock;


//ROB block.
typedef struct robBlock {
	
	int valid;
	int tag;	
	int ready;
	inst Instr;

} robBlock;


class decodeRegister {
public:
	int valid;
	int status;

	int width;
	inst *DEInstruction;
	
	decodeRegister(int);
	void initialize();
	void sendDE(inst *);	
	int DEStatus();
	int DEState();
};

class renameRegister {
public:
	int valid;
	int status;

	int width;
	inst *Instruction;

	renameRegister(int);
	void initialize();
	void sendRN(inst *);
	int RNStatus();
	int RNState();
};

class regReadRegister {
public:
	int valid;
	int status;

	int width;
	inst *Instruction;

	regReadRegister(int);
	void initialize();
	void sendRR(inst *);
	int RRStatus();
	int RRState();
};

class dispatchRegister {
public:
	int valid;
	int status;

	int width;
	inst *Instruction;

	dispatchRegister(int);
	void initialize();
	void sendDI(inst *);
	int DIStatus();
	int DIState();
};

class issueQueue {
public:
	int size;
	int width;
	int IQValid;
	int issueNum;
	long int cage;

	iqBlock *IQ;
	iqBlock *sortedIQ;

	inst *Instruction;

	issueQueue(int, int);
	void initialize();
	void sendIQ(inst *);
	int IQStatus(int);
	int IQState();
	void sortIssueQueue();

	inst * getIssueBundle(int);
};

class writeBackRegister {
public:
	int width;		//Use width*5 in functions.
	int WBValid;
	int *valid;		//width*5

	inst * Instruction;

	writeBackRegister(int);
	void initialize();
	void sendWB(inst *);
	int WBState();
};

class executeList {
public:
	int width;
	int EXValid;
	int valid[5];
	int wakeup[5];

	//Need to hold 5 instructions concurrently.
	//So Instruction[0], Instruction[1], Instruction[2]...
	inst * Instruction;

	executeList(int);
	void initialize();
	void sendEX(inst *);
	int EXState();

};

//Cicular FIFO for ROB Queue
class ROBQueue {
public:
	int front, back, size, count;
	int robValid;

	robBlock *ROB;
	ROBQueue(int);
	void initialize();
	int enqueue(robBlock *);
    void show();
    void dequeue();
    robBlock * generateROBEntry(inst *, int);
    int ROBStatus(int);
};

