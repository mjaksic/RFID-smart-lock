#include <SoftwareSerial.h>
#include "RDM6300.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

SoftwareSerial rdm_serial(3, 2);
RDM6300<SoftwareSerial> rdm(&rdm_serial);
RF24 radio(9, 10);
const byte address[6] = "00101";

#include <Adafruit_NeoPixel.h>
#define PIN        5
#define NUMPIXELS 24
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

bool flag_1, flag_2, isRead;
int ulazni, izlazni;
const int relayPin = 4, laserPin_1 = 7, laserPin_2 = 8;
const unsigned long my_ID = 0x003DE307, my_ID_2 = 0x00866D71;
unsigned long current_ID = 0, previous_ID = 0;

void setup()
{
  pinMode(relayPin, OUTPUT);
  pinMode(laserPin_1, OUTPUT);
  pinMode(laserPin_2, OUTPUT);
  digitalWrite(laserPin_1, HIGH);
  digitalWrite(laserPin_2, HIGH);
  SetLedColor(10, 0, 0);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  Serial.begin(115200);
}

void loop()
{
  delay(500);
  ulazni = analogRead(A0);
  izlazni = analogRead(A1);
  Serial.println(ulazni);
  Serial.println(izlazni);
  
  if(ulazni >= 450 && isRead == false && izlazni < 450)
  {
    current_ID = rdm.read();    
    isRead = true;
    
    if(previous_ID == current_ID)
    {
      isRead = false;
      previous_ID = 0;
    }

    else if (my_ID == current_ID || my_ID_2 == current_ID)
    { 
      flag_1 = true;
      previous_ID = current_ID;
      OpenDoor();
    }
    
    else if(my_ID != current_ID || my_ID_2 != current_ID)
    {
      previous_ID = current_ID;
      char text[] = "ID ERROR!";
      radio.write(&text, sizeof(text));
    }
  }
  
  else if(izlazni >= 450 && isRead == false && ulazni < 450)
    {
      current_ID = rdm.read();
      isRead = true;
      
      if(previous_ID == current_ID)
      {
        isRead = false;
        previous_ID = 0;
      }
      
      else if (my_ID == current_ID || my_ID_2 == current_ID)
      {
        flag_2 = true;
        previous_ID = current_ID;
        OpenDoor();
      }
      
      else if (my_ID != current_ID || my_ID_2 != current_ID)
      {
        previous_ID = current_ID;
        char text[] = "ID ERROR!";
        radio.write(&text, sizeof(text));
      }
    }
    
  else
  {
    flag_1 = 0, flag_2 = 0, isRead = false;
  }
    
  while(flag_1 == true)
  {
    izlazni = analogRead(A1);
    if (izlazni >= 300)
    {      
      flag_1 = false;
      char text[] = "User left!";
      radio.write(&text, sizeof(text));
      CloseDoor();
    }
  }
  
  while(flag_2 == true)
  {
    ulazni = analogRead(A0);
    if (ulazni >= 300)
    { 
      flag_2 = false;
      char text[] = "User entered!";
      radio.write(&text, sizeof(text));
      CloseDoor();
    }
  }
}

void SetLedColor(int r, int g, int b) {
  pixels.begin();
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
    pixels.show();
  }
}

void OpenDoor() {
  digitalWrite(relayPin, HIGH);
  SetLedColor(0, 10, 0);
  char text[] = "Door opened!";
  radio.write(&text, sizeof(text));
}

void CloseDoor() {
  digitalWrite(relayPin, LOW);
  SetLedColor(10, 0, 0);
  isRead = false;
}
