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

int cooldown = 1000;                // Time between movement/valve activations in ms
int counter = 0;
char selectMode;

unsigned long longValue = 111111111; // za izbrisati
unsigned long message;

uint32_t loWord, hiWord;
int messageArray[10];

int	messageArray1[10] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
byte delete1 = 0;
bool digiRead = true; //pleae delete immediately

unsigned int temp1 = 0;
unsigned int temp2 = 0;
byte i;
byte pawnTemp;						// Store position of a temporarly selected pawn
byte tableSide;
String arrayPart;
String debugMessage = " ";
String messageString;
byte selectHand = 1;
byte handSlot[1];                  // Defines if slot 0 (lower postion) or 1 (upper position) is empty/full
byte workMode = 0;                 // Select work mode (1-Auto, 2-Jog)

bool isUp = false;                 // Body is or isn't in elevated position.    
bool isMoving = false;             // The manipulator is or isn't moving.
bool isHandFull = false;

byte tableLeft[10] = { NULL, 1, 1, 1, 1, 1, 1, 1, 1, 1 };//Actual state left table
byte tableRight[10] = { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0 };//Actual state right table
int tempArray[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

byte selectTable = Mb.R[10];

//Pin configuration:
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

const byte pawnGrabbed_V1 = 10;          // DI2   senzor vakuum 1
const byte pawnGrabbed_V2 = 11;          // DI3   senzor vakuum 2

const byte interruptStartPin = 18;       // IN0   interrupt ulaz, Start tipka
volatile byte startPressed = LOW;

const byte interruptStopPin = 19;        // IN1   interrupt ulaz, Stop tipka
volatile byte stopPressed = LOW;

void setup() {
	uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x51, 0x06 };
	uint8_t ip[] = { 161, 53, 117, 212 };
	uint8_t gateway[] = { 161, 53, 116, 1 };
	uint8_t subnet[] = { 255, 255, 252, 0 };
	Ethernet.begin(mac, ip, subnet);

	delay(2000);

	Serial.begin(9600);
	Serial.print("Started. ");
	Serial.print("My IP address: ");
	Serial.println(Ethernet.localIP());



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

	attachInterrupt(digitalPinToInterrupt(interruptStartPin), StartPressed, FALLING);
	attachInterrupt(digitalPinToInterrupt(interruptStopPin), StopPressed, RISING);

	for (i = 0; i < 125; i++) {                                 // Set every Modbus register value to 0
		Mb.R[i] = 0;
	}
	Serial.println("Modbus registers set to 0.");
}

//////////////////////////////// MAIN //////////////////////////////////////////////
void loop() {

	while (Mb.R[0] != 1) {
		if (debugMessage != "Ready to start. ") {
			debugMessage = "Ready to start. ";
			Serial.println(debugMessage);
		}
		digitalWrite(LED_Start, 0);
		delay(500);
		Mb.Run();
		digitalWrite(LED_Start, 1);
		delay(500);
		Mb.Run();
	}

	Mb.Run();
	//možda else if bolje umjesto switch??
	if (isHandFull == true) {
		Mb.R[3] = 2;
	}
	switch (Mb.R[3]) {
	case 1:
		if (debugMessage != "Auto-mode selected.") {
			debugMessage = "Auto-mode selected. ";
			Serial.println(debugMessage);
		}

		temp1 = Mb.R[5];
		temp2 = Mb.R[6];
		message = makeLong(temp1, temp2);
		messageString = String(message); //String(message);


		Serial.print(" The re-converted received message is: ");
		Serial.println(messageString);

		if (messageString.length() == 9) {
			for (char i = 0; i < 9; i++) {
				arrayPart = messageString.charAt(i);
				messageArray[i + 1] = arrayPart.toInt();
			}
			messageArray[0] = 0;
		}
		else if (messageString.length() < 9) {
			for (char i = 0; i < messageString.length(); i++) {
				arrayPart = messageString.charAt(i);
				messageArray[i + (10 - messageString.length())] = arrayPart.toInt();
			}
			for (char i = 1; i < (10 - messageString.length()); i++) {
				messageArray[i] = 0;
			}
			messageArray[0] = NULL;
			//0 000 000 001
		}

		Serial.print("Our message Int array is: ");    //printanje arraya
		for (int i = 1; i < 10; i++)
		{
			Serial.print(messageArray[i]);
		} Serial.println(" ");


		if (messageString.length() < 10) {
			if (Mb.R[9] == 1) {
				Serial.println("Filling table 1.");
				for (byte j = 1; j < 10; j++) {

					if (messageArray[j] == 1) {
						//ako već nema figura tamo na tom mjestu:
						if (tableLeft[j] != 1) {
							//nadji prvu dostupnu desno i odi tamo
							pawnTemp = FindAvailablePawn(2); //dobit ćemo broj od 1-9
							if (pawnTemp != 0) {
								RotateRight();
								GoTo(2, pawnTemp);
								PawnPickUpNeo();
								tableRight[pawnTemp] = 0;

								//nazad na željeni stol
								RotateLeft();
								GoTo(1, j);

								PawnDrop();
								tableLeft[j] = 1;

							}
							else {
								Serial.println("No pawns available at right table");
							}
						}
					}
					else if (messageArray[j] == 0) {
						if (tableLeft[j] == 1) {
							pawnTemp = FindFreeSpot(2); //dobit ćemo broj od 1-9
							if (pawnTemp != 0) {
								RotateLeft();
								GoTo(1, j);
								PawnPickUpNeo();
								tableLeft[1] = 0;

								//nazad na željeni stol
								RotateRight();
								GoTo(2, pawnTemp);
								PawnDrop();
								tableRight[pawnTemp] = 1;
							}
							else {
								Serial.println("No pawns available at right table");
							}
						}
					}
				}
				if (debugMessage != "Auto-mode transfer completed.") {
					debugMessage = "Auto-mode transfer completed.";
					Serial.println(" ");
					Serial.println(debugMessage);
				}
			}
			else if (Mb.R[9] == 2) {
				Serial.println("Filling table 2.");
				for (byte j = 1; j < 10; j++) {

					if (messageArray[j] == 1) {
						//ako već nema figura tamo na tom mjestu
						if (tableRight[j] != 1) {
							//nadji prvu dostupnu lijevo i odi tamo
							pawnTemp = FindAvailablePawn(1);
							if (pawnTemp != 0) {
								RotateLeft();
								GoTo(1, pawnTemp);
								PawnPickUpNeo();
								tableLeft[pawnTemp] = 0;

								//nazad na trazeni stol
								RotateRight();
								GoTo(2, j);
								PawnDrop();
								tableRight[j] = 1;
							}
							else {
								Serial.println("No pawns available at right table");
							}
						}
					}
					else if (messageArray[j] == 0) {
						if (tableRight[j] == 1) {
							pawnTemp = FindFreeSpot(1); //dobit ćemo broj od 1-9
							if (pawnTemp != 0) {
								RotateRight();
								GoTo(1, j);
								PawnPickUpNeo();
								tableRight[j] = 0;

								RotateLeft();
								GoTo(2, pawnTemp);
								PawnDrop();
								tableLeft[pawnTemp] = 1;
							}
							else {
								Serial.println("No pawns available at right table");
							}
						}
					}
				}
				if (debugMessage != "Auto-mode transfer completed.") {
					debugMessage = "Auto-mode transfer completed.";
					Serial.println(" ");Serial.println(debugMessage);Serial.println(" ");

				}
			}
			else {
				Serial.println("Wrong table side choosen. Check input.");
			}


		}
		Mb.R[3] = 10;

		break;
	case 2:
		if (debugMessage != "Point-to-point mode. Please select a table side (1 or 2) and pawn position (1-9)") {
			debugMessage = "Point-to-point mode. Please select a table side (1 or 2) and pawn position (1-9)";
			Serial.println(debugMessage);
		}
		if ((Mb.R[9] == 1) || (Mb.R[9] == 2)) {
			if ((Mb.R[10] >= 1) && (Mb.R[10] <= 9)) {
				//dodati debug komentare što više, 
				if (Mb.R[9] == 1) {

					if ( (isHandFull == true) && (IsSpotEmpty(1, Mb.R[10]) == true) ) {
						RotateLeft();
						GoTo(1, Mb.R[10]);
						PawnDrop();
						tableLeft[Mb.R[10]] = 1;

						Mb.R[9] = 2;
						Mb.R[10] = 0;
						isHandFull == false;
					}
					else if (IsSpotEmpty(1, Mb.R[10]) == false) {
						RotateLeft();
						GoTo(1, Mb.R[10]);
						PawnPickUpNeo();
						tableLeft[Mb.R[10]] = 0;

						Mb.R[9] = 2;
						Mb.R[10] = 0;
						isHandFull = true;
					}
					else {
						Serial.println("The choosen position on the left table is empty.");
					}
				}
				else if (Mb.R[9] == 2) {
					if ((isHandFull == true) && (IsSpotEmpty(2, Mb.R[10]) == true)) {
						RotateRight();
						GoTo(2, Mb.R[10]);
						PawnDrop();
						tableRight[Mb.R[10]] = 1;

						Mb.R[9] = 1;
						Mb.R[10] = 0;
						isHandFull == false;
					}
					else if (IsSpotEmpty(2, Mb.R[10]) == false) {
						RotateRight();
						GoTo(2, Mb.R[10]);
						PawnPickUpNeo();
						tableRight[Mb.R[10]] = 0;

						Mb.R[9] = 1;
						Mb.R[10] = 0;
						isHandFull = true;
					}
					else {
						if (debugMessage != "Point-to-point mode. Please select a table side (1 or 2) and pawn position (1-9)") {
							debugMessage = "Point-to-point mode. Please select a table side (1 or 2) and pawn position (1-9)";
							Serial.println(debugMessage);
						}
					}
				}

			}
		}

		break;
	case 3:
		if (Mb.R[21] == 1) {
			if (Mb.R[20] == 1) {
				TableGoUp(1);
				Mb.R[21] = 0;
			}
			else if (Mb.R[20] == 2) {
				TableGoUp(2);
				Mb.R[21] = 0;
			}
			else {
				if ((debugMessage != "Please select a proper table number. Number received: ") && (Mb.R[20] != 0)) {
					debugMessage = "Please select a proper table number. Number received: ";
					Serial.print(debugMessage);
					Serial.println(Mb.R[20]);
				}
			}
		}
		if (Mb.R[22] == 1) {
			if (Mb.R[20] == 1) {
				TableGoDown(1);
				Mb.R[22] = 0;
			}
			else if (Mb.R[20] == 2) {
				TableGoDown(2);
				Mb.R[22] = 0;
			}
			else {
				if ((debugMessage != "Please select a proper table number. Number received: ") && (Mb.R[20] != 0)) {
					debugMessage = "Please select a proper table number. Number received: ";
					Serial.print(debugMessage);
					Serial.println(Mb.R[20]);
				}
			}
		}
		break;
	case 4:
		
		break;
	default:
		if (debugMessage != "Please select a mode [1-Auto, 2 - Point-to-point, 3-Jog].") {
			debugMessage = "Please select a mode [1-Auto, 2 - Point-to-point, 3-Jog].";
			Serial.println(debugMessage);
		}
		break;
	}



}



//////////////////////////////// FUNCTIONS //////////////////////////////////////////////

//////////////////////////////// FOR JOG MODE
void TableGoRight() {


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

void TableGoLeft() {
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
			Serial.print("Going right. ");

			digitalWrite(CONTROLLINO_R6, LOW);
			digitalWrite(CONTROLLINO_R5, HIGH);
			delay(cooldown);

			//Serial.print("Move status: %b, isM ");

			while (isMoving == true) {
				Serial.print("Waiting for input from sensors C5 and C6... ");
				if (digitalRead(C5_izvucen) == 1 && digitalRead(C6_uvucen) == 1) {
					isMoving = false;
					Serial.println("Move completed.");
				}
			}

		};
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
			Serial.print("Going right. ");

			digitalWrite(CONTROLLINO_R6, LOW);
			digitalWrite(CONTROLLINO_R5, HIGH);
			delay(cooldown);

			//Serial.print("Move status: %b, isM ");

			while (isMoving == true) {
				Serial.print("Waiting for input from sensors C5 and C6... ");
				if (digitalRead(C5_izvucen) == 1 && digitalRead(C6_uvucen) == 1) {
					isMoving = false;
					Serial.println("Move completed.");
				}
			}

		};
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
		Serial.print("The hand is up. Rotating to the left.");

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
		Serial.print("The hand is down. Rotating to the left.");

		digitalWrite(C8_cilindar, LOW);
		delay(cooldown);

		isUp = true;

		digitalWrite(C7_cilindar, LOW);
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

void GrabPawn() {
	if (isMoving == false) {

		isMoving = true;
		Serial.print("Grabbing the pawn with vacuum.. ");

		digitalWrite(Vacuum_1, HIGH);
		delay(cooldown);

		Serial.print("Waiting for input from vacuum sensor 1.. ");
		while (isMoving == true) {

			if (digitalRead(pawnGrabbed_V1) == 1) {
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
	Mb.R[0] = 0;
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
//////////////////////////////// FOR AUTO MODE
bool IsSpotEmpty(byte tableSide, byte pawnPosition) {
	//treba testirat ovo
	if (tableSide == 1) {
		if (tableLeft[pawnPosition] == 1) {
			return false;
		}
		else {
			return true;
		}
	}
	else if (tableSide == 2) {
		if (tableRight[pawnPosition] == 1) {
			return false;
		}
		else {
			return true;
		}
	}
	else {
		Serial.println("Wrong table side choosen for checking if spot is empty.");
	}
}
void GoTo(byte tableSide, byte pawnPosition) {

	if (tableSide == 1) {

		if ((pawnPosition == 1) || (pawnPosition == 4) || (pawnPosition == 7)) { // move table in x-axis
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
		else if ((pawnPosition == 2) || (pawnPosition == 5) || (pawnPosition == 8)) {
			if (isMoving == false) {

				isMoving = true;
				Serial.print("Going right. ");

				digitalWrite(CONTROLLINO_R6, LOW);
				digitalWrite(CONTROLLINO_R5, HIGH);
				delay(cooldown);

				//Serial.print("Move status: %b, isM ");

				while (isMoving == true) {
					Serial.print("Waiting for input from sensors C5 and C6... ");
					if (digitalRead(C5_izvucen) == 1 && digitalRead(C6_uvucen) == 1) {
						isMoving = false;
						Serial.println("Move completed.");
					}
				}

			};
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
		else if ((pawnPosition == 3) || (pawnPosition == 6) || (pawnPosition == 9)) {
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

			};
		}

		if ((pawnPosition == 1) || (pawnPosition == 2) || (pawnPosition == 3)) {
			if (isMoving == false) {
				isMoving = true;
				Serial.print("Moving left table up. ");

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
		else if ((pawnPosition == 4) || (pawnPosition == 5) || (pawnPosition == 6)) {
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
		else if ((pawnPosition == 7) || (pawnPosition == 8) || (pawnPosition == 9)) {
			if (isMoving == false) {
				isMoving = true;
				Serial.print("Moving left table down. ");

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
		else {
			Serial.print("Incorrect format of requested position. Req. position was: ");
			for (int i = 0; i < 10; i++)
			{
				Serial.print(messageArray[i]);
			}
			Serial.println(" ");
		}

	}
	else if (tableSide == 2) {

		if ((pawnPosition == 1) || (pawnPosition == 4) || (pawnPosition == 7)) {
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
		else if ((pawnPosition == 2) || (pawnPosition == 5) || (pawnPosition == 8)) {
			if (isMoving == false) {

				isMoving = true;
				Serial.print("Going right. ");

				digitalWrite(CONTROLLINO_R6, LOW);
				digitalWrite(CONTROLLINO_R5, HIGH);
				delay(cooldown);

				//Serial.print("Move status: %b, isM ");

				while (isMoving == true) {
					Serial.print("Waiting for input from sensors C5 and C6... ");
					if (digitalRead(C5_izvucen) == 1 && digitalRead(C6_uvucen) == 1) {
						isMoving = false;
						Serial.println("Move completed.");
					}
				}

			};
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
		else if ((pawnPosition == 3) || (pawnPosition == 6) || (pawnPosition == 9)) {
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

		if ((pawnPosition == 1) || (pawnPosition == 2) || (pawnPosition == 3)) {
			if (isMoving == false) {
				isMoving = true;
				Serial.print("Moving right table up. ");

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
		else if ((pawnPosition == 4) || (pawnPosition == 5) || (pawnPosition == 6)) {
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
		else if ((pawnPosition == 7) || (pawnPosition == 8) || (pawnPosition == 9)) {
			if (isMoving == false) {
				isMoving = true;
				Serial.print("Moving right table down. ");

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
		else {
			Serial.print("Incorrect format of requested position. Req. position was: ");
			for (int i = 0; i < 10; i++)
			{
				Serial.print(messageArray[i]);
			}
			Serial.println(" ");
		}
	}
	else {
		Serial.println("Wrong table side number choosen.");
	}



}
void PawnPickUpNeo() {

	Serial.print("Initiating vacuum activation. ");

	while (digitalRead(pawnGrabbed_V1) != 1) { // staviti OR ako nije start upaljen
		digitalWrite(C8_cilindar, HIGH); // !!! testirati u real life da li tu ide LOW!!!
		delay(cooldown);
		digitalWrite(Vacuum_1, HIGH);
		delay(cooldown);
		digitalWrite(C8_cilindar, LOW);
		delay(cooldown);
	}

	digitalWrite(C9_cilindar, HIGH);
	delay(cooldown);
	Serial.print("Pawn is picked up. ");
	isHandFull = true;

}
void PawnDrop() {

	digitalWrite(C9_cilindar, LOW);
	delay(cooldown);
	delay(cooldown);
	digitalWrite(C8_cilindar, HIGH); // !!! testirati u real life da li ce se spustiti dolje!!!
	delay(cooldown);

	Serial.print("Initiating vacuum de-activation. ");
	digitalWrite(Vacuum_1, LOW); //vacuum isključiti ovdje  // !!! testirati koji suction je pod brojem 1 u real life !!!


	digitalWrite(C8_cilindar, LOW);

	delay(cooldown);
	isHandFull = false;

}
byte FindAvailablePawn(byte tableSide) {
	byte pawn = 0;
	byte n = 1;
	byte i = 0;
	//možda bude problem pretvoriti INT u BYTE
	if (tableSide == 1) {
		while (i == 0) {
			if (tableLeft[n] == 1) {
				i = 1;
				pawn = n;
			}
			n = n + 1;
		}
	}
	else if (tableSide == 2) {
		while (i == 0) {
			if (tableRight[n] == 1) {
				i = 1;
				pawn = n;
			}
			n = n + 1;
		}
	}
	else {
		pawn = 0;
	}
	return pawn;
}
byte FindFreeSpot(byte tableSide) {
	byte pawn;
	byte n = 0;
	//možda bude problem pretvoriti INT u BYTE
	if (tableSide == 1) {
		while (n == 0) {
			for (i = 1; i < 10; i++) {
				if (tableLeft[i] == 0) {
					n = i;
					pawn = i;
				}
				else if (tableLeft[9] == 1) {
					n = 1;
					pawn = 0;
				}
			}
		}
	}
	else if (tableSide == 2) {
		while (n == 0) {
			for (i = 1; i < 10; i++) {
				if (tableRight[i] == 0) {
					n = i;
					pawn = i;
				}
				else if (tableRight[9] == 1) {
					n = 1;
					pawn = 0;
				}
			}
		}
	}
	else {
		pawn = 0;
	}
	return pawn;
}

