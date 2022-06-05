#include "MFRC522.h"
#include "Particle.h"
#include "neopixel/neopixel.h"


SYSTEM_MODE(AUTOMATIC);

#define RST_PIN A2
#define SS_PIN A5

#define YELLOW_LED D0
#define GREEN_LED D1
#define F_SERVO_PIN D7
#define BLUE_LED D3
#define SWITCH_PIN D4
#define L_SERVO_PIN D5
#define BUZZER_PIN D6

#define PIXEL_COUNT 12
#define PIXEL_PIN D2
#define PIXEL_TYPE WS2812



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
int timeCounter;
bool inside;
bool initSetup;

//construct RFID and LED ring hardware objects:
MFRC522 mfrc522(SS_PIN, RST_PIN);
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Method/Functions pre set:
void buzzerToggle(const char *event, const char *data);
bool CardRead(unsigned long &card);
unsigned long getID();
void ExpelFeed();
void OpenLid();
unsigned long InitFeeder();
void PetInside();
void PetOutside();
void SetSchedule();
void SetLEDStrip();
void FeedPet();


void setup()
{
  //init LED ring
  strip.begin();
  strip.setBrightness(50);
  strip.show();

  //init time and setup functionality
  timeCounter = 1;
  initSetup = false;
  Time.zone(-2);
  Serial.begin(9600);

  // init RFID sensor
  mfrc522.setSPIConfig();
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RFCfgReg);
  Serial.println(card);

  //init pin modes
  feedServo.attach(F_SERVO_PIN);
  lidServo.attach(L_SERVO_PIN);
  pinMode(SWITCH_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  //Particle cloud subscriptions for door device communication
  Particle.subscribe("Pet_status", buzzerToggle);
}

void loop()
{ 
  if (!initSetup){card = InitFeeder();}//checks if pet tag/card has been registered to devices
  SetSchedule();//Sets the feed schedule
  FeedPet();//Checks if card has been read and serves food if time == schedule
  delay(100);
}

/////////////////////////////////////////////////////////////////
void SetSchedule()
{ 
  switchState = digitalRead(SWITCH_PIN);
  if(switchState == HIGH)
  {
    timeCounter += 1;
     if(timeCounter > 12)
     {
       timeCounter = 1;
     }
    SetLEDStrip();
    Serial.println(timeCounter);
    tone(BUZZER_PIN, 200, 200);
    delay(200);
  }
}

void SetLEDStrip()
{
  strip.clear();
  strip.setPixelColor(timeCounter, 0,0,255);
  strip.show();
}

void FeedPet()
{
  if (CardRead(card))
  {
    if(timeCounter == Time.hourFormat12())
    {
      ExpelFeed();
      OpenLid();
      Particle.publish("Pet_Fed", "true");
    }
    else
    {
      tone(BUZZER_PIN, 600, 200);
      delay(200);
      tone(BUZZER_PIN, 600, 200);
      delay(200);
      Particle.publish("Pet_Fed", "false");
    }
  }
}

///////////////////////////////////////////////

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

    return false;
  }
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
  feedServo.write(90);
  delay(5000);
  feedServo.detach();
  delay(2000);
}

void OpenLid()
{
  lidServo.write(110);
  delay(30000);
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
    strip.clear();
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
