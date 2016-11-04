//Main function

#include "pipeline.h"

using namespace std;

int main(int argc, char *argv[])
{	
	//int cnt = 0;
	char mystr[20];
	char *p;
	string line;
	ifstream traceFile;
	int pipeStall = 4;
	int pipeDone = 4;

	p = &mystr[0];

	//Command line arguments
	int robSize;
	int iqSize;
	int width;
	string fileName;

	if(argc<3)
	{
		cout << "Wrong arguments." << endl;  
	}
	else
	{
		robSize = atoi(argv[1]);
		iqSize = atoi(argv[2]);
		width = atoi(argv[3]);
		fileName = argv[4];
	}

	//Remove .txt from fileName
	/*char trace[100];
	int len;
	len = fileName.size();
	
	for ( int i=0; i<=len; i++)
	{
		trace[i] = fileName[i];
	}
 
	trace[len-4] = 0;

	string file = string(trace);*/

	//Sequence number from input trace file
	int sequence = 0;

	//Instantiate pipeline
	Pipeline processor (robSize, iqSize, width);

	strncpy(mystr, fileName.c_str(), fileName.length());
	mystr[fileName.length()]=0;
	
	//Open trace file for reading
	traceFile.open(p);

	do
	{
		if ( pipeStall == INVALID)
		{
			for ( int i=0; i<width; i++)
			{
				if ( !traceFile.eof() && getline(traceFile, line) )
				{
					processor.Instruction[i].startFE = 0;
					processor.Instruction[i].durationFE = 0;
					processor.Instruction[i].startDE = 0;
					processor.Instruction[i].durationDE = 0;	
					processor.Instruction[i].startRN = 0;
					processor.Instruction[i].durationRN = 0;
					processor.Instruction[i].startRR = 0;
					processor.Instruction[i].durationRR = 0;
					processor.Instruction[i].startDI = 0;
					processor.Instruction[i].durationDI= 0;
					processor.Instruction[i].startIS = 0;
					processor.Instruction[i].durationIS = 0;	
					processor.Instruction[i].startEX = 0;
					processor.Instruction[i].durationEX = 0;
					processor.Instruction[i].startWB = 0;
					processor.Instruction[i].durationWB = 0;
					processor.Instruction[i].startRT = 0;
					processor.Instruction[i].durationRT = 0;

					processor.Instruction[i].src1Ready = NOTREADY;
					processor.Instruction[i].src2Ready = NOTREADY;


					istringstream ss(line);

					ss>>hex>> processor.Instruction[i].pc >>dec>> processor.Instruction[i].type >> processor.Instruction[i].destReg >> processor.Instruction[i].srcReg1 >> processor.Instruction[i].srcReg2;
					processor.Instruction[i].seqNo = sequence;

					processor.Instruction[i].dest = processor.Instruction[i].destReg;
					processor.Instruction[i].rs1 = processor.Instruction[i].srcReg1;
					processor.Instruction[i].rs2 = processor.Instruction[i].srcReg2;

					switch(processor.Instruction[i].type)
					{
						case 0: processor.Instruction[i].counter = 1;
						break;
						case 1: processor.Instruction[i].counter = 2;
						break;
						case 2: processor.Instruction[i].counter = 5;
						break;
					}
					sequence++;
					//cout << "Normal " << endl;
				}

				else if ( traceFile.eof() )
				{
					processor.Instruction[i].srcReg1 = -1;
					processor.Instruction[i].srcReg2 = -1;
					processor.Instruction[i].destReg = -2;
					processor.Instruction[i].seqNo = -1;
					//cout << "End of trace file " << endl;
				}
			}

			/*//Debug print.
			for(int j=0;j<width;j++)
			{
				cout 
				<< "PC is " << hex << processor.Instruction[j].pc << " " 
				<< "type is " << dec << processor.Instruction[j].type << " "
				<< "DestReg is " << processor.Instruction[j].destReg << " "
				<< "SrcReg1 is " << processor.Instruction[j].srcReg1 << " "
				<< "SrcReg2 is " << processor.Instruction[j].srcReg2 << " "
				<< endl;
			}*/

			pipeStall = processor.Simulate();
			//cout << "CC = " << processor.currentCycle << endl;
			// cout << "Stall 1 " << pipeStall << '\n';
			// cout << '\n' << '\n' ;
			pipeDone = processor.Advance_Cycle();
			//cout << " Pipedone " << pipeDone <<  endl;
			if ( pipeDone == VALID )
			{
				processor.Retire();
			}
		}

		else
		{
			pipeStall = processor.Simulate();
			//cout << "CC = " << processor.currentCycle << endl;
			// cout << "Stall 2 " << pipeStall << endl;

			pipeDone = processor.Advance_Cycle();
			//cout << " Pipedone " << pipeDone <<  endl;
			//cout << "Same inst " << endl;
		}

	} while ( (pipeDone == INVALID) || (!traceFile.eof()) );

	traceFile.close();

	processor.ROB->front = processor.ROB->front - 1;

	cout << "# === Simulator Command =========" << '\n';
	cout << "# " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << fileName << '\n'; 
	cout << "# === Processor Configuration ===" << "\n";	
	cout << "# ROB_SIZE = " << robSize << "\n" 
	     << "# IQ_SIZE  = " << iqSize << "\n"
	     << "# WIDTH    = " << width << endl;

	//int a = processor.currentCycle;
	streamsize default_prec = cout.precision(); 
	//float ipc = 0;
	//ipc = (float)(sequence)/(float)(processor.totalCount);
	
	cout << "# === Simulation Results ========" << '\n';
	cout << "# Dynamic Instruction Count    = " << sequence << '\n';
	cout << "# Cycles                       = " << processor.totalCount << '\n';
	//printf("# Instructions per Cycle (IPC) = %0.2f\n", ipc);
	cout.precision(2);
	cout.setf(ios::fixed, ios::floatfield);
	cout << "# Instructions Per Cycle (IPC) = " << (float)(sequence)/(float)(processor.totalCount) << '\n';
	cout.unsetf(ios::floatfield);
	cout.precision(default_prec);

//	processor.Destructor();

}
