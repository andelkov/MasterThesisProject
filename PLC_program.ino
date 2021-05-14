#include <Controllino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "Mudbus.h"

#define NULL 0
#define highWord(w) ((w) >> 16) 
#define lowWord(w) ((w) & 0xffff)
#define makeLong(hi, low) (((long) hi) << 16 | (low)) //convert two 16bit variables back to a 32bit variable

Mudbus Mb;

//Function codes 1(read coils), 3(read registers), 5(write coil), 6(write register)
//signed int Mb.R[0 to 125]
//Port 502 is default (defined by MB_PORT, in Mudbus.h) 

int cooldown = 1000;                // Time between movement/valve activations 
int counter = 0;
char selectMode;

unsigned long longValue = 1011111111;
unsigned long message;


uint32_t loWord, hiWord;
int messageArray[10];

byte i;
byte tableSide;
String arrayPart;
String messageString;
char selectHand = 1;
char handSlot[1];                  // Defines if slot 0 (lower postion) or 1 (upper position) is empty/full
byte workMode = 1;                 // Select work mode (1-Auto, 2-Jog)
bool isUp = false;                 // Body is or isn't in elevated position.    
bool isMoving = false;             // The manipulator is or isn't moving.

char tableLeft[10] = { NULL, 1, 1, 1, 1, 1, 1, 1, 1, 1 };//Actual state left table
char tableRight[10] = { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0 };//Actual state right table
int tempArray[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const byte C1_cilindar = CONTROLLINO_R1; // R1    Cilindar 1
const byte C2_cilindar = CONTROLLINO_R2; // R2    Cilindar 2

const byte C3_cilindar = CONTROLLINO_R3; // R3    Cilindar 3
const byte C4_cilindar = CONTROLLINO_R4; // R4    Cilindar 4

const byte C5_cilindar = CONTROLLINO_R5; // R5    Cilindar 5
const byte C6_cilindar = CONTROLLINO_R6; // R6    Cilindar 6

const byte C7_cilindar = CONTROLLINO_R7; // R7    Cilindar 7 (180 degree body rotate)
const byte C8_cilindar = CONTROLLINO_R8; // R8    Cilindar 8 (up-down of body)
const byte C9_cilindar = CONTROLLINO_R9; // R9    Cilindar 9 (180 degree hand rotate)


const byte C1_izvucen = CONTROLLINO_A0;  // AI0   senzor C1.0
const byte C1_uvucen = CONTROLLINO_A1;   // AI1   senzor C1.1
const byte C2_uvucen = CONTROLLINO_A2;   // AI2   senzor C2.0
const byte C2_izvucen = CONTROLLINO_A3;  // AI3   senzor C2.1

const byte C3_izvucen = CONTROLLINO_A4;  // AI4   senzor C3.0
const byte C3_uvucen = CONTROLLINO_A5;   // AI5   senzor C3.1
const byte C4_uvucen = CONTROLLINO_A6;   // AI6   senzor C4.0
const byte C4_izvucen = CONTROLLINO_A7;  // AI7   senzor C4.1

const byte C5_izvucen = CONTROLLINO_A8;  // AI8   senzor C5.0
const byte C5_uvucen = CONTROLLINO_A9;   // AI9   senzor C5.1
const byte C6_uvucen = CONTROLLINO_A10;  // AI10  senzor C6.0
const byte C6_izvucen = CONTROLLINO_A11; // AI11  senzor C6.1

const byte Vacuum_1 = CONTROLLINO_D0;    // DO0   upali vakuum 1
const byte Vacuum_2 = CONTROLLINO_D1;    // DO1   upali vakuum 2

const byte LED_Start = CONTROLLINO_D5;   // DO5   svjetlo Start/ON
const byte LED_Error = CONTROLLINO_D6;   // DO6   svjetlo Error
const byte LED_Stop = CONTROLLINO_D7;    // DO7   svjetlo Stop

const byte handIsRight = 66;             // DI0   senzor C7.0, ruka je sad desno
const byte handIsLeft = 67;              // DI1   senzor C7.1, ruka je sad lijevo

const byte objectGrabbed_V1 = 10;        // DI2   senzor vakuum 1
const byte objectGrabbed_V2 = 11;        // DI3   senzor vakuum 2

const byte interruptStartPin = 18;       // IN0   interrupt ulaz, Start tipka
volatile byte startPressed = LOW;

const byte interruptStopPin = 19;        // IN1   interrupt ulaz, Stop tipka
volatile byte stopPressed = LOW;                                                       //Pin configuration

void setup() {
	uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x51, 0x06 };
	uint8_t ip[] = { 192, 168, 1, 8 };
	uint8_t gateway[] = { 192, 168, 1, 1 };
	uint8_t subnet[] = { 255, 255, 255, 0 };
	Ethernet.begin(mac, ip, subnet);

	delay(2000);

	Serial.begin(9600);
	Serial.print("Started. ");
	Serial.print("My IP address: ");
	Serial.println(Ethernet.localIP());

	for (i = 0; i < 126; i++) {                                 // Set every Modbus register value to 0
		Mb.R[i] = 0;
	}
	Serial.println("Modbus registers set 0");

	pinMode(CONTROLLINO_IN0, INPUT);                            // IN0   tipka Start
	Mb.R[0] = CONTROLLINO_IN0; Mb.R[0] = 0;
	pinMode(CONTROLLINO_IN1, INPUT);                            // IN1   tipka Stop
	Mb.R[1] = CONTROLLINO_IN1; Mb.R[1] = 0;

	pinMode(CONTROLLINO_A0, INPUT);                             // AI0   senzor C1.0
	Mb.R[69] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A1, INPUT);                             // AI1   senzor C1.1
	Mb.R[1] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A2, INPUT);                             // AI2   senzor C2.0
	Mb.R[2] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A3, INPUT);                             // AI3   senzor C2.1
	Mb.R[3] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A4, INPUT);                             // AI4   senzor C3.0
	Mb.R[4] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A5, INPUT);                             // AI5   senzor C3.1
	Mb.R[5] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A6, INPUT);                             // AI6   senzor C4.0
	Mb.R[6] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A7, INPUT);                             // AI7   senzor C4.1
	Mb.R[7] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A8, INPUT);                             // AI8   senzor C5.0
	Mb.R[8] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A9, INPUT);                             // AI9   senzor C5.1
	Mb.R[9] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A10, INPUT);                            // AI10  senzor C6.0
	Mb.R[10] = CONTROLLINO_IN1;
	pinMode(CONTROLLINO_A11, INPUT);                            // AI11  senzor C6.1
	Mb.R[11] = CONTROLLINO_IN1;

	pinMode(66, INPUT);                                         // DI0   senzor C7.0, ruka je desno
	Mb.R[12] = CONTROLLINO_IN1;
	pinMode(67, INPUT);                                         // DI1   senzor C7.1
	Mb.R[13] = CONTROLLINO_IN1;
	pinMode(10, INPUT);                                         // DI2   senzor vakuum 1
	Mb.R[14] = CONTROLLINO_IN1;
	pinMode(11, INPUT);                                         // DI3   senzor vakuum 2
	Mb.R[15] = CONTROLLINO_IN1;

	pinMode(CONTROLLINO_D0, OUTPUT);                            // DO0   vacuum 1 ON (donji)
	Mb.R[80] = CONTROLLINO_IN0; Mb.C[80] = 0;
	pinMode(CONTROLLINO_D1, OUTPUT);                            // DO1   vacuum 2 ON (gornji)
	Mb.R[81] = CONTROLLINO_IN0; Mb.C[81] = 0;
	pinMode(CONTROLLINO_D5, OUTPUT);                            // DO5   LED Ready
	Mb.R[85] = CONTROLLINO_IN0; Mb.C[85] = 0;
	pinMode(CONTROLLINO_D6, OUTPUT);                            // DO6   LED Error
	Mb.R[86] = CONTROLLINO_IN0; Mb.C[86] = 0;
	pinMode(CONTROLLINO_D7, OUTPUT);                            // DO7   LED Stop
	Mb.R[87] = CONTROLLINO_IN0; Mb.C[87] = 0;

	pinMode(CONTROLLINO_R1, OUTPUT);                            // R1    Cilindar 1
	Mb.R[91] = CONTROLLINO_IN0; Mb.C[91] = 0;
	pinMode(CONTROLLINO_R2, OUTPUT);                            // R2    Cilindar 2
	Mb.R[92] = CONTROLLINO_IN0; Mb.C[92] = 0;
	pinMode(CONTROLLINO_R3, OUTPUT);                            // R3    Cilindar 3
	Mb.R[93] = CONTROLLINO_IN0; Mb.C[93] = 0;
	pinMode(CONTROLLINO_R4, OUTPUT);                            // R4    Cilindar 4
	Mb.R[94] = CONTROLLINO_IN0; Mb.C[94] = 0;
	pinMode(CONTROLLINO_R5, OUTPUT);                            // R5    Cilindar 5
	Mb.R[95] = CONTROLLINO_IN0; Mb.C[95] = 0;
	pinMode(CONTROLLINO_R6, OUTPUT);                            // R6    Aktuator 6
	Mb.R[96] = CONTROLLINO_IN0; Mb.C[96] = 0;
	pinMode(CONTROLLINO_R7, OUTPUT);                            // R7    Aktuator 7   (180 degree body rotate)
	Mb.R[97] = CONTROLLINO_IN0; Mb.C[97] = 0;
	pinMode(CONTROLLINO_R8, OUTPUT);                            // R8    Aktuator 8   (up-down of body)
	Mb.R[98] = CONTROLLINO_IN0; Mb.C[98] = 0;
	pinMode(CONTROLLINO_R9, OUTPUT);                            // R9    Aktuator 9   (180 degree hand rotate)
	Mb.R[99] = CONTROLLINO_IN0; Mb.C[99] = 0;

	pinMode(interruptStartPin, INPUT);
	pinMode(interruptStopPin, INPUT);

	attachInterrupt(digitalPinToInterrupt(interruptStartPin), StartPressed, FALLING);
	attachInterrupt(digitalPinToInterrupt(interruptStopPin), StopPressed, RISING);
}

//////////////////////////////// MAIN //////////////////////////////////////////////
void loop() {

	Mb.Run();

	if ((startPressed == LOW) || (Mb.R[0] != 1)) {
		digitalWrite(LED_Start, HIGH);
		delay(1000);
		digitalWrite(LED_Start, LOW);
	}

	switch (workMode) {
	case 1:
		Serial.println("Auto mode selected.");
		// 1 111 111 111 dolazi:
		loWord = lowWord(longValue);              // convert long to two words
		hiWord = highWord(longValue);
		message = makeLong(hiWord, loWord);

		messageString = String(message);

		for (char i = 0; i < 10; i++) {
			arrayPart = messageString.charAt(i);
			messageArray[i] = arrayPart.toInt();
		}

		Serial.print("The final end result is: ");    //printanje arraya
		for (int i = 0; i < 10; i++)
		{
			Serial.print(messageArray[i]);
		}
		// Now we have our message in an array.
		// Move head to specific point:
		// if moving to a left table
		if (messageArray[0] == 1) {
			for (char i = 1; i < 10; i++)
			{
				if (messageArray[i] == 1) {
					tempArray[i] = messageArray[i];

					if ((tempArray[1] == 1) || (tempArray[4] == 1) || (tempArray[7] == 1)) { // move table in x-axis
						Serial.println("TableGoLeft()");
					}
					else if ((tempArray[2] == 1) || (tempArray[5] == 1) || (tempArray[8] == 1)) {
						Serial.println("TableGoCenter()");
					}
					else if ((tempArray[3] == 1) || (tempArray[6] == 1) || (tempArray[9] == 1)) {
						Serial.println("TableGoRight()");
					}
					else {
					}

					if ((tempArray[1] == 1) || (tempArray[2] == 1) || (tempArray[3] == 1)) {  // move table in y-axis
					  //TableGoLeft()
						Serial.println("TableGoUp()");
						Serial.println("Next movement incoming. ");
					}
					else if ((tempArray[4] == 1) || (tempArray[5] == 1) || (tempArray[6] == 1)) {
						Serial.println("TableGoCenter()");
						Serial.println("Next movement incoming. ");
					}
					else if ((tempArray[7] == 1) || (tempArray[8] == 1) || (tempArray[9] == 1)) {
						Serial.println("TableGoDown(1)");
						Serial.println("Next movement incoming. ");
					}
					else {
						Serial.print("eror, lol ");
						//        for (int i = 0; i < 10; i++)
						//        {
						//          Serial.print(messageArray[i]);
						//        }
						//        Serial.println(" ");
					}

				}
				else {
					Serial.println("Next iteration. ");
				}
				tempArray[i] = 0;
			}
		}
		else {
			Serial.println("Wrong table side choosen. Check input.");
		}

		if (messageArray[0] == 2) {    //desni stol


		}
		else












			break;
	case 2:
		Serial.println("Jog mode selected");
		break;
	default:
		Serial.println("Please select a valid option [1-Auto mode, 2-Jog mode].");
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

				if (digitalRead(C3_izvucen) == 1 && digitalRead(C4_izvucen) == 1) {
					isMoving = false;
					Serial.println("Move completed.");
				}
			}
		}

	}
	else if (tableSide == 1) {

		if (isMoving == false) {
			isMoving = true;
			Serial.print("Moving left table up. ");

			digitalWrite(C1_cilindar, HIGH);
			digitalWrite(C2_cilindar, HIGH);
			delay(cooldown);

			Serial.print("Waiting for input from sensors on C1 and C2. ");
			while (isMoving == true) {

				if (digitalRead(C1_izvucen) == 1 && digitalRead(C2_izvucen) == 1) {
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

				if (digitalRead(C1_uvucen) == 1 && digitalRead(C2_uvucen) == 1) {
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

	if (isMoving == false && (isUp == true))  //maybe add "|| handSlot[0] == 0"
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

void StartPressed() {
	Serial.println("Start pressed.");
	startPressed = HIGH;

	digitalWrite(LED_Start, LOW);
	digitalWrite(LED_Stop, LOW);

	Mb.R[0] = 1;
	Mb.R[1] = 0;

}

void StopPressed() {
	Serial.println("Stop pressed.");
	Mb.R[1] = 1;
	digitalWrite(LED_Stop, HIGH);
}

bool isTableEmpty(byte tableSide) {
	bool notEmpty = false;
	if (tableSide == 1) {

		for (char i = 1; i < 10; i++) {
			if ((messageArray[i] == 1) && (notEmpty == false)) {
				notEmpty = true;
			}
		}
		if (notEmpty == true) {
			return true;
		}
		else {
			return false;
		}

	}
	else if (tableSide == 2) {
		for (char i = 1; i < 10; i++) {

			if ((messageArray[i] == 1) && (notEmpty == false)) {
				notEmpty = true;
			}

		}
		if (notEmpty == true) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}


}

