//Pipeline register function definitions.

#include "pipeline.h"
using namespace std;

//Decode register
decodeRegister::decodeRegister(int w)
{
    width = w;
    DEInstruction = new inst[width];
}

void decodeRegister::initialize()
{
    valid = INVALID;
    status = EMPTY;
}

void decodeRegister::sendDE(inst *I)
{
	for ( int i=0; i<width; i++)
	{
		DEInstruction[i] = I[i];
	}

	//Make status = 1 to indicate filled Decode Stage.
	status = FULL;
	valid = VALID;

	return;
}

int decodeRegister::DEStatus()
{
	return status;
}

int decodeRegister::DEState()
{
	return valid;
}

//Rename register
renameRegister::renameRegister(int w)
{
    width = w;
    Instruction = new inst[width];
}

void renameRegister::initialize()
{
    valid = INVALID;
    status = EMPTY;
}

void renameRegister::sendRN(inst *I)
{
	for ( int i=0; i<width; i++)
	{
		Instruction[i] = I[i];
	}

	valid = VALID;
	status = FULL;	
}

int renameRegister::RNStatus()
{
	return status;
}

int renameRegister::RNState()
{
	return valid;
}


//Register Read Register
regReadRegister::regReadRegister(int w)
{
    width = w;
    Instruction = new inst[width];
}

void regReadRegister::initialize()
{
    valid = INVALID;
    status = EMPTY;
}

int regReadRegister::RRStatus()
{
	return status;
}

int regReadRegister::RRState()
{
	return valid;
}

void regReadRegister::sendRR(inst *I)
{
	for ( int i=0; i<width; i++)
	{
		Instruction[i] = I[i];
	}

	status = FULL;	
	valid = VALID;	
}

//Dispatch Register
 dispatchRegister::dispatchRegister(int w)
 {
 	width = w;
    Instruction = new inst[width];
 }

 void dispatchRegister::initialize()
 {
 	valid = INVALID;
 	status = EMPTY;
 }

 int dispatchRegister::DIStatus()
 {
 	return status;
 }


int dispatchRegister::DIState()
{
	return valid;
}

void dispatchRegister::sendDI(inst *I)
{
	for ( int i=0; i<width; i++)
	{
		Instruction[i] = I[i];
	}

	status = FULL;
	valid = VALID;
}

//**************************************Issue Queue***********************************//
issueQueue::issueQueue(int s, int w)
{
	size = s;
	width = w;
	cage = 0;
	IQ = new iqBlock[size];
	sortedIQ = new iqBlock[size];
	Instruction = new inst[width*5];
}

void issueQueue::initialize()
{	
	IQValid = INVALID;
	for(int i=0; i<size; i++)
	{
		IQ[i].valid = INVALID;
		IQ[i].age = 0;
	}
}

//Check if there are 'width' free entries in the issue queue
int issueQueue::IQStatus(int w)
{
	int count = 0;
	for(int i=0; i<size; i++)
	{
		if ( IQ[i].valid == INVALID)
			count++;
	}

	//cout << "IQ empty count = " << count << '\n';
	if(count >= width)
		return EMPTY;
	else return FULL;

}

int issueQueue::IQState()
{
	return IQValid;
}

void issueQueue::sendIQ(inst *I)
{
	//Get instruction bundle and place it in IQ
	int count = width;
	int i = 0;
	IQValid = VALID;
	//cout << "Cnt = " << count << endl;
	
	do
	{
		for(int j=0; j<size; j++)
		{
			if (IQ[j].valid == INVALID)
			{
				 cage++;	
				 IQ[j].valid = VALID;
				 IQ[j].age = cage;
				 IQ[j].Instr.src1Ready = I[i].src1Ready;
				 IQ[j].Instr.src2Ready = I[i].src2Ready;
				 
				 IQ[j].Instr = *(I+i);
				 count--;
				 i++;
			}

			if (count == 0)
				break;
		}

	} while(count!=0);

	return;
}

inst * issueQueue::getIssueBundle(int w)
{
	inst *Instr;
	Instr = new inst[width];

	//Get the oldest 'wdith=w' instructions and then return them.
	sortIssueQueue();
	int count = 0;
	int k = 0;

	for(int i=0; i<size; i++)
	{
		if ( (IQ[i].valid == VALID) && (IQ[i].Instr.src1Ready == READY) && (IQ[i].Instr.src2Ready == READY)  )
		{	
			//Add instruction to issue bundle.

			Instr[k] = IQ[i].Instr;
			k++;
			count++;

			//Make that block in IQ free i.e. INVALID:
			IQ[i].valid = INVALID;
		}

		if ( count == w)
			break;
	}

	issueNum = count;

	return Instr;
}

//Sort IQ by age.
//Bubble sort algorithm.
void issueQueue::sortIssueQueue()
{
	iqBlock temp;
	for ( int j=0; j<size; j++)
	{
		sortedIQ[j] = IQ[j]; 
	}

	for( int j=0; j<size-1; j++)
	{
		for ( int i=0; i<size - j - 1; i++ )
		{
			if ( sortedIQ[i+1].age < sortedIQ[i].age)
			{
				temp = sortedIQ[i];
				sortedIQ[i] = sortedIQ[i+1];
				sortedIQ[i+1] = temp;			
			}
		}	
	}

	for ( int j=0; j<size; j++)
	{
		IQ[j] = sortedIQ[j]; 
	}
	return;
}

//Execute List stage. 5 stage pipelined FU.
executeList::executeList(int w)
{
	width = w;
	EXValid = INVALID;
    Instruction = new inst[width*5];	
}

void executeList::initialize()
{
	EXValid = INVALID;
	for ( int k=0; k<5; k++)
	{
		wakeup[k] = -2;
		valid[k] = INVALID;
	}
}

void executeList::sendEX(inst * I)
{
	//Advance other instructions in Instruction array.
	for (int i=3; i>=0; i--)
	{
		Instruction[i+1] = Instruction[i];
		valid[i+1] = valid[i];
	}

	//Fill in the first position.
	Instruction[0] = *(I+0);
	valid[0] = VALID;
	EXValid = VALID;

}

int executeList::EXState()
{
	return EXValid;
}

//Writaback stage

writeBackRegister::writeBackRegister(int w)
{
	width = w;
	WBValid = INVALID;
	Instruction = new inst[width*5];
}

void writeBackRegister::initialize()
{
	WBValid = INVALID;
	valid = new int[width*5];
	for ( int i=0; i<width*5; i++)
	{
		valid[i] = INVALID;
	}
}

void writeBackRegister::sendWB(inst *I)
{
	//Advance other instructions in Instruction array.
	for (int i=width*5 - 2; i>=0; i--)
	{
		Instruction[i+1] = Instruction[i];
		//Mark instruction as valid in WB pipeline register.		
		valid[i+1] = valid[i];
	}

	Instruction[0] = *(I+0);
	valid[0] = VALID;

	WBValid = VALID;
}

int writeBackRegister::WBState()
{
	return WBValid;
}

//ROBQueue functions

robBlock * ROBQueue::generateROBEntry(inst *I, int i)
{   
    robBlock *e;
    e = new robBlock;

    e->valid = VALID;
    e->tag = back + 1;
    e->Instr= *(I+i);
    e->ready = 0;

    return e;
}

ROBQueue::ROBQueue(int size) 
{
    front = back = -1;
    ROBQueue::size = size - 1;
    ROB = new robBlock[size];
    count = 0;
}

void ROBQueue::initialize()
{
    robValid = INVALID;

    //Invalidate all robBlocks
    for(int i =0; i <= size; i++)
    {
        ROB[i].valid = INVALID;
    }
}

int ROBQueue::enqueue(robBlock *entry) {
    if (( (front == 0) && (back == size) ) || (front == back + 1)) {
        //cout << "ROB is full enqueue\n";
        return 0;
    }
    else if (front == -1 && back == -1) {
        front = 0;
        back = 0;
        ROB[back] = *entry;
        ROB[back].valid = VALID;
        count++;
    }
    else if (back == size) {
        back = 0;
        ROB[back] = *entry;
        ROB[back].valid = VALID;
        count++;
    }
    else {
        back++;
        ROB[back] = *entry;
        ROB[back].valid = VALID;
        count++;
    }
    return 1;
}
 
void ROBQueue::dequeue() {
    if (front == -1 && back == -1) {
        //cout << "ROB is empty\n";
    }
    else {
        if (front == back) {
        ROB[front].valid = INVALID;
        ROB[front].ready = NOTREADY;
        front = -1;
        back = -1;
        count--;
    }
    else if (front == size) {
        ROB[front].valid = INVALID;
        ROB[front].ready = NOTREADY;        
        front = 0;
        count--;
    }
    else {
    	//cout << "Here front is " << front << endl; 
        ROB[front].valid = INVALID;
        //cout << "ROB 0 " << ROB[0].valid << endl;
        ROB[front].ready = NOTREADY;
        front++;
        count--;
    }
    }

    return;
}
 
void ROBQueue::show() {
    if (count == 0) {
        //cout << "ROB is empty\n";
    } else {
        for(int i = 0; i <= size; i++)
            cout 
        	<< "Valid[" << i << "] " << ROB[i].valid << " "
        	<< "Tag[" << i << "] " << ROB[i].tag << " "
        	<< "Ready[" << i << "] " << ROB[i].ready << " "
        	<< "Dest[" << i << "] " << ROB[i].Instr.destReg << " "
        	<< "PC[" << i << "] " << ROB[i].Instr.pc << " " << endl;
    }

    return;
}

//Check if ROB has space for 'w' new entries
int ROBQueue::ROBStatus(int w) {
    int a, b;

    if (( (front == 0) && (back == size) ) || (front == back + 1)) {
        //cout << "ROB is full 1\n";
        return FULL;
    }

    if (front == -1 && back == -1) {
        //cout << "ROB is empty\n";
        return EMPTY;
    }

    if ( back > front)
    {
        a = size - back;
        b = front;
        if ( a + b >= w)
        {
        	//cout << "ROB is empty\n";
            return EMPTY;
        }
        else 
        {
        	//cout << "ROB is full 2\n";
        	return FULL;
        }
    }

    if ( back < front)
    {
        if ( (front - back - 1) >= w )
        {
        	//cout << "ROB is empty\n";
        	return EMPTY;
        }
        else 
        {
        	//cout << "ROB is full 3\n";
        	return FULL;
   		}
   	}

   	//cout << "ROB is empty\n";	
    return EMPTY;
}

