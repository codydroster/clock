# 1 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino"
# 2 "C:\\Users\\cdros\\Developer\\Arduino\\Projects\\clock\\LLSketch\\LLSketch.ino" 2

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


AlPlc AxelPLC(-477531564);

// shared variables can be accessed with PLCIn.varname and PLCOut.varname




#define XDIR_PIN 0
#define YDIR_PIN 2
#define ZDIR_PIN 4

#define XPUL_PIN 1
#define YPUL_PIN 3
#define ZPUL_PIN 5
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
    attachInterrupt(PB_9, callback_alarm, FALLING);


 AxelPLC.Run();
}

void loop()
{
    if(alarm_flag) {

        detachInterrupt(PB_9);
        minutes = PLCIn.minute = rtc_controller.getMinutes();
        PLCIn.hour = rtc_controller.getHours();
        rtc_controller.setMinuteAlarm(minutes + 1);
        if(minutes == 59) {
            rtc_controller.setMinuteAlarm(0);
    }
        rtc_controller.clearAlarm();
        attachInterrupt(PB_9, callback_alarm, FALLING);
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


    if(digital_inputs.read(DIN_READ_CH_PIN_00) == true) {
        delay(2);
        if(digital_inputs.read(DIN_READ_CH_PIN_00) == true) {
            delay(2);
            if(digital_inputs.read(DIN_READ_CH_PIN_00) == true) {
                //get away from home prox before homing sequence. Use delay since we're in a loop.
                //Positive Dir
                 for(int i = 0; i < awayFromHomeShort; i++) {
                    digital_outputs.set(0, LOW);
                    MoveFast(1);
                    }
                    //Move negative until we reach home prox
                    while(digital_inputs.read(DIN_READ_CH_PIN_00) != true) {
                        digital_outputs.set(0, HIGH);
                        MoveSlow(1);
                    }
                    //Set position to 0 once prox activated
                    XPos = 0;
    }
    }
    }


    //debounce?
    if(digital_inputs.read(DIN_READ_CH_PIN_00) != true) {
        delay(2);
        if(digital_inputs.read(DIN_READ_CH_PIN_00) != true) {
            delay(2);
            if(digital_inputs.read(DIN_READ_CH_PIN_00) != true) {
                //Move towards home, negative
                while(digital_inputs.read(DIN_READ_CH_PIN_00) != true) {
                    digital_outputs.set(0, HIGH);
                    MoveFast(1);
                }
                //Move away from home, positive
                for(int i = 0; i < awayFromHomeShort; i++) {
                    digital_outputs.set(0, LOW);
                    MoveFast(1);
                    }
                // towards home, negative
                while(digital_inputs.read(DIN_READ_CH_PIN_00) != true) {
                    digital_outputs.set(0, HIGH);
                    MoveSlow(1);
                }
                XPos = 0;

    }
    }
}
}



void HomeY() {


     if(digital_inputs.read(DIN_READ_CH_PIN_01) == true) {
        delay(2);
        if(digital_inputs.read(DIN_READ_CH_PIN_01) == true) {
            delay(2);
            if(digital_inputs.read(DIN_READ_CH_PIN_01) == true) {
                //get away from home prox before homing sequence. Use delay since we're in a loop.
                    //Positive Dir
                for(int i = 0; i < awayFromHomeShort; i++) {
                    digital_outputs.set(2, LOW);
                    MoveFast(3);

                    }
                //Move negative until we reach home prox

                while(digital_inputs.read(DIN_READ_CH_PIN_01) != true) {
                    digital_outputs.set(2, HIGH);
                    MoveSlow(3);
                }
                //Set position to 0 once prox activated
                YPos = 0;
    }
    }
    }

    if(digital_inputs.read(DIN_READ_CH_PIN_01) != true) {
        delay(2);
        if(digital_inputs.read(DIN_READ_CH_PIN_01) != true) {
            delay(2);
            if(digital_inputs.read(DIN_READ_CH_PIN_01) != true) {
            //Move towards home, negative
            while(digital_inputs.read(DIN_READ_CH_PIN_01) != true) {
                digital_outputs.set(2, HIGH);
                MoveFast(3);
            }


            //Move away from home, positive
            for(int i = 0; i < awayFromHomeShort; i++) {
                digital_outputs.set(2, LOW);
                MoveFast(3);

                }
            // towards home, negative
            while(digital_inputs.read(DIN_READ_CH_PIN_01) != true) {
                digital_outputs.set(2, HIGH);
                MoveSlow(3);
            }
            YPos = 0;

    }
    }
    }
}



void HomeZ() {


 if(digital_inputs.read(DIN_READ_CH_PIN_02) == true) {
     delay(2);
     if(digital_inputs.read(DIN_READ_CH_PIN_02) == true) {
         delay(2);
         if(digital_inputs.read(DIN_READ_CH_PIN_02) == true) {
            //get away from home prox before homing sequence. Use delay since we're in a loop.
                //Positive Dir
            for(int i = 0; i < awayFromHomeShort; i++) {
                digital_outputs.set(4, LOW);
                MoveFast(5);
                }

            //Move negative until we reach home prox

            while(digital_inputs.read(DIN_READ_CH_PIN_02) != true) {
                digital_outputs.set(4, HIGH);
                MoveSlow(5);
            }
            //Set position to 0 once prox activated
            ZPos = 0;
    }
    }
    }


    if(digital_inputs.read(DIN_READ_CH_PIN_02) != true) {
        delay(2);
        if(digital_inputs.read(DIN_READ_CH_PIN_02) != true) {
            delay(2);
            if(digital_inputs.read(DIN_READ_CH_PIN_02) != true) {
            //Move towards home, negative
            while(!digital_inputs.read(DIN_READ_CH_PIN_02)) {
                digital_outputs.set(4, HIGH);
                MoveFast(5);
            }

            //Move away from home, positive
            for(int i = 0; i < awayFromHomeShort; i++) {
                digital_outputs.set(4, LOW);
                MoveFast(5);

                }
            // towards home, negative
            while(digital_inputs.read(DIN_READ_CH_PIN_02) != true) {
                digital_outputs.set(4, HIGH);
                MoveSlow(5);
            }
            ZPos = 0;
            }
    }
    }
        //Clearance
        while(ZPos < 12000) {
            digital_outputs.set(4, LOW);
            MoveFast(5);
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
                if(accelX < 1500) {
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
                if(accelX < 1500) {
                    accelX++;
                }
            }
    }

        previousMicrosX = currentMicros;
        if(PLCOut.XPos == XPos) {
            accelX = 1500;
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
                if(accelY < 1500) {
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
                if(accelY < 1500) {
                    accelY++;
                }
            }
    }
        previousMicrosY = currentMicros;
        if(PLCOut.YPos == YPos) {
            accelY = 1500;

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
                if(accelZ < 1500) {
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
                if(accelZ < 1500) {
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
