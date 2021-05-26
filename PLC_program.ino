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

int cooldown = 600;                // Time between movement/valve activations in ms
int counter = 0;
char selectMode;

unsigned long longValue = 111111111;
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
String messageString;
byte selectHand = 1;
byte handSlot[1];                  // Defines if slot 0 (lower postion) or 1 (upper position) is empty/full
byte workMode = 0;                 // Select work mode (1-Auto, 2-Jog)

bool isUp = false;                 // Body is or isn't in elevated position.    
bool isMoving = false;             // The manipulator is or isn't moving.

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
	Mb.R[0] = CONTROLLINO_IN0;
	pinMode(CONTROLLINO_IN1, INPUT);                            // IN1   tipka Stop
	Mb.R[1] = CONTROLLINO_IN1;

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

	for (i = 0; i < 125; i++) {                                 // Set every Modbus register value to 0
		Mb.R[i] = 0;
	}
	Serial.println("Modbus registers set to 0.");
}

//////////////////////////////// MAIN //////////////////////////////////////////////
void loop() {

	while (Mb.R[0] != 1) {
		digitalWrite(LED_Start, 0);
		delay(500);
		Mb.Run();
		digitalWrite(LED_Start, 1);
		delay(500);
		Mb.Run();
	}

	Mb.Run();

	switch (Mb.R[3]) {
	case 1:
		Serial.println(" Auto mode selected.");
		// 1 111 111 111 dolazi:
		temp1 = Mb.R[5];
		temp2 = Mb.R[6];
		message = makeLong(temp1, temp2);
		messageString = 111111111; //String(message);
		Serial.print(" The re-converted received message is: ");
		Serial.println(messageString);

		if (messageString.length() == 9) {
			for (char i = 1; i < 11; i++) {
				arrayPart = messageString.charAt(i);
				messageArray[i] = arrayPart.toInt();
			}
			messageArray[0] = 0;
		}
		else if (messageString.length() < 9) {
			for (char i = 1; i < (10 - messageString.length()); i++) {
				messageArray[i] = 0;
			}
			for (char i = (10 - messageString.length()); i < messageString.length(); i++) {
				arrayPart = messageString.charAt(i);
				messageArray[i] = arrayPart.toInt();
			}
			messageArray[0] = 0;

		}

		Serial.print("Our message Int array is: ");    //printanje arraya
		for (int i = 0; i < 10; i++)
		{
			Serial.print(messageArray[i]);
		} Serial.println(" ");
		// Now we have our message in an array.

		Serial.println("Proceeding to move the head.");

		if (Mb.R[5] != 0) {
			//možda tu ubaciti konverziju dva 16bit u 32bit jedan
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

								PawnDrop;
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
								PawnDrop;
								tableRight[pawnTemp] = 1;
							}
							else {
								Serial.println("No pawns available at right table");
							}
						}
					}
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
								PawnDrop;
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
								tableRight[1] = 0;

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
			}
			else {
				Serial.println("Wrong table side choosen. Check input.");
			}

			Mb.R[5] = 0;
		}




		break;
	case 2:

		pawnTemp = 1;
		Serial.print("Found pawn on position: ");
		Serial.println(pawnTemp);

		if (pawnTemp != 0) {
			RotateLeft();
			GoTo(1, pawnTemp);
			PawnPickUpNeo();
			tableLeft[pawnTemp] = 0;

			//nazad na trazeni stol
			RotateRight();
			Serial.println("Finished rotating.");
			GoTo(2, 4);
			Serial.println("Finished going to selected pawn.");

			PawnDrop();
			Serial.println("Dropped the pawn for fs.");
		}
		else {
			Serial.println("No pawns available at right table");
		}



		/*Serial.println("Jog mode selected");
		temp1 = Mb.R[5];
		temp2 = Mb.R[6];
		message = makeLong(temp1, temp2);
		messageString = String(message);
		Serial.println(messageString);

		if (message == longValue) {
			Serial.println("Fackin success, mate!!!!1");
			digitalWrite(LED_Start, 1);
			digitalWrite(CONTROLLINO_D6, 1);

		}*/
		break;
	case 3:
		Serial.println("Filling table 2.");

		for (byte j = 1; j < 10; j++) {

			if (messageArray1[j] == 1) {
				//ako već nema figura tamo na tom mjestu
				if (tableRight[j] == 0) {
					Serial.println("Finding available pawn.");
					pawnTemp = FindAvailablePawn(1);
					Serial.print("Found pawn: ");
					Serial.println(pawnTemp);

					if (pawnTemp != 0) {
						RotateLeft();
						GoTo(1, pawnTemp);
						PawnPickUpNeo();
						tableLeft[pawnTemp] = 0;

						//nazad na trazeni stol
						RotateRight();
						Serial.println("Finished rotating.");
						GoTo(2, j);
						Serial.println("Finished going to selected pawn.");
						PawnDrop();
						Serial.println("Dropped the pawn.");
						tableRight[j] = 1;
					}
					else {
						Serial.println("No pawns available at right table");
					}
				}
			}
			else if (messageArray1[j] == 0) {
				if (tableRight[j] == 1) {
					pawnTemp = FindFreeSpot(1); //dobit ćemo broj od 1-9
					if (pawnTemp != 0) {
						RotateRight();
						GoTo(1, j);
						PawnPickUpNeo();
						tableRight[1] = 0;

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

		break;
	case 4:
		break;
	default:
		Serial.println("Please select a valid option [1-Auto mode, 2-Jog mode].");
	}



}



//////////////////////////////// FUNCTIONS //////////////////////////////////////////////

//////////////////////////////// FOR JOG MODE
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
void PawnPickUpBeta() {
	if (isMoving == false) {
		isMoving = true;

		digitalWrite(C8_cilindar, HIGH); // !!! testirati u real life da li tu ide LOW!!!
		delay(cooldown);

		isUp = false;
		isMoving = false;
	}
	//vacuum upaliti ovdje  // !!! testirati koji suction je pod brojem 1 u real life !!!
	if (isMoving == false) {
		isMoving = true;

		digitalWrite(C8_cilindar, LOW);
		delay(cooldown);

		isUp = true;
		isMoving = false;
	}
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
void PawnPickUpNeo() {
	if (isMoving == false) {
		isMoving = true;

		digitalWrite(C8_cilindar, HIGH); // !!! testirati u real life da li tu ide LOW!!!
		delay(cooldown);

		isUp = false;
		isMoving = false;
	}
	//vacuum upaliti ovdje  // !!! testirati koji suction je pod brojem 1 u real life !!!
	if (isMoving == false) {
		isMoving = true;
		Serial.print("Initiating vacuum activation. ");

		digitalWrite(Vacuum_1, HIGH);
		delay(cooldown);

		if (digitalRead(pawnGrabbed_V1) == 1) {
			isMoving = false;
			Serial.println("Move completed.");
		}
	}
	if (isMoving == false) {
		isMoving = true;

		digitalWrite(C8_cilindar, LOW);
		delay(cooldown);

		isUp = true;
		isMoving = false;
	}
	if (isMoving == false) {
		isMoving = true;

		digitalWrite(C9_cilindar, HIGH);
		delay(cooldown);

		isUp = true;
		isMoving = false;
	}

}
void PawnDrop() {

	digitalWrite(C9_cilindar, LOW);
	delay(cooldown);
	digitalWrite(C8_cilindar, HIGH); // !!! testirati u real life da li ce se spustiti dolje!!!
	delay(cooldown);

	Serial.print("Initiating vacuum de-activation. ");
	digitalWrite(Vacuum_1, LOW); //vacuum isključiti ovdje  // !!! testirati koji suction je pod brojem 1 u real life !!!


	digitalWrite(C8_cilindar, LOW);

	delay(cooldown);

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