// reads a newline terminated string from serial and shows it on the screen together with the HEX value of each byte

// 57600 8 N 1 XON\XOFF
// no line ending 

#include <MicroView.h>

String inputString = "";         // a string to hold incoming data

void setup() {
    uView.begin();
    uView.clear(PAGE);
    Serial.begin(57600);
    
   // reserve 200 bytes for the inputString:
  inputString.reserve(200);

}

void loop()
{
  if(Serial.available()) {
    inputString = Serial.readStringUntil('\n');
    uView.clear(PAGE);
    uView.setCursor(0, 0);
    uView.println(inputString);
    uView.setCursor(0, 15);
    Serial.println(inputString);
    for (int i = 0; i < inputString.length(); i++) {
      uView.print(inputString.charAt(i), HEX);
      uView.print(' ');
      Serial.print(inputString.charAt(i), HEX);
      Serial.print(' ');
      }
    uView.display();
    inputString = "";
    Serial.println("");
    }
}

