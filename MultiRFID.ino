/**
  *
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS 1    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
 * SPI SS 2    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 */

/*  Wiring up the RFID Readers ***
 *  RFID readers based on the Mifare RC522 like this one:  http://amzn.to/2gwB81z
 *  get wired up like this:
 *
 *  RFID pin    Arduino pin (above)
 *  _________   ________
 *  SDA          SDA - each RFID board needs its OWN pin on the arduino
 *  SCK          SCK - all RFID boards connect to this one pin
 *  MOSI         MOSI - all RFID boards connect to this one pin
 *  MISO         MISO - all RFID boards connect to this one pin
 *  IRQ          not used
 *  GND          GND - all RFID connect to GND
 *  RST          RST - all RFID boards connect to this one pin
 *  3.3V         3v3 - all RFID connect to 3.3v for power supply
 *
 */


#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see typical pin layout above

//each SS_x_PIN variable indicates the unique SS pin for another RFID reader
#define SS_1_PIN        10         // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 2
#define SS_2_PIN        8          // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 1
#define SS_3_PIN        7          // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 1


//must have one SS_x_PIN for each reader connected
#define NR_OF_READERS   3

byte ssPins[] = {SS_1_PIN, SS_2_PIN,SS_3_PIN};

MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.
String read_rfid;

//these are hard coded "right" card reads. You will have to change these to match
//cards you have in inventory - even better this code should be updated
//so that you can store new valid cards in EEProm.

String ValidCard_1 = "8cc1ad85";
String ValidCard_2 = "5efc1f2b";
String ValidCard_3 = "1196b85";

boolean Card_1_ok = false;
boolean Card_2_ok = false;
boolean Card_3_ok = false;

//this is the pin the relay is on, change as needed for your code
#define DoorLockPin 4; 


int Read1 = 9;
int Read2 = 9;
boolean ValidCardOrder = true;
String Prevcard = "";
int NoCardCnt = 0;
int DoorOpenSec = 2;

/**
 * Initialize.
 */
void setup() {
  pinMode(DoorLockPin,OUTPUT);
  digitalWrite(DoorLockPin,HIGH);
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  SPI.begin();        // Init SPI bus

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
      }
}

/**
 * Main loop.
 */
void loop() {
  
  if (NoCardCnt > 150){
    Card_1_ok = false;
    Card_2_ok = false;
    Card_3_ok = false;  
    ValidCardOrder = true;
    digitalWrite(DoorLockPin,HIGH);
    NoCardCnt = 0;
    Prevcard = "";
    Read1 = 9;
    Read2 = 9;
  }  
   
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // Look for new cards  
    
          
    //   
    //Serial.println("Reading readers" + String(ValidCardOrder) + "-----" + String(Card_1_ok) + "--" + String(Card_2_ok) + "--" + String(Card_3_ok));   

    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      // Show some details of the PICC (that is: the tag/card)
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println(read_rfid);
      
      ValidateCard(reader);
      
      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    }
    else{
      NoCardCnt = NoCardCnt + 1;
      delay(50);
      //Serial.println("No card on sensor cnt"  + String(NoCardCnt));
    }  

     } 
}



void dump_byte_array(byte *buffer, byte bufferSize) {
  read_rfid = "";
  for (byte i = 0; i < bufferSize; i++) {
    read_rfid = read_rfid + String(buffer[i], HEX);
  }
}



void ValidateCard(int reader) {
   if (reader == Read1) { 
       Serial.println("Reader 1 already reads"); 
       goto GoNext;}
    if (reader == Read2) { 
       Serial.println("Reader 2 already reads");    
       goto GoNext;} 
       
   int x; 
   Prevcard = read_rfid;
    if (Card_1_ok == false && Card_2_ok == false && Card_3_ok == false){
      if (ValidCard_1 == read_rfid){
          Read1 = reader;
          Card_1_ok = true; 
          NoCardCnt = 0;
          goto GoNext;           
      } 
      else{
          ValidCardOrder = false;
          goto GoNext;      
      } 
    }
 
    if (Card_1_ok == true && Card_2_ok == false && Card_3_ok == false){
      if (ValidCard_2 == read_rfid){
          Read2 = reader;
          Card_2_ok = true; 
          NoCardCnt = 0;
          goto GoNext;     
      } 
      else{
          ValidCardOrder = false;
          goto GoNext;      
      } 
    }
 
    if (Card_1_ok == true && Card_2_ok == true && Card_3_ok == false){
      if (ValidCard_3 == read_rfid){
          Card_3_ok = true;   
          if (ValidCardOrder) { 
              open_lock();              
           }  
      } 
      else{
          ValidCardOrder = false;      
      } 
    }
    
GoNext:  
    x = 0;
 
}  


void open_lock(){
  //Serial.println("****************************************** Door opened");
  digitalWrite(DoorLockPin,LOW);
  delay(DoorOpenSec*1000);
  digitalWrite(DoorLockPin,HIGH);
  Card_1_ok = false;
  Card_2_ok = false;
  Card_3_ok = false;  
  ValidCardOrder = true;
  NoCardCnt = 0;
  Prevcard = "";
}  
 
