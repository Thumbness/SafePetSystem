#include "Arduino.h"
#include "MFRC522.h"

#define SS_PIN A5
#define RST_PIN A2

#define F_SERVO_PIN D2
#define L_SERVO_PIN D5
#define SWITCH_PIN D4

/*
 * Project safePetSystem
 * Description:
 * Author:
 * Date:
 */

Servo feedServo;
Servo lidServo;

unsigned long card;

bool inProximity;
int switchState;
MFRC522 mfrc522(SS_PIN, RST_PIN);


// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(9600);

  mfrc522.setSPIConfig();
  mfrc522.PCD_Init(); 
  Serial.println("Scan a MIFARE Classic PICC to demonstrate Value Blocks.");
  
  card = setupCard();
  Serial.print(card);
  // Put initialization like pinMode and begin functions here.
  feedServo.attach(F_SERVO_PIN);
  lidServo.attach(L_SERVO_PIN);
  pinMode(SWITCH_PIN, INPUT);
  

}

void loop() {
  switchState = digitalRead(SWITCH_PIN);
  if(switchState == HIGH)
  {
    ManualPress();
    Serial.print(card);
  }

  if(CardRead(card))
  {
    ManualPress();
    Serial.print(card);
  }

}
//Scans for Card ID
bool CardRead(unsigned long &card)
{
  if(mfrc522.PICC_IsNewCardPresent()) {
    unsigned long uid = getID();
      Serial.print("Card detected, UID: "); Serial.println(uid);
      if(card == uid)
      {
        return true;
      }
      return false;
    }
    return false;
}
//get RFID tag ID
unsigned long getID(){
  if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
    return -1;
  }
  unsigned long int hex_num;
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); // Stop reading
  return hex_num;
}
//press for servo motor function
void ManualPress(){
  Serial.print(switchState);
    feedServo.attach(F_SERVO_PIN);
    Serial.print(switchState);
    feedServo.write(180);
    lidServo.write(110);
    delay(2000);
    lidServo.write(0);
    feedServo.detach();
    delay(2000);
}

unsigned long setupCard()
{
  Serial.print("Setup Card: ");
  while(! mfrc522.PICC_IsNewCardPresent())
  {

  }
  Serial.print("Done!");
  unsigned long result = getID();
  return result;
}
