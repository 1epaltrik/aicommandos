/********************************************************************************
 *  Έλεγχος σηματοδοτών κυκλοφορίας σε διασταύρωση με κάμερα τεχνητής νοημοσύνης,
 *  για αποφυγή κυκλοφοριακής συμφόρησης
 *  
 *  1ο ΕΠΑΛ Τρικάλων - ΕΛΛΑΚ 2024
 *  
 *  Υλικό:
 *  Arduino Uno R4 WiFi (με ενσωματωμένο led matrix)
 *  Camera HuskyLens AI Machine Vision Sensor
 *  Servo Motor
 *********************************************************************************/

// To use ArduinoGraphics APIs, please include BEFORE Arduino_LED_Matrix
// https://www.arduino.cc/reference/en/libraries/arduinographics/
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

#include "HUSKYLENS.h"

HUSKYLENS huskylens;
//HUSKYLENS green line >> SDA; blue line >> SCL
int ID1 = 1; //first learned results.

#include <Servo.h>

Servo myservo;  // servo object

int x;  // οριζόντια μετατόπιση για στοίχιση δεξιά των μονοψήφιων αριθμών στο matrix
int lookAtRoad = 1; // μεταβλητή που ορίζει σε ποιον δρόμο (1 ή 2) κοιτάζει η κάμερα

// τα LEDs των 2 σηματοδοτών L1 και L2
int L1Green = 7;
int L1Orange = 8;
int L1Red = 9;
int L2Green = 10;
int L2Orange = 11;
int L2Red = 12;

unsigned long timeNow = 0;      // ο τρέχων χρόνος
unsigned long timeStart = 0;    // η στιγμή που ανάβει το κόκκινο
unsigned long timeStartPerSecond = 0;   // έναρξη της χρονομέτρησης για κάθε 1 sec
int timeToFinishRed;      // υπολειπόμενος χρόνος (sec) για να σβήσει το κόκκινο

int timeOrange = 1000;    // οι αρχικοί χρόνοι που διαρκούν τα φανάρια
int timeL1Red = 6000;
int timeL2Red = 6000;

void setup()
{
  // αρχικοποίηση του led matrix
  matrix.begin();

  // Κάνε scroll τον τίτλο στο led matrix!
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(50);
  const char text[] = "    1o EPAL TRIKALON    ";
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
  
  Serial.begin(115200);
  
  Wire.begin();
  while (!huskylens.begin(Wire))
  {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }
  // θα χρησιμοποιήσουμε τον ενσωματωμένο αλγόριθμο αναγνώρισης αντικειμένων του HuskyLens
  huskylens.writeAlgorithm(ALGORITHM_OBJECT_RECOGNITION);
  
  myservo.attach(3);  // attaches the servo on pin 3 to the servo object
  
  pinMode( L1Green, OUTPUT);
  pinMode( L1Orange, OUTPUT);
  pinMode( L1Red, OUTPUT);
  pinMode( L2Green, OUTPUT);
  pinMode( L2Orange, OUTPUT);
  pinMode( L2Red, OUTPUT);

  // αναβόσβημα όλων των LED για έλεγχο
  digitalWrite(L1Green, HIGH);
  digitalWrite(L1Orange, HIGH);
  digitalWrite(L1Red, HIGH);
  digitalWrite(L2Green, HIGH);
  digitalWrite(L2Orange, HIGH);
  digitalWrite(L2Red, HIGH);
  delay(1000);
  digitalWrite(L1Green, LOW);
  digitalWrite(L1Orange, LOW);
  digitalWrite(L1Red, LOW);
  digitalWrite(L2Green, LOW);
  digitalWrite(L2Orange, LOW);
  digitalWrite(L2Red, LOW);
  delay(1000);
}

void loop()
{
  clearMatrix();  // πριν το πορτοκαλί καθαρίζουμε τo led matrix
  digitalWrite(L1Orange, HIGH);
  digitalWrite(L2Red, HIGH);
  delay(timeOrange);
  digitalWrite(L1Orange, LOW);
  digitalWrite(L2Red, LOW);
  digitalWrite(L1Red, HIGH);
  digitalWrite(L2Green, HIGH);
  lookRoad1();      // στροφή της κάμερας προς το δρόμο 1
  waitAndCheck(timeL1Red);   // αναμονή μέχρι να τελειώσει ο χρόνος ή να εμφανιστεί ουρά αυτοκινήτων
  clearMatrix();    // πριν το πορτοκαλί καθαρίζουμε τo led matrix
  digitalWrite(L1Red, LOW);
  digitalWrite(L2Green, LOW);
  digitalWrite(L1Red, HIGH);
  digitalWrite(L2Orange, HIGH);
  delay(timeOrange);
  digitalWrite(L1Red, LOW);
  digitalWrite(L2Orange, LOW);
  digitalWrite(L1Green, HIGH);
  digitalWrite(L2Red, HIGH);
  lookRoad2();  // στροφή της κάμερας προς το δρόμο 2
  waitAndCheck(timeL2Red);  // αναμονή μέχρι να τελειώσει ο χρόνος ή να εμφανιστεί ουρά αυτοκινήτων
  digitalWrite(L1Green, LOW);
  digitalWrite(L2Red, LOW);
}

// Το υποπρόγραμμα waitAndCheck δέχεται ως παράμετρο το χρόνο που διαρκεί το κόκκινο.
// Αν κατά τη διάρκεια αυτού του χρόνου η κάμερα εντοπίσει ουρά από 2 αυτοκίνητα, τότε ο χρόνος μηδενίζεται.
// Επίσης εμφανίζει τον υπολειπόμενο χρόνο στο matrix κάθε δευτερόλεπτο.
void waitAndCheck(int t)
{
  timeToFinishRed = t/1000;   // μετατροπή από millisecond σε second
  timeNow = timeStart = timeStartPerSecond = millis();    // αρχικοποίηση χρονομετρητών
  while( timeNow-timeStart < t )
  {
    if( timeNow-timeStartPerSecond>=1000 )    // εκτέλεση κάθε second
    {
      timeStartPerSecond = millis();
      timeToFinishRed--;
      // οριζόντια μετατόπιση x: για να στοιχίζονται οι μονοψήφιοι αριθμοί δεξιά στο led matrix
      if( timeToFinishRed<10 )
        x=7;
      else
        x=2;
      matrix.clear();
      matrix.beginText(x, 1, 0xFFFFFF);
      matrix.println(timeToFinishRed);    // εμφάνιση του υπολειπόμενου χρόνου στο led matrix
      matrix.endText();
    }
    
    if( huskylens.requestBlocks(ID1) )    // έλεγχος για αντικείμενα με ID1 (αυτοκίνητα)
    {
      if( huskylens.countBlocks(ID1) >= 2 )
      {
        Serial.println("Εντοπίστηκαν 2 αυτοκίνητα!!!");
        // Αλλαγή των χρόνων που διαρκούν τα φανάρια
        if( lookAtRoad == 1 )
        {
          timeL2Red = 25000;
          timeL1Red = 5000;
        }
        else if( lookAtRoad == 2 )
        {
          timeL2Red = 5000;
          timeL1Red = 25000;          
        }
        break;  // τέλος του while, οπότε θα αλλάξουν τα φανάρια
      }
    }
    timeNow = millis();
  }
}

// Περιστροφή της κάμερας προς το δρόμο 2
void lookRoad2(void)
{
  for (int pos = 0; pos <= 100; pos++)
  {
    // goes from 0 degrees to 100 degrees in steps of 1 degree
    myservo.write(pos);
    delay(10); 
  }
  lookAtRoad = 2;
}

// Περιστροφή της κάμερας προς το δρόμο 1
void lookRoad1(void)
{
  for (int pos = 100; pos >= 0; pos--)
  {
    // goes from 100 degrees to 0 degrees in steps of 1 degree
    myservo.write(pos);
    delay(10);
  }
  lookAtRoad = 1;
}

// Καθαρισμός του matrix
void clearMatrix()
{
  matrix.clear();
  matrix.beginDraw();
  matrix.endDraw();
}
