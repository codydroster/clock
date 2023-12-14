#include <Arduino.h>
#line 1 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
#include <AlPlc_PMC.h>

/* pmc_1.0.4
      - Arduino_MachineControl (1.1.1)
*/

struct PLCSharedVarsInput_t
{
	int16_t XPosAct;
	int16_t YPosAct;
	int16_t ZPosAct;
	int16_t hour;
	int16_t minute;
	int16_t second;
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
	int16_t ADir;
	int16_t ASpeed;
	int16_t APos;
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(2120378239);

// shared variables can be accessed with PLCIn.varname and PLCOut.varname

#include <Arduino_MachineControl.h>


#define XDIR_PIN 0
#define YDIR_PIN 2
#define ZDIR_PIN 4

#define XPUL_PIN 1
#define YPUL_PIN 3
#define ZPUL_PIN 5
#define APUL_PIN 7
#define ACCELDEF 1500


using namespace machinecontrol;

unsigned long previousMicrosX = 0;
unsigned long previousMicrosY = 0;
unsigned long previousMicrosZ = 0;
unsigned long previousMicrosA = 0;

int seconds = 0;
int minutes = 0;
int hours = 0;
bool alarm_flag = false;

const int homingSpd = 250;
const int homingSpdSlow = 1250;
const int homingSpdFast = 250;
const int awayFromHomeShort = 1500;
const int awayFromHomeLong = 4000;
const int awayFromHome = 4000;



int accelX = 1500;
int accelY = 1500;
int accelZ = 1500;

int accelDistXY = 500;

int XPos;
int YPos;
int ZPos;
int APos;

//not implemented
int XMax = 100;
int YMax = 100;
int ZMax = 100;
bool homed = 0;


#line 94 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void setup();
#line 112 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void loop();
#line 169 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void callback_alarm();
#line 174 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void HomeX();
#line 227 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void HomeY();
#line 280 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void HomeZ();
#line 334 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void MovePosX(unsigned long currentMicros);
#line 380 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void MovePosY(unsigned long currentMicros);
#line 420 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void MovePosZ(unsigned long currentMicros);
#line 460 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void MoveA(unsigned long currentMicros);
#line 487 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void MoveAtSpeed(int pul, int speed);
#line 496 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
bool ReadInputDebounce(int pin);
#line 505 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void PulseLowX(unsigned long currentMicros);
#line 512 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void PulseLowY(unsigned long currentMicros);
#line 519 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void PulseLowZ(unsigned long currentMicros);
#line 526 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void PulseLowA(unsigned long currentMicros);
#line 94 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
void setup()
{
    //digital_outputs.setRetry();
    rtc_controller.begin();
    rtc_controller.enableAlarm();

    minutes = PLCIn.minute = rtc_controller.getMinutes();
    hours = PLCIn.hour = rtc_controller.getHours();
    rtc_controller.setMinuteAlarm(minutes + 1);
    if(minutes == 59) {
        rtc_controller.setMinuteAlarm(0);
    }
    attachInterrupt(RTC_INT, callback_alarm, FALLING);


	AxelPLC.Run();
}

void loop()
{   
    if(alarm_flag) {
        
        detachInterrupt(RTC_INT);
        minutes = PLCIn.minute = rtc_controller.getMinutes();
        PLCIn.hour = rtc_controller.getHours();
        rtc_controller.setMinuteAlarm(minutes + 1);
        if(minutes == 59) {
            rtc_controller.setMinuteAlarm(0);
    }
        rtc_controller.clearAlarm();
        attachInterrupt(RTC_INT, callback_alarm, FALLING);
        alarm_flag = false;
    }


    unsigned long currentMicros = micros();
    
    //regular move
    if(PLCOut.Home != true) {
    
        
    MovePosX(currentMicros);
    MovePosY(currentMicros);
    MovePosZ(currentMicros);
    MoveA(currentMicros);
    
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

    PulseLowX(currentMicros);
    PulseLowY(currentMicros);
    PulseLowZ(currentMicros);
    PulseLowA(currentMicros);
    
    
    


}

void callback_alarm() {
    alarm_flag = true;
    
}

void HomeX() {

            if(ReadInputDebounce(DIN_READ_CH_PIN_00)) {
                //get away from home prox before homing sequence.
                //Positive Dir
                 for(int i = 0; i < awayFromHomeShort; i++) {
                    digital_outputs.set(XDIR_PIN, LOW);
                    MoveAtSpeed(XPUL_PIN, homingSpd);
                    }
                    //Move negative until we reach home prox
                    while(!ReadInputDebounce(DIN_READ_CH_PIN_00)) {
                        digital_outputs.set(XDIR_PIN, HIGH);
                        MoveAtSpeed(XPUL_PIN, homingSpdSlow);
                    }
                    //Set position to 0 once prox activated
                    XPos = 0;
            
                
            } else {

                //Move towards home, negative
                while(!ReadInputDebounce(DIN_READ_CH_PIN_00)) {
                    digital_outputs.set(XDIR_PIN, HIGH);
                    MoveAtSpeed(XPUL_PIN, homingSpd);
                }

                
                //Move away from home, positive
                for(int i = 0; i < awayFromHomeShort; i++) {
                    digital_outputs.set(XDIR_PIN, LOW);
                    MoveAtSpeed(XPUL_PIN, homingSpd);
                    }
                // towards home, negative
                while(!ReadInputDebounce(DIN_READ_CH_PIN_00)) {
                    digital_outputs.set(XDIR_PIN, HIGH);
                    MoveAtSpeed(XPUL_PIN, homingSpdSlow);
                }
                XPos = 0;
       
            }
            
            //Clearance
        while(XPos < 1000) {
            digital_outputs.set(XDIR_PIN, LOW);
            MoveAtSpeed(XPUL_PIN, homingSpdSlow);
            XPos++;

        }

}



void HomeY() {

            if(ReadInputDebounce(DIN_READ_CH_PIN_01)) {
                //get away from home prox before homing sequence. Use delay since we're in a loop.
                    //Positive Dir
                for(int i = 0; i < awayFromHomeShort; i++) {
                    digital_outputs.set(YDIR_PIN, LOW);
                    MoveAtSpeed(YPUL_PIN, homingSpd);
                    }
                //Move negative until we reach home prox
        
                while(!ReadInputDebounce(DIN_READ_CH_PIN_01)) {
                    digital_outputs.set(YDIR_PIN, HIGH);
                    MoveAtSpeed(YPUL_PIN, homingSpdSlow);
                }
                //Set position to 0 once prox activated
                YPos = 0;
                
            } else {

                while(!ReadInputDebounce(DIN_READ_CH_PIN_01)) {
                    digital_outputs.set(YDIR_PIN, HIGH);
                    MoveAtSpeed(YPUL_PIN, homingSpd);
                }
                
        
                //Move away from home, positive
                for(int i = 0; i < awayFromHomeShort; i++) {
                    digital_outputs.set(YDIR_PIN, LOW);
                    MoveAtSpeed(YPUL_PIN, homingSpd);
                    
                    }
                // towards home, negative
                while(!ReadInputDebounce(DIN_READ_CH_PIN_01)) {
                    digital_outputs.set(YDIR_PIN, HIGH);
                    MoveAtSpeed(YPUL_PIN, homingSpd);
                }
                YPos = 0;
       
    }
                //Clearance
        while(YPos < 5000) {
            digital_outputs.set(YDIR_PIN, LOW);
            MoveAtSpeed(YPUL_PIN, homingSpdSlow);
            YPos++;

        }
    
    }




void HomeZ() {

        if(ReadInputDebounce(DIN_READ_CH_PIN_02)) {
            //get away from home prox before homing sequence. Use delay since we're in a loop.
                //Positive Dir
            for(int i = 0; i < awayFromHomeShort; i++) {
                digital_outputs.set(ZDIR_PIN, LOW);
                MoveAtSpeed(ZPUL_PIN, homingSpd);
                }
                
            //Move negative until we reach home prox
    
            while(!ReadInputDebounce(DIN_READ_CH_PIN_02)) {
                digital_outputs.set(ZDIR_PIN, HIGH);
                MoveAtSpeed(ZPUL_PIN, homingSpdSlow);
            }
            //Set position to 0 once prox activated
            ZPos = 0;
            
        } else {
    
            //Move towards home, negative
            while(!ReadInputDebounce(DIN_READ_CH_PIN_02)) {
                digital_outputs.set(ZDIR_PIN, HIGH);
                MoveAtSpeed(ZPUL_PIN, homingSpd);
            }
        
            //Move away from home, positive
            for(int i = 0; i < awayFromHomeShort; i++) {
                digital_outputs.set(ZDIR_PIN, LOW);
                MoveAtSpeed(ZPUL_PIN, homingSpd);
                    
                }
            // towards home, negative
            while(!ReadInputDebounce(DIN_READ_CH_PIN_02)) {
                digital_outputs.set(ZDIR_PIN, HIGH);
                MoveAtSpeed(ZPUL_PIN, homingSpdSlow);
            }
            ZPos = 0;
        }
    
    
        //Clearance
        while(ZPos < 14500) {
            digital_outputs.set(ZDIR_PIN, LOW);
            MoveAtSpeed(ZPUL_PIN, homingSpd);
            ZPos++;

        }
        
    }



void MovePosX(unsigned long currentMicros) {
    if (currentMicros - previousMicrosX >= PLCOut.XSpeed + accelX) {
        // negative
        
           //100    <    200    
        if(PLCOut.XPos < XPos) {
            digital_outputs.set(0, HIGH);
            XPos--;
            digital_outputs.set(1, HIGH);
            
            //if not at decel distance
            if(accelX > 0 && (XPos - PLCOut.XPos) > accelDistXY ) {
                accelX-=2;
            } else {
                if(accelX < ACCELDEF) {
                    accelX++;
              
            }
        }
    }
     
            //200       100
        if(PLCOut.XPos > XPos) {
            //Positive Direction
            digital_outputs.set(0, LOW);
            XPos++;
            digital_outputs.set(1, HIGH);
            
            //if not at decel distance
            if(accelX > 0 && (PLCOut.XPos - XPos) > accelDistXY) {
                accelX-=2;
            } else {
                if(accelX < ACCELDEF) {
                    accelX++;
                }
            }
        }
    
        previousMicrosX = currentMicros;
        if(PLCOut.XPos == XPos) {
            accelX = ACCELDEF;
        }
    }

}

void MovePosY(unsigned long currentMicros) {
     if (currentMicros - previousMicrosY >= PLCOut.YSpeed + accelY) {
        // negative
        if(PLCOut.YPos < YPos) {
            digital_outputs.set(2, HIGH);
            YPos--;
            digital_outputs.set(3, HIGH);
            
            if(accelY > 0 && (YPos - PLCOut.YPos) > accelDistXY ) {
                accelY-=2;
            } else {
                if(accelY < ACCELDEF) {
                    accelY++;
                }
            }
        }
    
        if(PLCOut.YPos > YPos) {
            //Positive Direction
            digital_outputs.set(2, LOW);
            YPos++;
            digital_outputs.set(3, HIGH);
            
            if(accelY > 0 && (PLCOut.YPos - YPos) > accelDistXY) {
                accelY-=2;
            } else {
                if(accelY < ACCELDEF) {
                    accelY++;
                }
            }
    }
        previousMicrosY = currentMicros;
        if(PLCOut.YPos == YPos) {
            accelY = ACCELDEF;
        
    }

}
}

void MovePosZ(unsigned long currentMicros) {
     if (currentMicros - previousMicrosZ >= PLCOut.ZSpeed + accelZ) {
        // negative
        if(PLCOut.ZPos < ZPos) {
            digital_outputs.set(4, HIGH);
            ZPos--;
            digital_outputs.set(5, HIGH);
            
            if(accelZ > 0 && (ZPos - PLCOut.ZPos) > accelDistXY ) {
                accelZ-=2;
            } else {
                if(accelZ < ACCELDEF) {
                    accelZ++;
                }
            }
        }
    
        if(PLCOut.ZPos > ZPos) {
            //Positive Direction
            digital_outputs.set(4, LOW);
            ZPos++;
            digital_outputs.set(5, HIGH);
            
            if(accelZ > 0 && (PLCOut.ZPos - ZPos) > accelDistXY) {
                accelZ-=2;
            } else {
                if(accelZ < ACCELDEF) {
                    accelZ++;
                }
            }
    }
        previousMicrosZ = currentMicros;
        if(PLCOut.ZPos == ZPos) {
            accelZ = 1500;
        
    }

}
}

void MoveA(unsigned long currentMicros) {
    if (currentMicros - previousMicrosA >= PLCOut.ASpeed) {
        // negative
        if(PLCOut.APos < APos) {
            digital_outputs.set(6, HIGH);
            APos--;
            digital_outputs.set(7, HIGH);
            
        }
    
        if(PLCOut.APos > APos) {
            //Positive Direction
            digital_outputs.set(6, LOW);
            APos++;
            digital_outputs.set(7, HIGH);
            
    }
    //reset position to 0, APosOut not really needed
        if(PLCOut.APos == 0) {
            APos = 0;
        }  
        previousMicrosA = currentMicros;
        
    }

}

void MoveAtSpeed(int pul, int speed) {
    digital_outputs.set(pul, HIGH);
    delayMicroseconds(speed);
    digital_outputs.set(pul, LOW);
    delayMicroseconds(speed);
}



bool ReadInputDebounce(int pin) {
    bool firstRead = digital_inputs.read(pin);
    delay(2); // Wait for the specified debounce delay
    bool secondRead = digital_inputs.read(pin);
    return (firstRead && secondRead); // Return true only if both readings are true
}



void PulseLowX(unsigned long currentMicros) {
    if(currentMicros - previousMicrosX > 100) {
        digital_outputs.set(1, LOW);

    }
}

void PulseLowY(unsigned long currentMicros) {
    if(currentMicros - previousMicrosY > 100) {
        digital_outputs.set(3, LOW);

    }
}

void PulseLowZ(unsigned long currentMicros) {
    if(currentMicros - previousMicrosZ > 100) {
        digital_outputs.set(5, LOW);

    }
}

void PulseLowA(unsigned long currentMicros) {
    if(currentMicros - previousMicrosA > 100) {
        digital_outputs.set(7, LOW);

    }
}











//

