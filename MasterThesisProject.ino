#include <Controllino.h>

int cooldown = 500;                // Time between movement/valve activations
int counter = 0;

char tableSide;
char selectHand = 1;

char handSlot[1];                  // Defines if slot 0 (lower postion) or 1 (upper position) is empty/full.
char leftTable[10] = { 0,1,1,1,1,1,1,1,1,1 };
char rightTable[10] = { 0,0,0,0,0,0,0,0,0,0 };

bool isUp = false;                 // Body is or isn't in elevated position.    
bool isMoving = false;             // The manipulator is or isn't moving.


byte C1_cilindar = CONTROLLINO_R1; // R1    Cilindar 1
byte C2_cilindar = CONTROLLINO_R2; // R2    Cilindar 2

byte C3_cilindar = CONTROLLINO_R3; // R3    Cilindar 3
byte C4_cilindar = CONTROLLINO_R4; // R4    Cilindar 4

byte C5_cilindar = CONTROLLINO_R5; // R5    Cilindar 1
byte C6_cilindar = CONTROLLINO_R6; // R6    Cilindar 1

byte C7_cilindar = CONTROLLINO_R7; // R7    Cilindar 7 (180 degree body rotate)
byte C8_cilindar = CONTROLLINO_R8; // R8    Cilindar 8 (up-down of body)
byte C9_cilindar = CONTROLLINO_R9; // R9    Cilindar 9 (180 degree hand rotate)


byte C1_uvucen = CONTROLLINO_A0;  // AI0   senzor C1.0
byte C1_izvucen = CONTROLLINO_A1; // AI1   senzor C1.1
byte C2_uvucen = CONTROLLINO_A2;  // AI2   senzor C2.0
byte C2_izvucen = CONTROLLINO_A3; // AI3   senzor C2.1

byte C3_izvucen = CONTROLLINO_A4; // AI4   senzor C3.0
byte C3_uvucen = CONTROLLINO_A5;  // AI5   senzor C3.1
byte C4_uvucen = CONTROLLINO_A6;  // AI6   senzor C4.0
byte C4_izvucen = CONTROLLINO_A7; // AI7   senzor C4.1

byte C5_izvucen = CONTROLLINO_A8; // AI8   senzor C5.0
byte C5_uvucen = CONTROLLINO_A9;  // AI9   senzor C5.1
byte C6_uvucen = CONTROLLINO_A10; // AI11  senzor C6.0
byte C6_izvucen = CONTROLLINO_A11;// AI10  senzor C6.1

byte handIsRight = 66;            // DI0   senzor C7.0, ruka je sad desno
byte handIsLeft = 67;             // DI1   senzor C7.1, ruka je sad lijevo

byte objectGrabbed_V1 = 10;       // DI2   senzor vakuum 1
byte objectGrabbed_V2 = 11;       // DI3   senzor vakuum 2

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
    pinMode(CONTROLLINO_D6, OUTPUT);                            // DO6   LED Stop
    pinMode(CONTROLLINO_D7, OUTPUT);                            // DO7   LED Error

    pinMode(CONTROLLINO_R1, OUTPUT);                            // R1    Cilindar 1
    pinMode(CONTROLLINO_R2, OUTPUT);                            // R2    Cilindar 2
    pinMode(CONTROLLINO_R3, OUTPUT);                            // R3    Cilindar 3
    pinMode(CONTROLLINO_R4, OUTPUT);                            // R4    Cilindar 4

    pinMode(CONTROLLINO_R5, OUTPUT);                            // R5    Cilindar 5
    pinMode(CONTROLLINO_R6, OUTPUT);                            // R6    Aktuator 6
    pinMode(CONTROLLINO_R7, OUTPUT);                            // R7    Aktuator 7   (180 degree body rotate)
    pinMode(CONTROLLINO_R8, OUTPUT);                            // R8    Aktuator 8   (up-down of body)
    pinMode(CONTROLLINO_R9, OUTPUT);                            // R9    Aktuator 9   (180 degree hand rotate)
} //end of setup()

//////////////////////////////// MAIN //////////////////////////////////////////////
void loop() {


    counter = counter + 1;
}



//////////////////////////////// FUNCTIONS //////////////////////////////////////////////


void GoRight() {

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

void GoLeft() {

    if (isMoving == false) {

        isMoving = true;
        Serial.print("Going left. ");

        digitalWrite(CONTROLLINO_R6, LOW);
        digitalWrite(CONTROLLINO_R5, LOW);
        delay(cooldown);

        Serial.print("Waiting for input from sensors C5 and C6. ");
        while (isMoving == true) {

            if (C5_uvucen == 1 && C6_uvucen == 1) {
                isMoving = false;
                Serial.println("Move completed.");
            }
        }

    }

}

void GoCenter() {
    if (isMoving == false) {
        isMoving = true;

        digitalWrite(C5_cilindar, HIGH);
        digitalWrite(C6_cilindar, LOW);
        delay(cooldown);

        Serial.print("Waiting for input from sensors C6 and C5. ");
        while (isMoving == true) {

            if (C5_izvucen == 1 && C6_izvucen == 1) {
                isMoving = false;
                Serial.println("Move completed.");
            }
        }
    }


}

void GoUp(char tableSide) {
    // R/r/1 - left table
    // L/l/2 - right table
    if (tableSide == 'R' || 'r' || 2) {

        if (isMoving == false) {
            isMoving = true;
            Serial.print("Moving right table up. ");

            digitalWrite(C3_cilindar, HIGH);
            digitalWrite(C4_cilindar, HIGH);
            delay(cooldown);

            Serial.print("Waiting for input from sensors on C1 and C2. ");
            while (isMoving == true) {

                if (C3_izvucen == 1 && C4_izvucen == 1) {
                    isMoving = false;
                    Serial.println("Move completed.");
                }
            }
        }

    }
    else if (tableSide == 'L' || 'l' || 1) {

        if (isMoving == false) {
            isMoving = true;
            Serial.print("Moving left table up. ");

            digitalWrite(C1_cilindar, HIGH);
            digitalWrite(C2_cilindar, HIGH);
            delay(cooldown);

            Serial.print("Waiting for input from sensors on C1 and C2. ");
            while (isMoving == true) {

                if (C1_izvucen == 1 && C2_izvucen == 1) {
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

void GoDown(char tableSide) {
    // R/r/1 - left table
    // L/l/2 - right table
    if (tableSide == 'R' || 'r' || 2) {

        if (isMoving == false) {
            isMoving = true;
            Serial.print("Moving right table down. ");

            digitalWrite(C3_cilindar, LOW);
            digitalWrite(C4_cilindar, LOW);
            delay(cooldown);

            Serial.print("Waiting for input from sensors on C3 and C4. ");
            while (isMoving == true) {

                if (C3_uvucen == 1 && C4_uvucen == 1) {
                    isMoving = false;
                    Serial.println("Move completed.");
                }
            }
        }

    }
    else if (tableSide == 'L' || 'l' || 1) {

        if (isMoving == false) {
            isMoving = true;
            Serial.print("Moving left table down. ");

            digitalWrite(C1_cilindar, LOW);
            digitalWrite(C2_cilindar, LOW);
            delay(cooldown);

            Serial.print("Waiting for input from sensors on C1 and C2. ");
            while (isMoving == true) {

                if (C1_uvucen == 1 && C2_uvucen == 1) {
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

    if (isMoving == false && (isUp == true || handSlot[0] == 0))
    {
        isMoving = true;

        digitalWrite(C7_cilindar, HIGH);
        delay(cooldown);

        while (isMoving == true) {

            if (handIsRight == 1) {
                isMoving = false;
            }
        }
    }
    else
    {
        Serial.println("Rotation to the right can't be perfomed. ");
    }

}

void RotateLeft() {

    if (isMoving == false && (isUp == true || handSlot[0] == 0))
    {
        isMoving = true;

        digitalWrite(C7_cilindar, LOW);
        delay(cooldown);

        while (isMoving == true) {

            if (handIsLeft == 1) {
                isMoving = false;
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

        isMoving = true;
        Serial.print("Initiating hand selection. ");

        switch (selectHand)
        {
        case 1:

            if (C9_cilindar == LOW)
            {
                isMoving = false;
                Serial.print("Hand 1 selected. ");
            }
            else
            {
                digitalWrite(C8_cilindar, HIGH);
                delay(cooldown);

                digitalWrite(C9_cilindar, LOW);
                delay(cooldown);

                if (handSlot[0] == 0)
                {
                    digitalWrite(C8_cilindar, LOW);
                    delay(cooldown);

                    isMoving = false;
                    Serial.print("Hand 1 selected. ");
                }
                else if (handSlot[0] == 1)
                {
                    digitalWrite(C8_cilindar, HIGH);
                    delay(cooldown);

                    isMoving = false;
                    Serial.print("Hand 1 selected. ");
                }
                else
                {
                    isMoving = false;
                    Serial.print("Hand 1 selected. ");
                }
            }
            break;
        case 2:
            if (C9_cilindar == HIGH)
            {
                isMoving = false;
                Serial.print("Hand 1 selected. ");
            }
            else
            {
                digitalWrite(C8_cilindar, HIGH);
                delay(cooldown);

                digitalWrite(C9_cilindar, HIGH);
                delay(cooldown);

                if (handSlot[0] == 0)
                {
                    digitalWrite(C8_cilindar, LOW);
                    delay(cooldown);

                    isMoving = false;
                    Serial.print("Hand 1 selected. ");
                }
                else if (handSlot[0] == 1)
                {
                    digitalWrite(C8_cilindar, HIGH);
                    delay(cooldown);

                    isMoving = false;
                    Serial.print("Hand 1 selected. ");
                }
                else
                {
                    isMoving = false;
                    Serial.print("Hand 1 selected. ");
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

        digitalWrite(C8_cilindar, HIGH);
        delay(cooldown);

        isUp = true;
        isMoving = false;
    }
}

void LiftDown() {
    if (isMoving == false) {
        isMoving = true;

        digitalWrite(C8_cilindar, LOW);
        delay(cooldown);

        isUp = false;
        isMoving = false;
    }
}



//////////////////////////////// FUNCTIONS //////////////////////////////////////////////