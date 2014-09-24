/*
 * IRrecord: record and play back IR signals as a minimal 
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * An IR LED must be connected to the output PWM pin 3.
 * A button must be connected to the input BUTTON_PIN; this is the
 * send button.
 * A visible LED can be connected to STATUS_PIN to provide status.
 *
 * The logic is:
 * If the button is pressed, send the IR code.
 * If an IR code is received, record it.
 *
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>
#include "Basics.h"

int RECV_PIN = A0;
int BUTTON_PIN = 12;
int STATUS_PIN = 13;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

void setup()
{
  irrecv.enableIRIn(); // Start the receiver
  pinMode(BUTTON_PIN, INPUT);
  pinMode(STATUS_PIN, OUTPUT);

  Serial.begin(9600);
  p("starting!\n", 0);
}

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state



// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results) 
{
  codeType = results->decode_type;
  int count = results->rawlen;
  if (codeType == UNKNOWN) 
  {
    Serial.println("Received unknown code, saving as raw");
    codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= codeLen; i++) 
    {
      if (i % 2) 
      {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else 
      {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else 
  {
    if (codeType == NEC) 
    {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) 
      {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (codeType == SONY) 
    {
      Serial.print("Received SONY: ");
    } 
    else if (codeType == RC5) 
    {
      Serial.print("Received RC5: ");
    } 
    else if (codeType == RC6) 
    {
      Serial.print("Received RC6: ");
    } 
    else 
    {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    codeValue = results->value;
    codeLen = results->bits;
  }
}

void sendCode(int repeat) 
{
  if (codeType == NEC) 
  {
    if (repeat) 
    {
      irsend.sendNEC(REPEAT, codeLen);
      Serial.println("Sent NEC repeat");
    } 
    else 
    {
      irsend.sendNEC(codeValue, codeLen);
      Serial.print("Sent NEC ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == SONY) 
  {
    irsend.sendSony(codeValue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  } 
  else if (codeType == RC5 || codeType == RC6) 
  {
    if (!repeat) 
    {
      // Flip the toggle bit for a new button press
      toggle = 1 - toggle;
    }
    // Put the toggle bit into the code to send
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codeType == RC5) 
    {
      Serial.print("Sent RC5 ");
      Serial.println(codeValue, HEX);
      irsend.sendRC5(codeValue, codeLen);
    } 
    else 
    {
      irsend.sendRC6(codeValue, codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == UNKNOWN /* i.e. raw */) 
  {
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.println("Sent raw");
  }
}

//Note: disables interrupts!    
void IrSend(decode_results code, int repeat=0)
{
  if (codeType == NEC) 
  {
    if (repeat) 
    {
      irsend.sendNEC(REPEAT, codeLen);
      Serial.println("Sent NEC repeat");
    } 
    else 
    {
      irsend.sendNEC(codeValue, codeLen);
      Serial.print("Sent NEC ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == SONY) 
  {
    irsend.sendSony(codeValue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  } 
  else if (codeType == RC5 || codeType == RC6) 
  {
    if (!repeat) 
    {
      // Flip the toggle bit for a new button press
      toggle = 1 - toggle;
    }
    // Put the toggle bit into the code to send
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codeType == RC5) 
    {
      Serial.print("Sent RC5 ");
      Serial.println(codeValue, HEX);
      irsend.sendRC5(codeValue, codeLen);
    } 
    else 
    {
      irsend.sendRC6(codeValue, codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == UNKNOWN /* i.e. raw */) 
  {
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.println("Sent raw");
  }
}

void PrintResults(decode_results *res)
{
  PRINT_INT(res->decode_type); //codeType
  PRINT_HEX(res->value); //codeValue
  PRINT_INT(res->bits); //codeLen
  PRINT_INT(res->rawlen);//count
}

//Sets variables appropriately such that the next call to SendCode will turn the TV on or off
bool StoreTvOnOffCode()
{
  //codeType = NEC;
  //codeValue = 0x10C8E11E;
  //codeLen = 32;
  decode_results res;
  res.decode_type = NEC;
  res.value = 0x10C8E11E;
  res.bits = 32;
  res.rawlen = 68;
  storeCode(&res);
  return true;
}

bool ToggleTvPower()
{
  StoreTvOnOffCode();

  int BackToBackDelayMs = 500;

  sendCode(0);
  delay(BackToBackDelayMs);
  sendCode(0);

  irrecv.enableIRIn();// I think we have to do this because interupts are probably disabled when we call sendCode()
  return true;
}

//Returns true if the Xbox One tried to send an Off Message
bool IsXboxOnOffCode(decode_results *results) 
{
  if (results->decode_type != SONY || 
    results->value != 0xA90 ||
    results->bits != 12) 
  {
    p("Not an XboxOnOff code.\n");
    return false;
  }

  p("Xbox OnOff code detected!\n");
  return true;
}

void loop() 
{
  char incomingByte;
  if (Serial.available() > 0) 
  {
    incomingByte = Serial.read();
  }

  if (incomingByte == 't') 
  {
    Serial.println("Pressed, sending");
    digitalWrite(STATUS_PIN, HIGH);


    ToggleTvPower();
    //sendCode(0);
    digitalWrite(STATUS_PIN, LOW);
    delay(50); // Wait a bit between retransmissions


    Serial.println("Done!");
    incomingByte = NULL;//Clear the incomingByte 
    irrecv.enableIRIn(); // Re-enable receiver
  } 

  else if (irrecv.decode(&results)) 
  {
    p("receiving...\n", 0);
    digitalWrite(STATUS_PIN, HIGH);
    storeCode(&results);
    PrintResults(&results);
    irrecv.resume(); // resume receiver

    digitalWrite(STATUS_PIN, LOW);

    if(IsXboxOnOffCode(&results))
    {
      //p("Sleeping...\n");
      //delay(5l0);
      p("Now toggling TV power...\n");
      ToggleTvPower();
    }

  }

  /* // If button pressed, send the code.
   int buttonState = digitalRead(BUTTON_PIN);
   
   if (lastButtonState == HIGH && buttonState == LOW) 
   {
   Serial.println("Released");
   irrecv.enableIRIn(); // Re-enable receiver
   }
   
   if (buttonState) 
   {
   Serial.println("Pressed, sending");
   digitalWrite(STATUS_PIN, HIGH);
   sendCode(lastButtonState == buttonState);
   digitalWrite(STATUS_PIN, LOW);
   delay(50); // Wait a bit between retransmissions
   } 
   else if (irrecv.decode(&results)) 
   {
   digitalWrite(STATUS_PIN, HIGH);
   storeCode(&results);
   irrecv.resume(); // resume receiver
   digitalWrite(STATUS_PIN, LOW);
   }
   lastButtonState = buttonState;*/
}













