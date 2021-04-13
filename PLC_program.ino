#include <Controllino.h>
#include <SPI.h>

int cooldown = 1000;                // Time between movement/valve activations
int counter = 0;

char tableSide;
char selectHand = 1;

char handSlot[1];                  // Defines if slot 0 (lower postion) or 1 (upper position) is empty/full.

char leftTable[3][3] = {

  {1,  1,  1  },
  {1,  1,  1  },
  {1,  1,  1  }

};

char rightTable[3][3] = {

  {1,  1,  1  },
  {1,  1,  1  },
  {1,  1,  1  }

};

bool isUp = false;                 // Body is or isn't in elevated position.    
bool isMoving = false;             // The manipulator is or isn't moving.


const byte C1_cilindar = CONTROLLINO_R1; // R1    Cilindar 1
const byte C2_cilindar = CONTROLLINO_R2; // R2    Cilindar 2

const byte C3_cilindar = CONTROLLINO_R3; // R3    Cilindar 3
const byte C4_cilindar = CONTROLLINO_R4; // R4    Cilindar 4

const byte C5_cilindar = CONTROLLINO_R5; // R5    Cilindar 5
const byte C6_cilindar = CONTROLLINO_R6; // R6    Cilindar 6

const byte C7_cilindar = CONTROLLINO_R7; // R7    Cilindar 7 (180 degree body rotate)
const byte C8_cilindar = CONTROLLINO_R8; // R8    Cilindar 8 (up-down of body)
const byte C9_cilindar = CONTROLLINO_R9; // R9    Cilindar 9 (180 degree hand rotate)


const byte C1_izvucen = CONTROLLINO_A0; // AI0   senzor C1.0
const byte C1_uvucen = CONTROLLINO_A1;  // AI1   senzor C1.1
const byte C2_uvucen = CONTROLLINO_A2;  // AI2   senzor C2.0
const byte C2_izvucen = CONTROLLINO_A3; // AI3   senzor C2.1

const byte C3_izvucen = CONTROLLINO_A4; // AI4   senzor C3.0
const byte C3_uvucen = CONTROLLINO_A5;  // AI5   senzor C3.1
const byte C4_uvucen = CONTROLLINO_A6;  // AI6   senzor C4.0
const byte C4_izvucen = CONTROLLINO_A7; // AI7   senzor C4.1

const byte C5_izvucen = CONTROLLINO_A8; // AI8   senzor C5.0
const byte C5_uvucen = CONTROLLINO_A9;  // AI9   senzor C5.1
const byte C6_uvucen = CONTROLLINO_A10; // AI10  senzor C6.0
const byte C6_izvucen = CONTROLLINO_A11;// AI11  senzor C6.1

const byte Vacuum_1 = CONTROLLINO_D0;   // DO0   upali vakuum 1
const byte Vacuum_2 = CONTROLLINO_D1;   // DO1   upali vakuum 2

const byte LED_Start = CONTROLLINO_D5;  // DO5   svjetlo Start/ON
const byte LED_Error = CONTROLLINO_D6;  // DO6   svjetlo Error
const byte LED_Stop = CONTROLLINO_D7;   // DO7   svjetlo Stop

const byte handIsRight = 66;            // DI0   senzor C7.0, ruka je sad desno
const byte handIsLeft = 67;             // DI1   senzor C7.1, ruka je sad lijevo

const byte objectGrabbed_V1 = 10;       // DI2   senzor vakuum 1
const byte objectGrabbed_V2 = 11;       // DI3   senzor vakuum 2

const byte interruptStartPin = CONTROLLINO_IN0; // IN0   interrupt ulaz, Start tipka
volatile byte startPressed = LOW;

const byte interruptStopPin = CONTROLLINO_IN1;  // IN1   interrupt ulaz, Stop tipka
volatile byte stopPressed = LOW;

void setup() {
    Serial.begin(9600);

    pinMode(CONTROLLINO_IN0, INPUT);                            // IN0   tipka Start
    pinMode(CONTROLLINO_IN1, INPUT);                            // IN1   tipka Stop

    pinMode(CONTROLLINO_A0, INPUT);                             // AI0   senzor C1.0
    pinMode(CONTROLLINO_A1, INPUT);                             // AI1   senzor C1.1
    pinMode(CONTROLLINO_A2, INPUT);                             // AI2   senzor C2.0
    pinMode(CONTROLLINO_A3, INPUT);                             // AI3   senzor C2.1
    pinMode(CONTROLLINO_A4, INPUT);                             // AI4   senzor C3.0
    pinMode(CONTROLLINO_A5, INPUT);                             // AI5   senzor C3.1
    pinMode(CONTROLLINO_A6, INPUT);                             // AI6   senzor C4.0
    pinMode(CONTROLLINO_A7, INPUT);                             // AI7   senzor C4.1
    pinMode(CONTROLLINO_A8, INPUT);                             // AI8   senzor C5.0
    pinMode(CONTROLLINO_A9, INPUT);                             // AI9   senzor C5.1
    pinMode(CONTROLLINO_A10, INPUT);                            // AI10  senzor C6.0
    pinMode(CONTROLLINO_A11, INPUT);                            // AI11  senzor C6.1

    pinMode(66, INPUT);                                         // DI0   senzor C7.0, ruka je desno
    pinMode(67, INPUT);                                         // DI1   senzor C7.1
    pinMode(10, INPUT);                                         // DI2   senzor vakuum 1
    pinMode(11, INPUT);                                         // DI3   senzor vakuum 2

    pinMode(CONTROLLINO_D0, OUTPUT);                            // DO0   vacuum 1 ON (donji)
    pinMode(CONTROLLINO_D1, OUTPUT);                            // DO1   vacuum 2 ON (gornji)
    pinMode(CONTROLLINO_D2, OUTPUT);                            // DO2
    pinMode(CONTROLLINO_D3, OUTPUT);                            // DO3
    pinMode(CONTROLLINO_D4, OUTPUT);                            // DO4
    pinMode(CONTROLLINO_D5, OUTPUT);                            // DO5   LED Ready
    pinMode(CONTROLLINO_D6, OUTPUT);                            // DO6   LED Error
    pinMode(CONTROLLINO_D7, OUTPUT);                            // DO7   LED Stop

    pinMode(CONTROLLINO_R1, OUTPUT);                            // R1    Cilindar 1
    pinMode(CONTROLLINO_R2, OUTPUT);                            // R2    Cilindar 2
    pinMode(CONTROLLINO_R3, OUTPUT);                            // R3    Cilindar 3
    pinMode(CONTROLLINO_R4, OUTPUT);                            // R4    Cilindar 4

    pinMode(CONTROLLINO_R5, OUTPUT);                            // R5    Cilindar 5
    pinMode(CONTROLLINO_R6, OUTPUT);                            // R6    Aktuator 6
    pinMode(CONTROLLINO_R7, OUTPUT);                            // R7    Aktuator 7   (180 degree body rotate)
    pinMode(CONTROLLINO_R8, OUTPUT);                            // R8    Aktuator 8   (up-down of body)
    pinMode(CONTROLLINO_R9, OUTPUT);                            // R9    Aktuator 9   (180 degree hand rotate)

    pinMode(interruptStartPin, INPUT);
    pinMode(interruptStopPin, INPUT);

    attachInterrupt(interruptStartPin, StartPressed, FALLING);
    attachInterrupt(interruptStopPin, StopPressed, RISING);
} 

//////////////////////////////// MAIN //////////////////////////////////////////////
void loop() {
    Serial.println("Starting program....");
    digitalWrite(LED_Start, HIGH);

    if (startPressed == HIGH ) {
        
        delay(2000);

        LiftUp();

        TableGoLeft();
        TableGoCenter(1);
        TableGoRight();
        TableGoUp(1);

        LiftDown();

        digitalWrite(Vacuum_1, HIGH);
        delay(500);

        LiftUp();
        TableGoCenter(1);
        LiftDown();
        digitalWrite(Vacuum_1, LOW);
        delay(500);

        RotateRight();
        
        LiftDown();
        digitalWrite(Vacuum_1, HIGH);
        delay(500);

        SelectHand(2);
        digitalWrite(Vacuum_1, LOW);
        delay(500);

        digitalWrite(LED_Error, HIGH);

       
        startPressed = LOW;

    }

    
    
}



//////////////////////////////// FUNCTIONS //////////////////////////////////////////////


void TableGoRight() {

    if (isMoving == false) {

        isMoving = true;
        Serial.print("Going right. ");

        digitalWrite(CONTROLLINO_R6, HIGH);
        digitalWrite(CONTROLLINO_R5, HIGH);
        delay(cooldown);

        //Serial.print("Move status: %b, isM ");
        
        while (isMoving == true) {
            Serial.print("Waiting for input from sensors C5 and C6... ");
            if (digitalRead(C5_izvucen) == 1 && digitalRead(C6_izvucen) == 1) {
                isMoving = false;
                Serial.println("Move completed.");
            }
        }

    }

}

void TableGoLeft() {
    // comment
    if (isMoving == false) {

        isMoving = true;
        Serial.print("Going left. ");

        digitalWrite(CONTROLLINO_R6, LOW);
        digitalWrite(CONTROLLINO_R5, LOW);
        delay(cooldown);

        Serial.print("Waiting for input from sensors C5 and C6. ");
        while (isMoving == true) {

            if (digitalRead(C5_uvucen) == 1 && digitalRead(C6_uvucen) == 1) {
                isMoving = false;
                Serial.println("Move completed.");
            }
        }

    }

}

void TableGoCenter(char tableSide) {
    // R/r/1 - left table
    // L/l/2 - right table
    if (tableSide == 2) {

        if (isMoving == false) {
            isMoving = true;
            Serial.print("Moving right table to centre. ");

            digitalWrite(C3_cilindar, HIGH);
            digitalWrite(C4_cilindar, LOW);
            delay(cooldown);

            Serial.print("Waiting for input from sensors on C3 and C4. ");
            while (isMoving == true) {

                if (digitalRead(C3_izvucen) == 1 && digitalRead(C4_uvucen) == 1) {
                    isMoving = false;
                    Serial.println("Move completed.");
                }
            }
        }

    }
    else if (tableSide == 1) {

        if (isMoving == false) {
            isMoving = true;
            Serial.print("Moving left table to centre. ");

            digitalWrite(C1_cilindar, HIGH);
            digitalWrite(C2_cilindar, LOW);
            delay(cooldown);

            Serial.print("Waiting for input from sensors on C1 and C2. ");
            while (isMoving == true) {

                if (digitalRead(C1_izvucen) == 1 && digitalRead(C2_uvucen) == 1) {
                    isMoving = false;
                    Serial.println("Move completed.");
                }
            }
        }

    }
    else
    {
        Serial.println("Incorrect input. Aborting sequence...");
    }
}

void TableGoUp(char tableSide) {
    // 2 - right table
    // 1 - left table
    if (tableSide == 2) {

        if (isMoving == false) {
            isMoving = true;
            Serial.print("Moving right table up. ");

            digitalWrite(C3_cilindar, HIGH);
            digitalWrite(C4_cilindar, HIGH);
            delay(cooldown);

            Serial.print("Waiting for input from sensors on C3 and C4. ");
            while (isMoving == true) {
                
                if (digitalRead(C3_izvucen)==1 && digitalRead(C4_izvucen)==1) {
                    isMoving = false;
                    Serial.println("Move completed.");
                }
            }
        }

    }
    else if (tableSide==1) {

        if (isMoving == false) {
            isMoving = true;
            Serial.print("Moving left table up. ");

            digitalWrite(C1_cilindar, HIGH);
            digitalWrite(C2_cilindar, HIGH);
            delay(cooldown);

            Serial.print("Waiting for input from sensors on C1 and C2. ");
            while (isMoving == true) {
                
                if (digitalRead(C1_izvucen)==1 && digitalRead(C2_izvucen)==1) {
                    isMoving = false;
                    Serial.println("Move completed.");
                }
            }
        }


    }
    else
    {
        Serial.println("Incorrect input. Aborting sequence...");
    }

}

void TableGoDown(char tableSide) {
    // R/r/1 - left table
    // L/l/2 - right table
    if (tableSide == 2) {

        if (isMoving == false) {
            isMoving = true;
            Serial.print("Moving right table down. ");

            digitalWrite(C3_cilindar, LOW);
            digitalWrite(C4_cilindar, LOW);
            delay(cooldown);

            Serial.print("Waiting for input from sensors on C3 and C4. ");
            while (isMoving == true) {

                if (digitalRead(C3_uvucen) == 1 && digitalRead(C4_uvucen) == 1) {
                    isMoving = false;
                    Serial.println("Move completed.");
                }
            }
        }

    }
    else if (tableSide == 1) {

        if (isMoving == false) {
            isMoving = true;
            Serial.print("Moving left table down. ");

            digitalWrite(C1_cilindar, LOW);
            digitalWrite(C2_cilindar, LOW);
            delay(cooldown);

            Serial.print("Waiting for input from sensors on C1 and C2. ");
            while (isMoving == true) {

                if (digitalRead(C1_uvucen)==1 && digitalRead(C2_uvucen)==1) {
                    isMoving = false;
                    Serial.println("Move completed.");
                }
            }
        }

    }
    else
    {
        Serial.println("Incorrect input. Aborting sequence...");
    }

}


void RotateRight() {

    if (isMoving == false && (isUp == true ))  //maybe add "|| handSlot[0] == 0"
    {
        isMoving = true;
        Serial.print("Rotating to the right.");
        digitalWrite(C7_cilindar, HIGH);
        delay(cooldown);

        while (isMoving == true) {

            if (digitalRead(handIsRight) == 1) {
                isMoving = false;
                Serial.println("Move completed.");
            }
        }
    }
    else if (isMoving == false && (isUp == false )) //maybe add "|| handSlot[0] == 0"
    {
        isMoving = true;
        Serial.print("Rotating to the right.");
        digitalWrite(C8_cilindar, LOW);
        delay(cooldown);

        isUp = true;

        digitalWrite(C7_cilindar, HIGH);
        delay(cooldown);

        while (isMoving == true) {

            if (digitalRead(handIsRight) == 1) {
                isMoving = false;
                Serial.println("Move completed.");
            }
        }
    }
    else
    {
        Serial.println("Rotation to the right can't be perfomed. ");
    }

}

void RotateLeft() {

    if (isMoving == false && (isUp == true)) //maybe add "|| handSlot[0] == 0"
    {
        isMoving = true;
        Serial.print("Rotating to the right.");

        digitalWrite(C7_cilindar, LOW);
        delay(cooldown);

        while (isMoving == true) {

            if (digitalRead(handIsLeft) == 1) {
                isMoving = false;
                Serial.println("Move completed.");
            }
        }
    }
    else if (isMoving == false && (isUp == false)) //maybe add "|| handSlot[0] == 0"
    {
        isMoving = true;
        Serial.print("Rotating to the right.");

        digitalWrite(C8_cilindar, LOW);
        delay(cooldown);

        isUp = true;

        digitalWrite(C7_cilindar, HIGH);
        delay(cooldown);

        while (isMoving == true) {

            if (digitalRead(handIsLeft) == 1) {
                isMoving = false;
                Serial.println("Move completed.");
            }
        }
    }
    else
    {
        Serial.println("Rotation to the left can't be perfomed. ");
    }



}

void SelectHand(char selectHand) {


    if (isMoving == false) {
        // 1-lower suction (default)
        // 2-upper suction
        isMoving = true;
        Serial.print("Initiating hand selection. ");

        switch (selectHand)
        {
        case 1:

            if (C9_cilindar == LOW)
            {
                isMoving = false;
                Serial.println("Hand 1 selected. ");
            }
            else
            {
                digitalWrite(C8_cilindar, LOW);
                delay(cooldown);

                digitalWrite(C9_cilindar, LOW);
                delay(cooldown);

                if (handSlot[0] == 0)
                {
                    digitalWrite(C8_cilindar, HIGH);
                    delay(cooldown);

                    isMoving = false;
                    Serial.println("Hand 1 selected. ");
                }
                else if (handSlot[0] == 1)
                {
                    digitalWrite(C8_cilindar, LOW);
                    delay(cooldown);

                    isMoving = false;
                    Serial.println("Hand 1 selected. ");
                }
                else
                {
                    isMoving = false;
                    Serial.println("Hand 1 selected. ");
                }
            }
            break;
        case 2:
            if (C9_cilindar == HIGH)
            {
                isMoving = false;
                Serial.println("Hand 1 selected. ");
            }
            else
            {
                digitalWrite(C8_cilindar, LOW);
                delay(cooldown);

                digitalWrite(C9_cilindar, HIGH);
                delay(cooldown);

                if (handSlot[0] == 0)
                {
                    digitalWrite(C8_cilindar, HIGH);
                    delay(cooldown);

                    isMoving = false;
                    Serial.println("Hand 1 selected. ");
                }
                else if (handSlot[0] == 1)
                {
                    digitalWrite(C8_cilindar, LOW);
                    delay(cooldown);

                    isMoving = false;
                    Serial.println("Hand 1 selected. ");
                }
                else
                {
                    isMoving = false;
                    Serial.println("Hand 1 selected. ");
                }
            }
            break;
        default:
            Serial.println("Incorret option selected. Aborting sequence...");
            isMoving = false;
            break;
        }
    }
}

void LiftUp() {
    if (isMoving == false) {
        isMoving = true;

        digitalWrite(C8_cilindar, LOW);
        delay(cooldown);

        isUp = true;
        isMoving = false;
    }
}

void LiftDown() {
    if (isMoving == false) {
        isMoving = true;

        digitalWrite(C8_cilindar, HIGH);
        delay(cooldown);

        isUp = false;
        isMoving = false;
    }
}

void Vacuum1(char state) {


    if (isMoving == false) {

        isMoving = true;
        Serial.print("Going right. ");

        digitalWrite(CONTROLLINO_R6, HIGH);
        digitalWrite(CONTROLLINO_R5, HIGH);
        delay(cooldown);

        Serial.print("Waiting for input from sensors C5 and C6. ");
        while (isMoving == true) {

            if (C5_izvucen == 1 && C6_izvucen == 1) {
                isMoving = false;
                Serial.println("Move completed.");
            }
        }

    }

}

void StartPressed () {
    Serial.println("Start pressed.");
    startPressed = HIGH;
    digitalWrite(LED_Start, LOW);
}

void StopPressed() {
    Serial.println("Start pressed.");
    //stopPressed
}

//////////////////////////////// FUNCTIONS //////////////////////////////////////////////
