//Webbing Cutter V1

#include <LiquidCrystal.h> 
#include <Servo.h>
#include <Stepper.h>

// initialize the interface pins for the LCD
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

//Define Various Menu Variables used throughout
int timer = 0;
byte totalRows = 2;                // total rows of LCD
byte totalCols = 16;               // total columns of LCD
int returndata = 0;                // Used for return of button presses
unsigned long timeoutTime = 0;     // this is set and compared to millis to see when the user last did something.
const int menuTimeout = 20000;     // time to timeout in a menu when user doesn't do anything.
unsigned long lastButtonPressed;   // this is when the last button was pressed. It's used to debounce.
const int debounceTime = 150;      // this is the debounce and hold delay. Otherwise, you will FLY through the menu by touching the button. 

const int buttonUp = 4;            // Set pin for UP Button
const int buttonDown = 6;          // Set pin for DOWN Button
const int buttonSelect = 5;        // Set pin for SLELECT Button

int buttonStateUp = 0;             // Initalise ButtonStates
int buttonStateDown = 0;
int buttonState;

// constants for indicating whether cursor should be redrawn
#define MOVECURSOR 1 
// constants for indicating whether cursor should be redrawn
#define MOVELIST 2  


//Hardware
const int encoder_a = 2; // Green (Red - 5v, Black - GND)
const int encoder_b = 3; // White
long encoder = 0;

const int StepperStepPin = A3;
const int StepperDirectionPin = A4;
const int stepsPerRevolution = 200;
Stepper stepper(stepsPerRevolution, StepperStepPin, StepperDirectionPin);
const int StepperMaxRPM = 100;

Servo servoMark; Servo servoCut;

const int hotKnife = A2;

//Job Variables
int selectedQty = 0;
int currentQty = 0;
float feedLength = 0;
int cut = 0;
int markQty = 0;
int markLength[10];

//Program Flow Variables
int setQtyFlag = 0;
int lastLoop = 0;
//float currentDist = 0;

//Function Prototypes
void encoderPinChangeA(void);
void encoderPinChangeB(void);
int read_buttons();
void RunJob(void);
void EnterQty(void);


void setup(){

    // set up the LCD's number of columns and rows: 
    lcd.begin(totalCols, totalRows);      
    
    // initialize the serial communications port:
    Serial.begin(9600);
        
    pinMode(buttonUp, INPUT_PULLUP);
    pinMode(buttonDown, INPUT_PULLUP);
    pinMode(buttonSelect, INPUT_PULLUP);

    pinMode(encoder_a, INPUT_PULLUP);
    pinMode(encoder_b, INPUT_PULLUP);

    attachInterrupt(0, encoderPinChangeA, CHANGE);
    attachInterrupt(1, encoderPinChangeB, CHANGE);

    stepper.setSpeed(StepperMaxRPM);

    servoMark.attach(14, 700, 2300); //analog pin 0 pin, min, max

    servoCut.attach(15, 700, 2300); //analog pin 1

    pinMode(hotKnife, OUTPUT);

      // Boot Up Message
    Serial.println("Webbing Cutter");
    lcd.print("Webbing Cutter");
    lcd.setCursor(0, 1);
    lcd.print("Starting Up");
    Serial.print("Starting Up");
    delay(300);
    for( int i = 0; i < 6; i++) {
        Serial.print(".");
        lcd.print(".");
        delay(100);
    } 
  
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
    
    lcd.clear();
  
}  


void loop() {
  
  byte topItemDisplayed = 0;  // stores menu item displayed at top of LCD screen
  byte cursorPosition = 0;  // where cursor is on screen, from 0 --> totalRows. 

  // redraw = 0  - don't redraw
  // redraw = 1 - redraw cursor
  // redraw = 2 - redraw list
  byte redraw = MOVELIST;  // triggers whether menu is redrawn after cursor move.
  byte i=0; // temp variable for loops.
  byte totalMenuItems = 0;  //a while loop below will set this to the # of menu items.

// Create list of Menu Items
  char* menuItems[]={
    "Load Material", 
    "End Loops",
    "TSUL Webbing", 
    "UTSP Webbing",
    "Whoopie Slings",
    "Extensions",
    "Drawstring T",
    "Drawstring T+",
    "Drawstring S",
    "Drawstring S+",
    "Drawstring D",
    "Drawstring L",
    "Drawstring H",
    "Drawstring P",
    "Unload/Reverse",
    "Cut",
    "",
  };

// count how many items are in list.

  while (menuItems[totalMenuItems] != ""){
    totalMenuItems++;  
  }
  
  //subtract 1 so we know total items in array.
  totalMenuItems--;  
  

  lcd.clear();  // clear the screen so we can paint the menu.

  boolean stillSelecting = true;  // set because user is still selecting.

  timeoutTime = millis() + menuTimeout; // set initial timeout limit. 

  do   // Run a loop while waiting for user to select menu option.
  {

// Call any other setup actions required and/or process anything else required whilst in setup mode as opposed to things setup regardless of setup mode


    
// Call read buttons routine which analyzes buttons and gets a response. Default response is 0.  
switch(read_buttons())
    {  

   
      // Case responses depending on what is returned from read buttons routine
      
      case 1:  // 'UP' BUTTON PUSHED

      timeoutTime = millis()+menuTimeout;  // reset timeout timer
      //  if cursor is at top and menu is NOT at top
      //  move menu up one.
      if(cursorPosition == 0 && topItemDisplayed > 0)  //  Cursor is at top of LCD, and there are higher menu items still to be displayed.
      {
        topItemDisplayed--;  // move top menu item displayed up one. 
        redraw = MOVELIST;  // redraw the entire menu
      }

      // if cursor not at top, move it up one.
      if(cursorPosition>0)
      {
        cursorPosition--;  // move cursor up one.
        redraw = MOVECURSOR;  // redraw just cursor.
      }
      break;

   
   
   
      case 2:    // 'DOWN' BUTTON PUSHED

      timeoutTime = millis()+menuTimeout;  // reset timeout timer
      // this sees if there are menu items below the bottom of the LCD screen & sees if cursor is at bottom of LCD 
      if((topItemDisplayed + (totalRows-1)) < totalMenuItems && cursorPosition == (totalRows-1))
      {
        topItemDisplayed++;  // move menu down one
        redraw = MOVELIST;  // redraw entire menu
      }
      if(cursorPosition<(totalRows-1))  // cursor is not at bottom of LCD, so move it down one.
      {
        cursorPosition++;  // move cursor down one
        redraw = MOVECURSOR;  // redraw just cursor.
      }
      break;

      
      
      
      
      
      case 4:  // SELECT BUTTON PUSHED

      timeoutTime = millis()+menuTimeout;  // reset timeout timer
      switch(topItemDisplayed + cursorPosition) // adding these values together = where on menuItems cursor is.
      {
      case 0:  // menu item 1 selected          
        Serial.println("Load");
        lcd.clear();
        Load();
        stillSelecting = false;
       break;

      case 1:  // menu item 2 selected
        Serial.println("End Loops");
        lcd.clear();
        setQtyFlag = 0;
        EndLoops();
        stillSelecting = false;
        break;

      case 2:  // menu item 3 selected
        Serial.println("TSUL Webbing");
        lcd.clear();
        setQtyFlag = 0;
        TSULWebbing();
        stillSelecting = false;
        break;

      case 3:  // menu item 4 selected
        Serial.println("UTSPWebbing");
        lcd.clear();
        setQtyFlag = 0;
        UTSPWebbing();
        stillSelecting = false;
        break;

      case 4:  // menu item 5 selected
        Serial.println("Whoopie Slings");
        lcd.clear();
        setQtyFlag = 0;
        WhoopieSlings();
        stillSelecting = false;
        break;

      case 5:  // menu item 5 selected
        Serial.println("Extensions");
        lcd.clear();
        setQtyFlag = 0;
        Extensions();
        stillSelecting = false;
        break;

      case 6:  // menu item 6 selected
        Serial.println("Drawstring T");
        lcd.clear();
        setQtyFlag = 0;
        DrawstringTreeStraps();
        stillSelecting = false;
        break;

      case 7:  // menu item 6 selected
        Serial.println("Drawstring T+");
        lcd.clear();
        setQtyFlag = 0;
        DrawstringTreeStrapsPlus();
        stillSelecting = false;
        break;

      case 8:  // menu item 6 selected
        Serial.println("Drawstring S");
        lcd.clear();
        setQtyFlag = 0;
        DrawstringSingle();
        stillSelecting = false;
        break;

      case 9:  // menu item 6 selected
        Serial.println("Drawstring S+");
        lcd.clear();
        setQtyFlag = 0;
        DrawstringSinglePlus();
        stillSelecting = false;
        break;

      case 10:  // menu item 6 selected
        Serial.println("Drawstring D");
        lcd.clear();
        setQtyFlag = 0;
        DrawstringDouble();
        stillSelecting = false;
        break;

      case 11:  // menu item 6 selected
        Serial.println("Drawstring L");
        lcd.clear();
        setQtyFlag = 0;
        DrawstringLong();
        stillSelecting = false;
        break;

      case 12:  // menu item 6 selected
        Serial.println("Drawstring H");
        lcd.clear();
        setQtyFlag = 0;
        DrawstringHeron();
        stillSelecting = false;
        break;

      case 13:  // menu item 6 selected
        Serial.println("Drawstring P");
        lcd.clear();
        setQtyFlag = 0;
        DrawstringPelican();
        stillSelecting = false;
        break;

      case 14:  // menu item 6 selected
        Serial.println("Unload/Reverse");
        lcd.clear();
        Unload();
        stillSelecting = false;
        break;

      case 15:  // menu item 6 selected
        Serial.println("Cut");
        lcd.clear();
        Cut();
        stillSelecting = false;
        break;
       
      }
      break;


}

    switch(redraw){  //  checks if menu should be redrawn at all.
    case MOVECURSOR:  // Only the cursor needs to be moved.
      redraw = false;  // reset flag.
      if (cursorPosition > totalMenuItems) // keeps cursor from moving beyond menu items.
        cursorPosition = totalMenuItems;
      for(i = 0; i < (totalRows); i++){  // loop through all of the lines on the LCD
        lcd.setCursor(0,i);
        lcd.print(" ");                      // and erase the previously displayed cursor
        lcd.setCursor((totalCols-1), i);
        lcd.print(" ");
      }
      lcd.setCursor(0,cursorPosition);      // go to LCD line where new cursor should be & display it.
      lcd.print(">");
      lcd.setCursor((totalCols-1), cursorPosition);
      lcd.print("<");
      break;  // MOVECURSOR break.

    case MOVELIST:  // the entire menu needs to be redrawn
      redraw=MOVECURSOR;  // redraw cursor after clearing LCD and printing menu.
      lcd.clear(); // clear screen so it can be repainted.
      if(totalMenuItems>((totalRows-1))){  // if there are more menu items than LCD rows, then cycle through menu items.
        for (i = 0; i < (totalRows); i++){
          lcd.setCursor(1,i);
          lcd.print(menuItems[topItemDisplayed + i]);
        }
      }
      else{  // if menu has less items than LCD rows, display all available menu items.
        for (i = 0; i < totalMenuItems+1; i++){
          lcd.setCursor(1,i);
          lcd.print(menuItems[topItemDisplayed + i]);
        }
      }
   
      break;  // MOVELIST break
      
    }

    if (timeoutTime<millis()){  // user hasn't done anything in awhile
      stillSelecting = false;  // tell loop to bail out.
     
    }
  } 


  while (stillSelecting == true);  //
  
//End of Start Setup mode if

lcd.clear();

}

void Load() {
  
  lcd.print("Load Material");
  lcd.setCursor(0, 1);
  lcd.print("Feeding 6 inches");
  delay(500);

  feedLength = 6;
  cut = 0;
  
  RunJob();
  
  lcd.clear();
  
}

void EndLoops() {

  lcd.setCursor(0,0);
  lcd.print("End Loops ");

  lastLoop = 1;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 36.25;
  cut = 1;
  markQty = 7;
  markLength[0] = 3;
  markLength[1] = 5;
  markLength[2] = 7;
  markLength[3] = 16;
  markLength[4] = 20;
  markLength[5] = 25.25;
  markLength[6] = 27.15;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void TSULWebbing() {
  
  lcd.setCursor(0,0);
  lcd.print("TSUL ");

  lastLoop = 2;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 66;
  cut = 1;
  markQty = 2;
  markLength[0] = 6;
  markLength[1] = 60;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void UTSPWebbing() {
  
  lcd.setCursor(0,0);
  lcd.print("UTSP ");

  lastLoop = 3;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 128;
  cut = 1;
  markQty = 2;
  markLength[0] = 8;
  markLength[1] = 120;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void WhoopieSlings() {
  
  lcd.setCursor(0,0);
  lcd.print("Whoopie ");

  lastLoop = 4;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 82;
  cut = 1;
  markQty = 4;
  markLength[0] = 3;
  markLength[1] = 5;
  markLength[2] = 8;
  markLength[3] = 14;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void Extensions() {
  
  lcd.setCursor(0,0);
  lcd.print("Extensions ");

  lastLoop = 5;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 92;
  cut = 1;
  markQty = 7;
  markLength[0] = 3;
  markLength[1] = 5;
  markLength[2] = 9;
  markLength[3] = 17.25;
  markLength[0] = 25.75;
  markLength[1] = 87;
  markLength[2] = 89;
  

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void DrawstringTreeStraps() {
  
  lcd.setCursor(0,0);
  lcd.print("Drawstring T ");

  lastLoop = 6;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 12;
  cut = 1;
  markQty = 0;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void DrawstringTreeStrapsPlus() {
  
  lcd.setCursor(0,0);
  lcd.print("Drawstring T+ ");
  
  lastLoop = 7;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 13;
  cut = 1;
  markQty = 0;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void DrawstringSingle() {
  
  lcd.setCursor(0,0);
  lcd.print("Drawstring S ");
  
  lastLoop = 8;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 16;
  cut = 1;
  markQty = 0;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void DrawstringSinglePlus() {
  
  lcd.setCursor(0,0);
  lcd.print("Drawstring S+ ");

  lastLoop = 9;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 17;
  cut = 1;
  markQty = 0;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void DrawstringDouble() {
  
  lcd.setCursor(0,0);
  lcd.print("Drawstring D ");

  lastLoop = 10;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 18;
  cut = 1;
  markQty = 0;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void DrawstringLong() {
  
  lcd.setCursor(0,0);
  lcd.print("Drawstring L ");

  lastLoop = 11;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 15;
  cut = 1;
  markQty = 0;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void DrawstringHeron() {
  
  lcd.setCursor(0,0);
  lcd.print("Drawstring H ");

  lastLoop = 12;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 16;
  cut = 1;
  markQty = 0;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void DrawstringPelican() {
  
  lcd.setCursor(0,0);
  lcd.print("Drawstring P ");

  lastLoop = 13;
  
  while (setQtyFlag != 1){
    EnterQty();
    }

  //Mode Variables
  feedLength = 17;
  cut = 1;
  markQty = 0;

  while (currentQty < selectedQty) {
  
  lcd.setCursor(13, 0);
  lcd.print("#");
  lcd.print(currentQty);

  RunJob();

  }

  //feed 8in
  Cut();

  Serial.println("Done!");

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Done!");

  delay(1000);
    
  currentQty = 0;
  selectedQty = 0;
  
  return;
  
}

void Unload() {

  lcd.print("Unloading");
  lcd.setCursor(0, 1);
  lcd.print("Reverse 6 inches");
  delay(500);

  feedLength = -6;
  cut = 0;
  
  RunJob();
  
  lcd.clear();
  
}

void Cut() {
  
  Serial.println("Cutting");
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Cutting");

  digitalWrite(hotKnife, HIGH);
  servoCut.write(180);
  delay(500);
  digitalWrite(hotKnife, LOW);
  servoCut.write(0);
  delay(15);
  
}

void RunJob() {

  int i = 0;

  for (float currentDist = 0; currentDist < feedLength;){
      
      currentDist++;
      delay(300);
      Serial.println(currentDist);
      //stepper.step(1);

      if (currentDist == 8 && cut == 1) Cut();

      if (currentDist == markLength[i] && i <= markQty && markQty != 0) {
        Serial.println("Mark Location ");
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("Mark #");
        lcd.print(i + 1);
        
        
        servoMark.write(150);
        delay(300);
        servoMark.write(5);
        delay(300);
        
        
        i++;
      }
  }

  encoder = 0; //empty for next round
  currentQty++;
  
}

void EnterQty() {

  lcd.setCursor(0,1);
  lcd.print("Enter Qty: ");
    
  switch(read_buttons()) {  
      
      case 1:  // 'UP' BUTTON PUSHED
      
      selectedQty++;

      Serial.println(selectedQty);
      lcd.setCursor(11,1);
      lcd.print("     ");
      lcd.setCursor(11,1);
      lcd.print(selectedQty);
      break;
   
      case 2:    // 'DOWN' BUTTON PUSHED

      if (selectedQty > 0) {

        selectedQty--;

        Serial.println(selectedQty);
        lcd.setCursor(11,1);
        lcd.print("     ");
        lcd.setCursor(11,1);
        lcd.print(selectedQty);
        
      }
      break;

      case 4:    // 'SELECT' BUTTON PUSHED

      if (selectedQty > 0) {

        setQtyFlag = 1;
        
        switch (lastLoop){
          case 1:
          EndLoops();
          break;
          
          case 2:
          TSULWebbing();
          break;

          case 3:
          UTSPWebbing();
          break;

          case 4:
          WhoopieSlings();
          break;

          case 5:
          Extensions();
          break;

          case 7:
          DrawstringTreeStraps();
          break;

          case 8:
          DrawstringTreeStrapsPlus();
          break;

          case 9:
          DrawstringSingle();
          break;

          case 10:
          DrawstringSinglePlus();
          break;

          case 11:
          DrawstringDouble();
          break;

          case 12:
          DrawstringLong();
          break;

          case 13:
          DrawstringHeron();
          break;

          case 14:
          DrawstringPelican();
          break;
          
        }
      }
      break;
    }
  
}

int read_buttons(){  // you may need to swap "void" with "int" or "byte"
  
  int returndata = 0;
 
  if ((lastButtonPressed + debounceTime) < millis()){  // see if it's time to check the buttons again
    
    // read Up button
    buttonState = digitalRead(buttonUp);
   
    if (buttonState == LOW){
      returndata = returndata + 1;
      lastButtonPressed = millis();
      Serial.println("UP");
    }

    // read Down button
    buttonState = digitalRead(buttonDown);
    
    if (buttonState == LOW){
      returndata = returndata + 2;
      lastButtonPressed = millis();
      Serial.println("DOWN");
    }

    //read Select button
    buttonState = digitalRead(buttonSelect);
    
    if (buttonState == LOW){
      returndata = returndata + 4; 
      lastButtonPressed = millis();
      Serial.println("SELECT");
    }

  }
  
  return returndata; // this spits back to the function that calls it the variable returndata.
}

void encoderPinChangeA() {
encoder += digitalRead(encoder_a) == digitalRead(encoder_b) ? -1 : 1;
}

void encoderPinChangeB() {
encoder += digitalRead(encoder_a) != digitalRead(encoder_b) ? -1 : 1;
}
