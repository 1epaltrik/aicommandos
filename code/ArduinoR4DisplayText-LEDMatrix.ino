// To use ArduinoGraphics APIs, please include BEFORE Arduino_LED_Matrix
// https://www.arduino.cc/reference/en/libraries/arduinographics/

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

int x;  // οριζόντια μετατόπιση για στοίχιση δεξιά των μονοψήφιων αριθμών

void setup()
{
  // αρχικοποίηση του matrix
  matrix.begin();

  // Κάνε scroll τον τίτλο!
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(50);
  const char text[] = "    1o EPAL TRIKALON    ";
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

void loop()
{
  for( int i=0; i<100; i++ )
  {
    // οριζόντια μετατόπιση x: για να στοιχίζονται οι μονοψήφιοι αριθμοί δεξιά
    if( i<10 )
      x=7;
    else
      x=2;

    matrix.clear();
    matrix.beginText(x, 1, 0xFFFFFF);
    matrix.println(i);
    matrix.endText();
    delay(200);
  }
}
