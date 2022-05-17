#include "MFRC522.h"

#define RST_PIN A2
#define SS_PIN A5

#define YELLOW_LED D0
#define GREEN_LED D1
#define F_SERVO_PIN D2
#define BLUE_LED D3
#define SWITCH_PIN D4
#define L_SERVO_PIN D5
#define BUZZER_PIN D6

/*
 * Project safePetSystem
 * Description:
 * Author: Thomas Morgan
 * Date:
 */

Servo feedServo;
Servo lidServo;
unsigned long card;
unsigned long uid;
int switchState;
bool inside;
bool initSetup;

MFRC522 mfrc522(SS_PIN, RST_PIN);

void buzzerToggle(const char *event, const char *data);
bool CardRead(unsigned long &card);
unsigned long getID();
void ExpelFeed();
void OpenLid();
unsigned long InitFeeder();
void PetInside();
void PetOutside();

void setup()
{
  initSetup = false;
  Time.zone(-2);
  Serial.begin(9600);

  mfrc522.setSPIConfig();
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RFCfgReg);
  Serial.println(card);

  feedServo.attach(F_SERVO_PIN);
  lidServo.attach(L_SERVO_PIN);
  pinMode(SWITCH_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  Particle.subscribe("Pet_status", buzzerToggle);
}

void loop()
{
  switchState = digitalRead(SWITCH_PIN);
  if(!initSetup)
  {
    card = InitFeeder();
  }

  if (switchState == HIGH)
  {
    ExpelFeed();
    Serial.println("Button Pressed");
    Serial.print(Time.hourFormat12());
    Serial.println(uid);
  }

  if (CardRead(card))
  {
    ExpelFeed();
    OpenLid();
    Particle.publish("Pet_Fed", "true");
  }
  delay(300);
}

/////////////////////////////////////////////////////////////////

void buzzerToggle(const char *event, const char *data)
{

  std::string temp = (std::string)data;
  Particle.publish(data);

  if (temp == "inside")
  {
    PetInside();
  }
  if (temp == "outside")
  {
    PetOutside();
  }
}

void PetInside()
{
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  tone(BUZZER_PIN, 200, 200);
  delay(200);
  tone(BUZZER_PIN, 300, 200);
  delay(200);
  tone(BUZZER_PIN, 400, 200);
  delay(200);
  tone(BUZZER_PIN, 500, 200);
  delay(200);
  tone(BUZZER_PIN, 600, 200);
  delay(200);
  Serial.print("inside");
}

void PetOutside()
{
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  tone(BUZZER_PIN, 600, 200);
  delay(200);
  tone(BUZZER_PIN, 500, 200);
  delay(200);
  tone(BUZZER_PIN, 400, 200);
  delay(200);
  tone(BUZZER_PIN, 300, 200);
  delay(200);
  tone(BUZZER_PIN, 200, 200);
  Serial.print("outisde");
}

// Scans for Card ID
bool CardRead(unsigned long &card)
{

  if (mfrc522.PICC_IsNewCardPresent())
  {
    uid = getID();

    Serial.print("Card detected, UID: ");
    Serial.println(uid);
    if (card == uid)
    {
      PetInside();
      return true;
    }
    Particle.publish("Pet_Fed", "false");
    return false;
  }
  Particle.publish("Pet_Fed", "false");
  return false;
}


// get RFID tag ID
unsigned long getID()
{
  if (!mfrc522.PICC_ReadCardSerial())
  { // Since a PICC placed get Serial and continue
    return -1;
  }
  unsigned long int hex_num;
  hex_num = mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] << 8;
  hex_num += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); // Stop reading
  return hex_num;
}


void ExpelFeed()
{

  feedServo.attach(F_SERVO_PIN);
  feedServo.write(180);
  delay(2000);
  feedServo.detach();
  delay(2000);
}


void OpenLid()
{
  lidServo.write(110);
  delay(15000);
  lidServo.write(0);
}


unsigned long InitFeeder()
{
  Serial.println("Scan new card: ");
  do
  {
    digitalWrite(BLUE_LED, HIGH);
    delay(500);
    digitalWrite(BLUE_LED, LOW);
    delay(500);
  } while (!mfrc522.PICC_IsNewCardPresent());

  tone(BUZZER_PIN, 500, 2000);
  Serial.print("Done!");
  card = getID();
  std::string s = std::to_string(card);
  char const *pchar = s.c_str();
  Particle.publish("Card_ID", pchar);
  initSetup = true;
  return card;
}
