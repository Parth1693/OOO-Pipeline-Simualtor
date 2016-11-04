#include "pipeline.h"
using namespace std;

Pipeline::Pipeline (int rob, int iq, int w)
{
	rob_size = rob;
	iq_size = iq;
	width = w;

    retireDest1 = new int[width*5];
    retireDest2 = new int[width*5];
    wbDest = new int[width*5];

	RMT = new rmtBlock[67];

	ROB = new ROBQueue(rob_size);

    //Initialize instruction array
    Instruction = new inst[width];
	
    //Instantiate all pipeline registers.
    DE = new decodeRegister(width);
    RN = new renameRegister(width);  
    RR = new regReadRegister(width);
    DI = new dispatchRegister(width);
    IQ = new issueQueue(iq_size, width); 
    WB = new writeBackRegister(width);

    //'Width' number of execution units present in the processor.
    EX = new executeList*[width];
    
    for( int i=0; i<width; i++)
    {
        EX[i] = new executeList(width);
    }     

	initialize();
}


void Pipeline::Destructor()
{   
    //Delete all pointers.
    //delete[] DE->DEInstruction;
    delete DE;
    //delete RN->Instruction;
    delete RN;
    //delete RR->Instruction;
    delete RR;
    //delete DI->Instruction;
    delete DI;
    ///delete IQ->IQ;
    //delete IQ->sortedIQ;
    delete IQ;
    //delete WB->valid;
    //delete WB->Instruction;
    delete WB;

    // for ( int i=0; i<width; i++)
    // {
    //     delete EX[i]->Instruction;
    // }

    delete [] EX;

    delete RMT;
    //delete ROB->ROB;
    delete ROB;    
}


void Pipeline::initialize()
{
	//Set all valid values to 0.
	//RMT initialization
	for( int i=0;i<67;i++)
	{
		RMT[i].RMTValid = INVALID;
	}

    for ( int i=0; i<width*5; i++)
    {
        retireDest1[i] = -2;
        retireDest2[i] = -2;
        wbDest[i] = -2;
    }

    currentCycle = 0;
    flip = 1;
    stall = 0;
    totalCount = 0;

	//Instantiate and initialize all pipeline registers of given width.
    DE->initialize();

    RN->initialize();

    RR->initialize();	

    DI->initialize();

    //IQ initialization
    IQ->initialize();

    for ( int i=0; i<width; i++)
    {
        EX[i]->initialize();        
    }

    WB->initialize();

    //ROB initialization
    ROB->initialize();

}

int Pipeline::Advance_Cycle()
{
    currentCycle++;
    //Check if pipeline is empty i.e. ROB has been written to the ARF. (No more instructions in Retire stage)
    //cout << "In Advance Cycle" ;
    if ( currentCycle > 2 && ROB->ROBStatus(ROB->size) == EMPTY )
        return VALID;        //Pipe is done.
    else
        return INVALID;     //Pipe is NOT done.
}

int Pipeline::Simulate()
{
    Retire();

    Writeback();

    Execute();

    Issue();

    Dispatch();
    
    RegRead();

    Rename();

    Decode();

    Fetch();

    //If pipeline is stalled, return VALID.
    if ( stall == 1)
    return VALID;       //Pipe is stalled.        
    else return INVALID;     
}

//Use Instruction varibale from Pipeline class itself.
void Pipeline::Fetch()
{   
    int check = 0;
    //Set cycle timing info for instructions.
    for(int i=0;i<width;i++)
    {
        Instruction[i].startFE = currentCycle;        
    }

    //Clear retireDest1 and retireDest2
    if ( currentCycle % 2 == 1) //ODD cycle
    {
        //Clear RD2
        for ( int i=0; i<width*5; i++)
        {
            retireDest2[i] = -2;
        }
    }
    else
    //EVEN cycle ; CLear RD1
    {
        for( int i=0; i<width*5; i++)
        {
            retireDest1[i] = -2;
        }

    }

    //cout << "==================== FETCH =======================================" << endl;
    //printStage(Instruction);
    
    //Forward to DE stage if DE is empty.
    if ( DE->DEStatus() == EMPTY )
    {
        stall = 0;
        //Duration in FE
        for ( int i=0; i<width; i++)
        {
            Instruction[i].startDE = currentCycle + 1;
            Instruction[i].durationFE = currentCycle + 1 - Instruction[i].startFE;
        }
        //cout << "Sending to DE" << '\n' ;
        for ( int k=0; k<width; k++)
        {
            if ( Instruction[k].seqNo == -1 )
                check = check + 1;
        }
        if ( check == 0 )
            { DE->sendDE(Instruction); }
        else if ( check < width)
            { DE->sendDE(Instruction); }
        else if ( check == width)
            {} //Dont send to DE. 
    }
    else if ( DE->DEStatus() == FULL && currentCycle > 2)
    {
        stall = 1;
    }
    else stall = 0;

    return;
}

void Pipeline::Decode()
{

  if ( DE->DEState() == VALID)
  {
    //cout << "==================== DECODE =======================================" << endl;
    //printStage( DE->DEInstruction );

    if ( DE->DEStatus() == FULL)       //Check if DE has an instruction bundle to forward.
    {
        RN->valid = VALID;
        if ( RN->RNStatus() == EMPTY && RN->RNState() == VALID)     //Check if RN stage is empty and valid.
        {
            //Set cycle timing info for instructions.
            for(int i=0;i<width;i++)
            {
                DE->DEInstruction[i].startRN = currentCycle + 1;  
                DE->DEInstruction[i].durationDE = currentCycle + 1 - DE->DEInstruction[i].startDE;      
            } 

            RN->sendRN(DE->DEInstruction);
            DE->status = EMPTY;
        }
    }
  
  }
}

void Pipeline::Rename()
{
    if ( RN->RNState() == VALID)
    {
    //cout << "==================== RENAME =======================================" << endl;
    //printStage(RN->Instruction);

    if (RN->RNStatus() == FULL)
    {
        //cout << "RN is full " << endl;
        //Check for space in RR stage, space in ROB for WIDTH number of entries.
        if ( RR->RRStatus() == EMPTY && ROB->ROBStatus(width) == EMPTY )
        {
            //cout << "In send to RR " << endl;
            //***********************************************************************************************************************************
            //Rename stage processing
            //***********************************************************************************************************************************

            for(int i=0; i<width; i++)
            {
                robBlock *entry;
                entry = new robBlock;
                
                //1. Allocate entry in ROB.
                entry = ROB->generateROBEntry(RN->Instruction, i);
                ROB->enqueue(entry);
                
                // ROB->show();
                // cout << '\n';

                if ( RN->Instruction[i].srcReg1 != -1 )
                {
                    if ( RMT[RN->Instruction[i].rs1].RMTValid == VALID )
                    {
                        //Rename srcReg1
                        RN->Instruction[i].srcReg1 = RMT[RN->Instruction[i].rs1].tag;
                        RN->Instruction[i].renamed1 = VALID;
                    }
                    else
                    {
                        RN->Instruction[i].src1Ready = READY;
                    }
                }
                else
                {
                    RN->Instruction[i].src1Ready = READY;
                }

                if ( RN->Instruction[i].srcReg2 != -1 )
                {
                    if ( RMT[RN->Instruction[i].rs2].RMTValid == VALID )
                    {
                        //Rename srcReg2
                        RN->Instruction[i].srcReg2 = RMT[RN->Instruction[i].rs2].tag;
                        RN->Instruction[i].renamed2 = VALID;
                    }
                    else
                    {
                        RN->Instruction[i].src2Ready = READY;
                    }
                }
                else
                {
                    RN->Instruction[i].src2Ready= READY;
                }


                //Allocate entry in RMT.
                if ( RN->Instruction[i].destReg != -1 )
                {
                    RMT[RN->Instruction[i].dest].tag = ROB->back;
                    RMT[RN->Instruction[i].dest].RMTValid = VALID;
                    RN->Instruction[i].robTag = ROB->back;
                }
                else
                {
                    RN->Instruction[i].robTag = ROB->back;  
                }
                        
                //3. Rename destination registers.
                if ( RN->Instruction[i].destReg != -1 )
                {
                    if ( RMT[RN->Instruction[i].dest].RMTValid == VALID)
                    {
                        //Get name from RMTs
                       RN->Instruction[i].destReg = RMT[RN->Instruction[i].dest].tag;
                    }
                }

                //Setting ready portion.
    
                //From ROB
                //For rs1
                if ( ROB->ROB[RN->Instruction[i].srcReg1].ready == READY)
                {
                    RN->Instruction[i].src1Ready = READY;
                }

                //For rs2
                if ( ROB->ROB[RN->Instruction[i].srcReg2].ready == READY)
                {
                    RN->Instruction[i].src2Ready = READY;
                }

            }

            //ROB->show();
            //cout << "Front " << ROB->front << " Back " << ROB->back << endl;
            //cout << '\n';

            //Send bundle to RegRead Stage.
            //Set cycle timing info for instructions.

            //Print RMT state
            /*//DEBUG
            for ( int k=0; k<67; k++)
            {
                cout << "VALID[" << k << "] " << RMT[k].RMTValid << " " << RMT[k].tag << '\n';
            }*/

            for(int i=0; i<width; i++)
            {
                RN->Instruction[i].startRR = currentCycle + 1;  
                RN->Instruction[i].durationRN = currentCycle + 1 - RN->Instruction[i].startRN;      
            } 

            RR->sendRR(RN->Instruction);
            RN->status = EMPTY;
            //cout << "Send RR " << endl;
        }
    }
    }
}


void Pipeline::RegRead()
{

if ( RR->RRState() == VALID)
{

    //cout << "==================== REGREAD =======================================" << endl;
    //printStage(RR->Instruction);

    if ( RR->RRStatus() == FULL)
    {
        //***********************************************************************************************************************************
        //RegRead stage processing
        //***********************************************************************************************************************************
        for(int i =0; i<width; i++)
        { 
            //Readiness of source register operands

            //For rs1
            if ( RR->Instruction[i].srcReg1 == -1 )
            {
                RR->Instruction[i].src1Ready = READY;
            }
            else
            {
                /*if ( RMT[RR->Instruction[i].rs1].RMTValid == INVALID)
                {
                    RR->Instruction[i].src1Ready = READY;
                }*/

                if ( (ROB->ROB[ RR->Instruction[i].srcReg1 ].ready == READY) )
                {
                    RR->Instruction[i].src1Ready = READY;
                }
           
                //Wakeup from RT stage.
                switch(flip)
                {
                    case -1:
                    for ( int k=0; k<width*5; k++)
                    {                        
                        if ( RR->Instruction[i].srcReg1 == retireDest1[k] )
                        {
                            RR->Instruction[i].src1Ready = READY;                                                    
                        }   
                    }                     
                    break;
                    case 1:
                    for ( int k=0; k<width*5; k++)
                    {                        
                        if ( RR->Instruction[i].srcReg1 == retireDest2[k] )
                        {
                            RR->Instruction[i].src1Ready = READY;                              
                        }   
                    }                              
                    break;
                }

                //Check for wakeup from Execute stage.
                for ( int j=0; j<width; j++)
                {
                    for ( int k=0; k<5; k++)
                    {
                        if ( EX[j]->wakeup[k] == RR->Instruction[i].srcReg1 ) 
                        {
                            RR->Instruction[i].src1Ready = READY;
                        }  
                    }                 
                } 

                //From WB stage;
                for ( int k=0; k<width*5; k++)
                {
                    if ( wbDest[k] == RR->Instruction[i].srcReg1 )
                    {
                        RR->Instruction[i].src1Ready = READY;
                    }
                }

                //From ROB
                if ( ROB->ROB[RR->Instruction[i].srcReg1].ready == READY)
                {
                    RR->Instruction[i].src1Ready = READY;
                }
                   
            }

            //For rs2
            if ( RR->Instruction[i].srcReg2 == -1 )
            {
                RR->Instruction[i].src2Ready = READY; 
            }
            else
            {
                /*if ( RMT[RR->Instruction[i].rs2].RMTValid == INVALID)
                {
                    RR->Instruction[i].src2Ready = READY;
                }*/

                if ( (ROB->ROB[ RR->Instruction[i].srcReg2 ].ready == READY) )
                {
                    RR->Instruction[i].src2Ready = READY;
                }

                //Wakeup from RT stage.
                switch(flip)
                {
                    case 1:
                    for ( int k=0; k<width*5; k++)
                    {                        
                        if ( RR->Instruction[i].srcReg2 == retireDest1[k] )
                        {
                            RR->Instruction[i].src2Ready = READY;                              
                        }   
                    }                     
                    break;
                    case -1:
                    for ( int k=0; k<width*5; k++)
                    {                        
                        if ( RR->Instruction[i].srcReg2 == retireDest2[k] )
                        {
                            RR->Instruction[i].src2Ready = READY;                          
                        }   
                    }                              
                    break;
                }

                //Check for wakeup from Execute stage.
                for ( int j=0; j<width; j++)
                {
                    for ( int k=0; k<5; k++)
                    {
                        if ( EX[j]->wakeup[k] == RR->Instruction[i].srcReg2 ) 
                        {
                            RR->Instruction[i].src2Ready = READY;
                        }  
                    }                     
                }

                //From WB stage;
                for ( int k=0; k<width*5; k++)
                {
                    if ( wbDest[k] == RR->Instruction[i].srcReg2 )
                    {
                        RR->Instruction[i].src2Ready = READY;
                    }
                }

            }  

        } // For ends.

    } //RRStatus ends.   

    if ( DI->DIStatus() == EMPTY || DI->DIState() == INVALID )
    {
        if (RR->RRStatus() == FULL )
        {
          //Send instruction bundle to DI
            for(int i=0; i<width; i++)
            {
                RR->Instruction[i].startDI = currentCycle + 1;  
                RR->Instruction[i].durationRR = currentCycle + 1 - RR->Instruction[i].startRR;      
            } 

            DI->sendDI(RR->Instruction);
            RR->status = EMPTY;
            //cout << "RR empty " << endl;
        }    
    } //DIStatus ends
    
}//RRState end.

}

void Pipeline::Dispatch()
{

if ( DI->DIState() == VALID)
{
    //cout << "==================== DISPATCH =======================================" << endl;
    //printStage(DI->Instruction);

    if ( DI->DIStatus() == FULL)
    {

        //Wakeup instructions from execute stage.
        for ( int i=0; i<width; i++)
        {
            //For rs1
            if ( DI->Instruction[i].srcReg1 != -1)
            {
                for ( int j=0; j<width; j++)
                {
                    for ( int k=0; k<5; k++)
                    {
                        if ( EX[j]->wakeup[k] == DI->Instruction[i].srcReg1 ) 
                        {
                            DI->Instruction[i].src1Ready = READY;
                        }  
                    }                 
                }                
            }

            //For rs2
            if ( DI->Instruction[i].srcReg2 != -1)
            {
                for ( int j=0; j<width; j++)
                {
                    for ( int k=0; k<5; k++)
                    {
                        if ( EX[j]->wakeup[k] == DI->Instruction[i].srcReg2 ) 
                        {
                            DI->Instruction[i].src2Ready = READY;
                        }  
                    }                 
                }                
            }
          

            //Check ROB as well
            if ( ROB->ROB[DI->Instruction[i].srcReg1].ready == READY)
            {
                DI->Instruction[i].src1Ready = READY;
            }

            if ( ROB->ROB[DI->Instruction[i].srcReg2].ready == READY)
            {
                DI->Instruction[i].src2Ready = READY;
            }

        }


        //From WB
        //For rs1
        for ( int i=0; i<width; i++)
        {       
            for ( int k=0; k<width*5; k++)
            {
                if ( wbDest[k] == DI->Instruction[i].srcReg1 )
                {
                    DI->Instruction[i].src1Ready = READY;
                }
            }
        }

        //For rs2
        for ( int i=0; i<width; i++)
        {       
            for ( int k=0; k<width*5; k++)
            {
                if ( wbDest[k] == DI->Instruction[i].srcReg2 )
                {
                    DI->Instruction[i].src2Ready = READY;
                }
            }
        }  


        //Check number of free entries in IQ before sending.
        if ( IQ->IQStatus(width) == EMPTY )
        {
            //cout << "IQ is empty" << endl;
            //Set cycle timing info for instructions.
            for(int i=0;i<width;i++)
            {
                DI->Instruction[i].startIS = currentCycle + 1;  
                DI->Instruction[i].durationDI = currentCycle + 1 - DI->Instruction[i].startDI;      
            }

            IQ->sendIQ(DI->Instruction);
            DI->status = EMPTY;
            DI->valid = INVALID;
            //cout << "DI empty " << endl;
        }
    }
}

}

void Pipeline::Issue()
{
    if ( IQ->IQState() == VALID )
    {
  
    //cout << "==================== ISSUE QUEUE =======================================" << endl;
    /*for ( int i=0; i<IQ->size; i++)
    {
        cout << "Age = " << IQ->IQ[i].age << endl;
    }*/

    //IQ->sortIssueQueue();

    /*for ( int i=0; i<IQ->size; i++)
    {
        cout << "SAge = " << IQ->IQ[i].age << endl;
    }
    cout << '\n' ;*/

   //Wakeup from execute stage

    //For src1Ready
    for ( int i=0; i<IQ->size; i++)
    {
        if ( IQ->IQ[i].valid == VALID)
        {
            for ( int j=0; j<width; j++)
            {
                for ( int k=0; k<5; k++)
                {
                    if ( (EX[j]->wakeup[k] == IQ->IQ[i].Instr.srcReg1) || (IQ->IQ[i].Instr.srcReg1 == -1) )
                    {
                        IQ->IQ[i].Instr.src1Ready = READY;
                    }
                }
            }
        }
    }

    //For src2Ready
    for ( int i=0; i<IQ->size; i++)
    {
        if ( IQ->IQ[i].valid == VALID)
        {
            for ( int j=0; j<width; j++)
            {
                for ( int k=0; k<5; k++)
                {
                    if ( (EX[j]->wakeup[k] == IQ->IQ[i].Instr.srcReg2) || (IQ->IQ[i].Instr.srcReg2 == -1) )
                    {
                        IQ->IQ[i].Instr.src2Ready = READY;                         
                    }
                }
            }
        }
    }  

    //ROB check
    //For rs1
    for ( int i=0; i<IQ->size; i++)
    {
        if ( IQ->IQ[i].valid == VALID)
        {
            if ( ROB->ROB[IQ->IQ[i].Instr.srcReg1].ready == READY )
            {
                IQ->IQ[i].Instr.src1Ready = READY; 
            }
        }
    } 

    //For rs2
    for ( int i=0; i<IQ->size; i++)
    {
        if ( IQ->IQ[i].valid == VALID)
        {
            if ( ROB->ROB[IQ->IQ[i].Instr.srcReg2].ready == READY )
            {
                IQ->IQ[i].Instr.src2Ready = READY; 
            }  
        }
    }  

    //From WB
    //For rs1
    for ( int i=0; i<IQ->size; i++)
    {       
        for ( int k=0; k<width*5; k++)
        {
            if ( wbDest[k] == IQ->IQ[i].Instr.srcReg1 )
            {
                IQ->IQ[i].Instr.src1Ready = READY;
            }
        }
    }

    //For rs2
    for ( int i=0; i<IQ->size; i++)
    {       
        for ( int k=0; k<width*5; k++)
        {
            if ( wbDest[k] == IQ->IQ[i].Instr.srcReg2 )
            {
                IQ->IQ[i].Instr.src2Ready = READY;
            }
        }
    }

    /*for ( int i=0; i<IQ->size; i++)
    {
        if ( IQ->IQ[i].valid == VALID)
        {
            if ( IQ->IQ[i].Instr.seqNo == 3358 )
            {
                cout << "src1Ready " << IQ->IQ[i].Instr.src1Ready << endl;
                cout << "src2Ready " << IQ->IQ[i].Instr.src2Ready << endl;
            }
        }
    }*/     

        inst * Instruction1;
        Instruction1 = new inst[width];

        Instruction1 = IQ->getIssueBundle(width);
        //cout << "IssueNum " << IQ->issueNum << " send EX" <<  endl;
        for( int i=0; i<IQ->issueNum; i++)
        {
            Instruction1[i].startEX = currentCycle + 1;
            Instruction1[i].durationIS = currentCycle + 1 - Instruction1[i].startIS;
            EX[i]->sendEX(&Instruction1[i]);
        }

    }
}

void Pipeline::Execute()
{
    //Can have width*5 instructions in-flight.
    
    //Can writeback maximum width*5 instructions at a time.
    //Remove instructions which are finishing this cycle.
    //Also decrement counters for instructions currently in execute stage.
    //If counter-1=0, send to WB stage.
    
if ( EX[0]->EXValid == VALID)
{

    //cout << "==================== EXECUTE =======================================" << endl;
    
    /*//Print all 5 stages
    //DEBUG
    for ( int k=0; k<width; k++)
    {
        cout << "EX[" << k << "]" << '\n';
        for ( int i=0; i<5; i++)
        {
            cout << "Valid[" << i << "] " << EX[k]->valid[i] << '\n';
        }
    }*/
    

    for ( int k=0; k<width; k++)
    {
        //Check if there is instruction in that execute_list stage first, i.e. valid bit!!
        for( int i=0; i<5; i++)
        {

            if ( EX[k]->EXValid == VALID)
            {

                if ( EX[k]->valid[i] == VALID)
                { 
                    EX[k]->Instruction[i].counter--;
                    if ( EX[k]->Instruction[i].counter == 0)     //Send to WB
                    {
                        EX[k]->valid[i] = INVALID;
                        EX[k]->wakeup[i] = EX[k]->Instruction[i].destReg;
                        
                        EX[k]->Instruction[i].startWB = currentCycle + 1;
                        EX[k]->Instruction[i].durationEX = currentCycle + 1 - EX[k]->Instruction[i].startEX;   
                        WB->sendWB( &(EX[k]->Instruction[i]) ); 

                        //Also send over bypass to wakeup instructions in RR, DI and IQ.
                        //Check wakeup array in RR, DI and IQ.
                        //Max width*5 instruction can be woken up.
                   
                    }
                    else EX[k]->wakeup[i] = -2;
                } 
                else EX[k]->wakeup[i] = -2;

            }

        }
    }

}

}

void Pipeline::Writeback()
{
    flip = flip * (-1);

    if ( WB->WBState() == VALID)
    {

    //cout << "==================== WRITEBACK =======================================" << endl;
    //printWB(WB->Instruction);


    for (int i=0; i<width*5; i++)
    {
        if ( WB->valid[i] == VALID)
        {
            WB->valid[i] = INVALID;
            WB->Instruction[i].startRT = currentCycle + 1;
            WB->Instruction[i].durationWB = currentCycle + 1 - WB->Instruction[i].startWB;
            if ( flip == 1)
            {                
                retireDest1[i] = WB->Instruction[i].destReg;
            }

            if ( flip == -1)
            {
                retireDest2[i] = WB->Instruction[i].destReg;
            }

            wbDest[i] = WB->Instruction[i].destReg;

            ROB->robValid = VALID;
            //Mark instruction as ready in ROB.
            ROB->ROB[ WB->Instruction[i].robTag ].valid = VALID;
            ROB->ROB[ WB->Instruction[i].robTag ].ready = READY;
            ROB->ROB[ WB->Instruction[i].robTag].Instr = WB->Instruction[i];
        }
        else
            wbDest[i] = -2;
    }
    
    // ROB->show();
    // cout << "Front " << ROB->front << " Back " << ROB->back << endl;
    // cout << '\n';

    }

    /*
    //Print retireDest
    for ( int i=0; i<width*5; i++)
    {
        cout << "RetDest1 " << i << " = " << retireDest1[i] <<  '\n';
    }
    for ( int i=0; i<width*5; i++)
    {
        cout << "RetDest-1 " << i << " = " << retireDest2[i] <<  '\n';
    }*/    
}

void Pipeline::Retire()
{
    int retireCount = 0;
         
    if ( ROB->robValid == VALID )
    {          
        //Retire 'width' ready instructions from head of the ROB
        //All ready instruction?! Maximum will be 'wdith' in number.
    //cout << "==================== RETIRE =======================================" << endl;   
    //ROB->show();
    //cout << "Front =" << ROB->front << endl;

        for ( int i=0; i<ROB->size; i++)
        {
            if ( ROB->ROB[ROB->front].valid == VALID && ROB->ROB[ROB->front].ready == READY )
            {
                //Then retire.
                ROB->ROB[ROB->front].Instr.durationRT = currentCycle + 1 - ROB->ROB[ROB->front].Instr.startRT;
                retireCount++;

                //RESET entry in RMT INVALID if it has latest tag!
                if ( ROB->ROB[ROB->front].Instr.robTag == RMT[ ROB->ROB[ROB->front].Instr.dest ].tag )
                {
                    RMT[ ROB->ROB[ROB->front].Instr.dest ].RMTValid = INVALID;
                }

                if ( ROB->ROB[ROB->front].Instr.seqNo != -1 )
                {
                    //Print stats
                    cout 
                    << ROB->ROB[ROB->front].Instr.seqNo << " "
                    << "fu{" << ROB->ROB[ROB->front].Instr.type << "} "
                    << "src{" << ROB->ROB[ROB->front].Instr.rs1 << "," << ROB->ROB[ROB->front].Instr.rs2 << "} "
                    << "dst{" << ROB->ROB[ROB->front].Instr.dest << "} "
                    << "FE{" << ROB->ROB[ROB->front].Instr.startFE << "," << ROB->ROB[ROB->front].Instr.durationFE << "} "
                    << "DE{" << ROB->ROB[ROB->front].Instr.startDE << "," << ROB->ROB[ROB->front].Instr.durationDE << "} "
                    << "RN{" << ROB->ROB[ROB->front].Instr.startRN << "," << ROB->ROB[ROB->front].Instr.durationRN << "} "
                    << "RR{" << ROB->ROB[ROB->front].Instr.startRR << "," << ROB->ROB[ROB->front].Instr.durationRR << "} "
                    << "DI{" << ROB->ROB[ROB->front].Instr.startDI << "," << ROB->ROB[ROB->front].Instr.durationDI << "} "
                    << "IS{" << ROB->ROB[ROB->front].Instr.startIS << "," << ROB->ROB[ROB->front].Instr.durationIS << "} "
                    << "EX{" << ROB->ROB[ROB->front].Instr.startEX << "," << ROB->ROB[ROB->front].Instr.durationEX << "} "
                    << "WB{" << ROB->ROB[ROB->front].Instr.startWB << "," << ROB->ROB[ROB->front].Instr.durationWB << "} "
                    << "RT{" << ROB->ROB[ROB->front].Instr.startRT << "," << ROB->ROB[ROB->front].Instr.durationRT << "} "
                    << endl;

                    if ( ROB->ROB[ROB->front].Instr.seqNo == 9999 )
                    {
                        totalCount = ROB->ROB[ROB->front].Instr.startRT + ROB->ROB[ROB->front].Instr.durationRT;
                    }
                }
                       
                ROB->dequeue();
                
            }          

            if ( retireCount == width )
                break;
        }
    }
}

void Pipeline::printStage(inst *Instruc)
{

    for( int i=0; i<width; i++)
    {
        cout 
        << Instruc[i].seqNo << " "
        << "src{" << Instruc[i].srcReg1 << "," << Instruc[i].srcReg2 << "} "
        << "dst{" << Instruc[i].destReg << "} "
        << "FE{" << Instruc[i].startFE << "," << Instruc[i].durationFE << "} "
        << "DE{" << Instruc[i].startDE << "," << Instruc[i].durationDE << "} "
        << "RN{" << Instruc[i].startRN << "," << Instruc[i].durationRN << "} "
        << "RR{" << Instruc[i].startRR << "," << Instruc[i].durationRR << "} "
        << "DI{" << Instruc[i].startDI << "," << Instruc[i].durationDI << "} "
        << "IS{" << Instruc[i].startIS << "," << Instruc[i].durationIS << "} "
        << "EX{" << Instruc[i].startEX << "," << Instruc[i].durationEX << "} "
        << "WB{" << Instruc[i].startWB << "," << Instruc[i].durationWB << "} "
        << "RT{" << Instruc[i].startRT << "," << Instruc[i].durationRT << "} "
        << endl;
    }
}

void Pipeline::printWB(inst *Instruc)
{

    for( int i=0; i<width*5; i++)
    {
        if ( WB->valid[i] == VALID)
        {
            cout 
            << Instruc[i].seqNo << " "
            << "src{" << Instruc[i].srcReg1 << "," << Instruc[i].srcReg2 << "} "
            << "dst{" << Instruc[i].destReg << "} "
            << "FE{" << Instruc[i].startFE << "," << Instruc[i].durationFE << "} "
            << "DE{" << Instruc[i].startDE << "," << Instruc[i].durationDE << "} "
            << "RN{" << Instruc[i].startRN << "," << Instruc[i].durationRN << "} "
            << "RR{" << Instruc[i].startRR << "," << Instruc[i].durationRR << "} "
            << "DI{" << Instruc[i].startDI << "," << Instruc[i].durationDI << "} "
            << "IS{" << Instruc[i].startIS << "," << Instruc[i].durationIS << "} "
            << "EX{" << Instruc[i].startEX << "," << Instruc[i].durationEX << "} "
            << "WB{" << Instruc[i].startWB << "," << Instruc[i].durationWB << "} "
            << "RT{" << Instruc[i].startRT << "," << Instruc[i].durationRT << "} "
            << endl;
        }
    }
}

