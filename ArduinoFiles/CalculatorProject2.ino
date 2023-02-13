/*
  SD card read/write

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>
#define INIT_DELAY 200
#define PER_CHAR_DELAY 100
char fname[10] = "test0.txt";
byte first = 0, total = 0, current=0;
bool called=false;
//SoftwareSerial mySerial(10,8);
File myFile;
void setup() {
  // Open serial communications and wait for port to open:
  //pinMode(2, INPUT);
  //pinMode(3, INPUT);
  Serial.begin(9600);
  SD.begin();
  for(int i=0;i<10;i++)
  {
    myFile=SD.open(fname);
    if(myFile)
    {
      myFile.close();
      SD.remove(fname);
    }
    fname[4]++;
  }
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  //attachInterrupt(0, out, FALLING);
  attachInterrupt(0, previous_file, FALLING);
  attachInterrupt(1, next_file, FALLING);
}

void loop() {
  if (Serial.available())
  {
    set_file();
    myFile = SD.open(fname, FILE_WRITE);
    char c = Serial.read();
    if (c != '@') myFile.write(c);
    myFile.close();
    if (c == '@')
    {
      if (total < 9)
      {
        increase_total();
      }
      else
      {
        increase_first();
        SD.remove(fname);
      }
      
    }
  }
}

void out()
{
  if (total <= 0)
  {
    delay(INIT_DELAY);
    delay(PER_CHAR_DELAY);
    Serial.write('#');
    delay(PER_CHAR_DELAY);
    Serial.write('\n');
    return;
  }
  file_pop();
  delay(INIT_DELAY);
  myFile = SD.open(fname);
  while (myFile.available())
  {
    delay(PER_CHAR_DELAY);
    Serial.write(myFile.read());
  }
  myFile.close();
  delay(PER_CHAR_DELAY);
  Serial.write('\n');
  SD.remove(fname);
}

void previous_file()
{
  decrease_pointer();
  access_file();
}

void next_file()
{
  increase_pointer();
  access_file();
}

void access_file()
{
  if (current < 0 || current >= total)
  {
    delay(INIT_DELAY);
    delay(PER_CHAR_DELAY);
    Serial.write('#');
    delay(PER_CHAR_DELAY);
    Serial.write('\n');
    return;
  }
  find_file();
  delay(INIT_DELAY);
  myFile = SD.open(fname);
  while (myFile.available())
  {
    delay(PER_CHAR_DELAY);
    Serial.write(myFile.read());
  }
  myFile.close();
  delay(PER_CHAR_DELAY);
  Serial.write('\n');
}

void file_pop()
{
  fname[4] = ('0' + (first + total-1) % 10);
  total--;
}

void increase_pointer()
{
  if(current+1<total) current++;
}

void decrease_pointer()
{
  if(current>0) current--;
}

void find_file()
{
  fname[4] = ('0' + (first + current) % 10);
}

void increase_first()
{
  fname[4] = '0' + first;
  first = (first + 1) % 10;
  current=total;
}

void increase_total()
{
  total++;
  current=total;
}

void set_file()
{
  fname[4] = '0' + (first + total) % 10;
}
