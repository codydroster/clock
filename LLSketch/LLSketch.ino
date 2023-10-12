#include <AlPlc_PMC.h>

/* pmc_1.0.4
      - Arduino_MachineControl (1.1.1)
*/

struct PLCSharedVarsInput_t
{
	int16_t XPosAct;
	int16_t YPosAct;
	int16_t ZPosAct;
};
PLCSharedVarsInput_t& PLCIn = (PLCSharedVarsInput_t&)m_PLCSharedVarsInputBuf;

struct PLCSharedVarsOutput_t
{
	bool XDir;
	int16_t XSpeed;
	int16_t XPos;
	bool YDir;
	int16_t YSpeed;
	int16_t YPos;
	bool ZDir;
	int16_t ZSpeed;
	int16_t ZPos;
	bool Home;
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(36529155);

// shared variables can be accessed with PLCIn.varname and PLCOut.varname

#include <Arduino_MachineControl.h>

#define XDIR_PIN 0
#define YDIR_PIN 2
#define ZDIR_PIN 4

#define XPUL_PIN 1
#define YPUL_PIN 3
#define ZPUL_PIN 5


using namespace machinecontrol;

unsigned long previousMillisX = 0;
unsigned long previousMillisY = 0;
unsigned long previousMillisZ = 0;

const int homingSpd = 250;
const int homingSpdSlow = 1250;
const int homingSpdFast = 250;
const int awayFromHomeShort = 1500;
const int awayFromHomeLong = 4000;
const int awayFromHome = 4000;


int XPos;
int YPos;
int ZPos;

int XMax = 100;
int YMax = 100;
int ZMax = 100;
bool homed = 0;


void setup()
{
    

	AxelPLC.Run();
}

void loop()
{   

    unsigned long currentMillis = micros();

    //regular move
    if(PLCOut.Home != true) {
    
        
    MovePosX(currentMillis);
    MovePosY(currentMillis);
    MovePosZ(currentMillis);
    
    }
    //homing
    if(PLCOut.Home == true) {

        
        //Z home
        HomeZ();
        //X home
        HomeX();
        //Y home
        HomeY();        
        

        


    }
        
    PLCIn.XPosAct = XPos;
    PLCIn.YPosAct = YPos;
    PLCIn.ZPosAct = ZPos;

    PulseLowX(currentMillis);
    PulseLowY(currentMillis);
    PulseLowZ(currentMillis);
    
}



void HomeX() {

 
    if(digital_inputs.read(DIN_READ_CH_PIN_00) == true) {

        //get away from home prox before homing sequence. Use delay since we're in a loop.
            //Positive Dir
         for(int i = 0; i < awayFromHomeShort; i++) {
            digital_outputs.set(XDIR_PIN, LOW);
            MoveFast(XPUL_PIN);
            }
        //Move negative until we reach home prox
        while(digital_inputs.read(DIN_READ_CH_PIN_00) != true) {
            digital_outputs.set(XDIR_PIN, HIGH);
            MoveSlow(XPUL_PIN);
        }
        //Set position to 0 once prox activated
        XPos = 0;
    }
    
    
    if(digital_inputs.read(DIN_READ_CH_PIN_00) != true) {
        //Move towards home, negative
        while(digital_inputs.read(DIN_READ_CH_PIN_00) != true) {
            digital_outputs.set(XDIR_PIN, HIGH);
            MoveFast(XPUL_PIN);
        }
        //Move away from home, positive
        for(int i = 0; i < awayFromHomeShort; i++) {
            digital_outputs.set(XDIR_PIN, LOW);
            MoveFast(XPUL_PIN);
            }
        // towards home, negative
        while(digital_inputs.read(DIN_READ_CH_PIN_00) != true) {
            digital_outputs.set(XDIR_PIN, HIGH);
            MoveSlow(XPUL_PIN);
        }
        XPos = 0;
       
    }
}


void HomeY() {

 
     if(digital_inputs.read(DIN_READ_CH_PIN_01) == true) {
        
        //get away from home prox before homing sequence. Use delay since we're in a loop.
            //Positive Dir
         for(int i = 0; i < awayFromHomeShort; i++) {
            digital_outputs.set(YDIR_PIN, LOW);
            MoveFast(YPUL_PIN);
            
            }
        //Move negative until we reach home prox

        while(digital_inputs.read(DIN_READ_CH_PIN_01) != true) {
            digital_outputs.set(YDIR_PIN, HIGH);
            MoveSlow(YPUL_PIN);
        }
        //Set position to 0 once prox activated
        YPos = 0;
    }
    
    
    if(digital_inputs.read(DIN_READ_CH_PIN_01) != true) {
        //Move towards home, negative
        while(digital_inputs.read(DIN_READ_CH_PIN_01) != true) {
            digital_outputs.set(YDIR_PIN, HIGH);
            MoveFast(YPUL_PIN);
        }
        

        //Move away from home, positive
        for(int i = 0; i < awayFromHomeShort; i++) {
            digital_outputs.set(YDIR_PIN, LOW);
            MoveFast(YPUL_PIN);
            
            }
        // towards home, negative
        while(digital_inputs.read(DIN_READ_CH_PIN_01) != true) {
            digital_outputs.set(YDIR_PIN, HIGH);
            MoveSlow(YPUL_PIN);
        }
        YPos = 0;
       
    }
    }




void HomeZ() {

 
 if(digital_inputs.read(DIN_READ_CH_PIN_02) == true) {
        //get away from home prox before homing sequence. Use delay since we're in a loop.
            //Positive Dir
        for(int i = 0; i < awayFromHomeShort; i++) {
            digital_outputs.set(ZDIR_PIN, LOW);
            MoveFast(ZPUL_PIN);
            }
            
        //Move negative until we reach home prox

        while(digital_inputs.read(DIN_READ_CH_PIN_02) != true) {
            digital_outputs.set(ZDIR_PIN, HIGH);
            MoveSlow(ZPUL_PIN);
        }
        //Set position to 0 once prox activated
        ZPos = 0;
    }
    
    
    if(digital_inputs.read(DIN_READ_CH_PIN_02) != true) {
        //Move towards home, negative
        while(!digital_inputs.read(DIN_READ_CH_PIN_02)) {
            digital_outputs.set(ZDIR_PIN, HIGH);
            MoveFast(ZPUL_PIN);
        }

        //Move away from home, positive
        for(int i = 0; i < awayFromHomeShort; i++) {
            digital_outputs.set(ZDIR_PIN, LOW);
            MoveFast(ZPUL_PIN);
            
            }
        // towards home, negative
        while(digital_inputs.read(DIN_READ_CH_PIN_02) != true) {
            digital_outputs.set(ZDIR_PIN, HIGH);
            MoveSlow(ZPUL_PIN);
        }
        ZPos = 0;
       
    }
        //Clearance
        while(ZPos < 12000) {
            digital_outputs.set(ZDIR_PIN, LOW);
            MoveFast(ZPUL_PIN);
            ZPos++;

        }
        
    }



void MovePosX(unsigned long currentMillis) {
     if (currentMillis - previousMillisX >= PLCOut.XSpeed) {
        // negative
        if(PLCOut.XPos < XPos) {
            digital_outputs.set(0, HIGH);
            digital_outputs.set(1, HIGH);
            XPos--;
        }
    
        if(PLCOut.XPos > XPos) {
            //Positive Direction
            digital_outputs.set(0, LOW);
            digital_outputs.set(1, HIGH);
            XPos++;
    }
        previousMillisX = currentMillis;
        
    }

}

void MovePosY(unsigned long currentMillis) {
     if (currentMillis - previousMillisY >= PLCOut.YSpeed) {
        // negative
        if(PLCOut.YPos < YPos) {
            digital_outputs.set(2, HIGH);
            digital_outputs.set(3, HIGH);
            YPos--;
        }
    
        if(PLCOut.YPos > YPos) {
            //Positive Direction
            digital_outputs.set(2, LOW);
            digital_outputs.set(3, HIGH);
            YPos++;
    }
        previousMillisY = currentMillis;
        
    }

}

void MovePosZ(unsigned long currentMillis) {
     if (currentMillis - previousMillisZ >= PLCOut.ZSpeed) {
        // negative
        if(ZPos > PLCOut.ZPos) {
            digital_outputs.set(4, HIGH);
            digital_outputs.set(5, HIGH);
            ZPos--;
        }
    
        if(ZPos < PLCOut.ZPos) {
            //Positive Direction
            digital_outputs.set(4, LOW);
            digital_outputs.set(5, HIGH);
            ZPos++;
    }
        previousMillisZ = currentMillis;
        
    }

}

void MoveSlow(int pul) {
    digital_outputs.set(pul, HIGH);
    delayMicroseconds(homingSpdSlow);
    digital_outputs.set(pul, LOW);
    delayMicroseconds(homingSpdSlow);
}

void MoveFast(int pul) {
    digital_outputs.set(pul, HIGH);
    delayMicroseconds(homingSpdFast);
    digital_outputs.set(pul, LOW);
    delayMicroseconds(homingSpdFast);
}




void PulseLowX(unsigned long currentMillis) {
    if(currentMillis - previousMillisX > 40) {
        digital_outputs.set(1, LOW);

    }
}

void PulseLowY(unsigned long currentMillis) {
    if(currentMillis - previousMillisY > 40) {
        digital_outputs.set(3, LOW);

    }
}

void PulseLowZ(unsigned long currentMillis) {
    if(currentMillis - previousMillisZ > 40) {
        digital_outputs.set(5, LOW);

    }
}




//
