// initialization for the lcd
#include <LiquidCrystal.h>
const int RS = 8;
const int enable = 9;
const int d4 = 6;
const int d5 = 5;
const int d6 = 4;
const int d7 = 3;
LiquidCrystal lcd(RS, enable, d4, d5, d6, d7);
int lcdState = 0; 
// lcdState == 0 => on the lcd will be displayed the current time and the remaining time until the next fall
// lcdState == 1 => the button was pressed for settings and the password must be entered
// lcdState == 2 => the lcd will display "Incorrect password"
// lcdState == 3 => the lcd will display the settings menu
// lcdState == 4 => the lcd will display the option "Set time"
// lcdState == 5 => the lcd will display the option "Change time of start"
// lcdState == 6 => the lcd will display the option "Change interval"

// initialization for the button
const int pushButton = 2;
int buttonState;

// initialization for the joystick
const int pinX = A0;
const int pinY = A1;
int xValue = 0;
int yValue = 0;
bool joyMoved = false;
int minThreshold = 200;
int maxThreshold = 700;

// initialization for the micro servo
#include <Servo.h>
int servoPin = 10;
Servo servo;  
int angle = 0;

// function for the micro servo
void fall() {
  for(angle = 0; angle < 90; angle++) {                                
    servo.write(angle);            
    delay(15);                 
  }
}

// variables and functions for the lcd
void lcdWelcome() {
  lcd.setCursor(3, 0);
  lcd.print("Pet Feeder");
  unsigned long currentTime = millis();
  int i = 0;
  while (i != 16) {
    if (millis() -  currentTime == 100) {
      currentTime = millis();
      lcd.setCursor(i, 1); lcd.print("~");
      i++;
    }
  }
  lcd.clear();
}

int hour, minutes, seconds; // variables for the current time
bool notChanging = true; // the current time (the variables hour, minutes and seconds) are not
                         // in the function setAndPrintTime where the user can change their values
bool timeChanged = false; // if the function setAndPrintTime() changed the values of the current
                          // time then timeChanged = true and the setRemainingTime() can't use  
                          // millis() to caculate the time
int rHour, rMinutes, rSeconds; // remaining hours/minutes/seconds until the fall
int fallHour = 0, fallMinutes = 0, fallSeconds = 30; // the default time of the interval
int startHour = 0, startMinutes = 0, startSeconds = 0; // the user can set a certain time(starting time)
                                                       // from where the remaining time can start decreasing
unsigned long lastFallTime; // retain the time of the last fall in seconds

void setRemainingTime() {
  // retain the time of the last fall 
  if (!timeChanged) 
       // lastFallTime = the number of seconds since the last fall from the start of the project
       lastFallTime = millis()/1000;
  else 
       // lastFallTime = the number of seconds of the current time
       lastFallTime  = (hour * 60 + minutes) * 60 + seconds;
  
  // set the remaining time
  rHour = fallHour;
  rMinutes = fallMinutes;
  rSeconds = fallSeconds;
}
void lcdDisplayInit() {
  // Display the current time on the first line of the lcd
  lcd.setCursor(0, 0);
  lcd.print("Hour:"); 
  
  lcd.setCursor(5, 0);
  if (hour < 10) lcd.print("0");
  lcd.print(hour); lcd.setCursor(7, 0); lcd.print(":"); 
  
  lcd.setCursor(8, 0);
  if (minutes < 10) lcd.print("0"); 
  lcd.print(minutes); lcd.setCursor(10, 0); lcd.print(":"); 

  lcd.setCursor(11, 0);
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds); 

  // Display the remaining time on the second line of the lcd
  lcd.setCursor(1, 1);
  lcd.print("RT:");  

  lcd.setCursor(5, 1);
  if (rHour < 10) lcd.print("0");
  lcd.print(rHour); 
  lcd.setCursor(7, 1); lcd.print(":"); 
  
  lcd.setCursor(8, 1);
  if (rMinutes < 10) lcd.print("0");
  lcd.print(rMinutes); 
  lcd.setCursor(10, 1); lcd.print(":"); 
  
  lcd.setCursor(11, 1);
  if (rSeconds < 10) lcd.print("0");
  lcd.print(rSeconds); 
}


int password[4];
int correctPassword[4] = {0, 0, 0, 0};
int posPassword = 0; // the position of the number in the password who is modified

// verify if the password in the password[] array is the same as the password in the correctPassword[] array
bool passwordIsCorrect() { 
  if (password[0] == correctPassword[0] && password[1] == correctPassword[1] &&
      password[2] == correctPassword[2] && password[3] == correctPassword[3])
      return true;
  return false;
}
void lcdEnterPassword() {
  lcd.setCursor(0, 0);
  lcd.print("Password:        "); 

  // print the values of the password array with spaces between them
  lcd.setCursor(0, 1); lcd.print("  "); 
  lcd.setCursor(2, 1); lcd.print("  "); 
  lcd.setCursor(4, 1); lcd.print("  "); 
  lcd.setCursor(6, 1); lcd.print("  "); 
  lcd.setCursor(8, 1); lcd.print("  "); 
  
  lcd.setCursor(1, 1); lcd.print(password[0]); 
  lcd.setCursor(3, 1); lcd.print(password[1]); 
  lcd.setCursor(5, 1); lcd.print(password[2]); 
  lcd.setCursor(7, 1); lcd.print(password[3]); 
  lcd.setCursor(9, 1); lcd.print("  "); 
  lcd.setCursor(11, 1); lcd.print(">>   "); // the option to submit the password
   
  // navigate through the password array with the x-axis of the joystick
  xValue = analogRead(pinX);
  if (xValue < minThreshold && joyMoved == false) {
    if (posPassword > 0) posPassword--;
    else posPassword = 4;
    joyMoved = true;
  }
  if (xValue > maxThreshold && joyMoved == false) {
    if (posPassword < 4) posPassword++;
    else posPassword = 0;
    joyMoved = true;
  }
  if (xValue >= minThreshold && xValue <= maxThreshold) {
    joyMoved = false;
  }

  // the joystick put the posPassword in the last position(the option to submit the password)
  if (posPassword == 4) {
    lcd.setCursor(10, 1);
    lcd.print(">");
    delay(500);
    // verify if the password is correct
    if (passwordIsCorrect()) {
      lcdState = 3; // display the settings menu
      delay(50);
    }
    else {
      // if the password is incorrect, display "Incorrect password" for 2 seconds
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Incorrect");
      lcd.setCursor(4, 1);
      lcd.print("password");
      delay(2000);

      // then go back and display the time in the lcdDisplayInit()
      lcd.clear();
      lcdState = 0;
      posPassword = 0;
      return;
    }
  }
  else {
    // the joystick put the posPassword in the one of the digits of the password
    // (the positions 0, 1, 2 or 3)
    if (posPassword == 3) { lcd.setCursor(6, 1); lcd.print(">"); }
    else if (posPassword == 2) {lcd.setCursor(4, 1); lcd.print(">");}
         else if (posPassword == 1) {lcd.setCursor(2, 1); lcd.print(">"); }
              else {lcd.setCursor(0, 1); lcd.print(">"); }
  }
  
  // change the value of the password[posPassword]
  yValue = analogRead(pinY);
  if (yValue < minThreshold && joyMoved == false && posPassword != 4) {
    if (password[posPassword] > 0) password[posPassword]--;
    else password[posPassword] = 9;
    joyMoved = true;
  }
  if (yValue > maxThreshold && joyMoved == false && posPassword != 4) {
    if (password[posPassword] < 9) password[posPassword]++;
    else password[posPassword] = 0;
    joyMoved = true;
  }
  if (yValue >= minThreshold && yValue <= maxThreshold) {
    joyMoved = false;
  }
}

int posSettings = 0; // for the navigation through the settings menu 
void lcdDisplaySettings() {
  // if an option has been select =>  go to this option 
  if (lcdState == 4) {
      setAndPrintTime(&hour, &minutes, &seconds);
      return;
  }
  else if (lcdState == 5) {
        setAndPrintTime(&startHour, &startMinutes, &startSeconds);
        return;
       }
       else if(lcdState == 6){
        setAndPrintTime(&fallHour, &fallMinutes, &fallSeconds);
        return;
       }
  
  // print the options
  lcd.setCursor(0, 0); lcd.print(" Set the time");
  lcd.setCursor(0, 1); lcd.print(" Ch.hour Ch.Int");
  
  // navigate through the settings menu with the x-axis of the joystick
  xValue = analogRead(pinX);
  if (xValue < minThreshold && joyMoved == false) {
    if (posSettings > 0) posSettings--;
    else posSettings = 3;
    joyMoved = true;
  }
  if (xValue > maxThreshold && joyMoved == false) {
    if (posSettings < 3) posSettings++;
    else posSettings = 0;
    joyMoved = true;
  }
  if (xValue >= minThreshold && xValue <= maxThreshold) {
    joyMoved = false;
  }
  if (posSettings == 0) {
    lcd.setCursor(0, 0);
    lcd.print(">");
  }
  else if (posSettings == 1) {
    lcd.setCursor(0, 1);
    lcd.print(">");
  }
  else {
    lcd.setCursor(8, 1);
    lcd.print(">");
  }

  // check if an option has been selected
   buttonState = digitalRead(pushButton);
  if (buttonState) {
    lcd.clear();
    lcd.setCursor(0, 0);
    // display the option lcd according to the position selected from the menu
    if (posSettings == 0) {
      notChanging = false; // stop the passing time until the time is set
      timeChanged = true;  // the current time will be changed and lastTimeFall  
                           // will not be calculated using millis()
      lcd.print("Set the time");
      lcdState = 4;
      setAndPrintTime(&hour, &minutes, &seconds);
    }
    else if (posSettings == 1) {
      lcd.print("Starting hour");
      notChanging = false; // stop the passing time until the time is set
      lcdState = 5;
      setAndPrintTime(&startHour, &startMinutes, &startSeconds);
    }
    else {
      lcd.print("Set the interval");
      lcdState = 6;
      setAndPrintTime(&fallHour, &fallMinutes, &fallSeconds);
    }
  }
  delay(50);
}

int change[3], posChange;
// change the values for the variables h(= hour = change[0]), 
// m(= minutes = change[1]), s(= seconds = change[2]) 
void setAndPrintTime(int *h, int *m, int *s) {
  // print the time
  lcd.setCursor(5, 1);
  if (*h < 10) lcd.print("0");
  lcd.print(*h); lcd.setCursor(7, 1); lcd.print(":"); 
  
  lcd.setCursor(8, 1);
  if (*m < 10) lcd.print("0"); 
  lcd.print(*m); lcd.setCursor(10, 1); lcd.print(":"); 

  lcd.setCursor(11, 1);
  if (*s < 10) lcd.print("0");
  lcd.print(*s);

  // set the time
  // navigate through the variables(h, m, s) with the x-axis
  xValue = analogRead(pinX);
  if (xValue < minThreshold && joyMoved == false) {
    if (posChange > 0) posChange--;
    else posChange = 2;
    joyMoved = true;
  }
  if (xValue > maxThreshold && joyMoved == false) {
    if (posChange < 2) posChange++;
    else posChange = 0;
    joyMoved = true;
  }
  if (xValue >= minThreshold && xValue <= maxThreshold) {
    joyMoved = false;
  }

  // change the value of a variable(h/m/s) with the y-axis
  yValue = analogRead(pinY);
  change[0] = (*h);
  change[1] = (*m);
  change[2] = (*s);
  if (yValue < minThreshold && joyMoved == false) {
    if (change[posChange] > 0) change[posChange]--;
    else if (posChange != 0) change[posChange] = 59;
    else change[posChange] = 23;
    joyMoved = true;
  }
  if (yValue > maxThreshold && joyMoved == false) {
    if (posChange == 0) {
      if (change[0] < 23) change[0]++;
      else change[0] = 0;
    }
    else {
      if (change[posChange] < 59) change[posChange]++;
      else change[posChange] = 0;
    }
    joyMoved = true;
  }
  if (yValue >= minThreshold && yValue <= maxThreshold) {
    joyMoved = false;
  }

  (*h) = change[0];
  (*m) = change[1];
  (*s) = change[2];
  delay(100);

  // verify if the user has finished changing the time
  buttonState = digitalRead(pushButton);
  if (buttonState) {
    if (lcdState == 4 || lcdState == 5) lastFallTime += ((*h) * 60 + (*m)) * 60 + (*s);
    lcdState = 0;
    notChanging = true; // start the passingTime() function
    lcd.clear();
  }
  delay(100);
}

unsigned long currentTime;
int timePassingInterval = 1000;
unsigned long lastTimePassing = 0;
int lastHour, lastMinutes, lastSeconds; // the time of the last fall 
void passingTime() {
  // calculation of the current time in hours, minutes, seconds
  if (!timeChanged)  {
    currentTime = (millis())/1000; // the time in seconds
  }
  else currentTime = (hour * 60 + minutes) * 60 + seconds;
  
  minutes = currentTime/60; // the total of minutes (including the minutes 
                            // which have not yet been transformed in hours)
  seconds = currentTime - minutes*60;
  hour = minutes/60;
  minutes = currentTime/60 - hour*60;
  if (hour == 24) hour = 0;
  
  if (timeChanged && (millis() - lastTimePassing > timePassingInterval)) {
    seconds++;
    lastTimePassing = millis();
  }

  // calculation of the last fall time in hours, minutes, seconds
  int startingHour = (startHour * 60 + startMinutes) * 60 + startSeconds;
  if (currentTime == startingHour)  {
    fall();
    setRemainingTime();
  }
  if (currentTime >= startingHour) {
    // calculation of the interval between the current time and the last fall
    unsigned long auxCurrentTime = currentTime - lastFallTime;
    int auxMinutes = auxCurrentTime/60; // the total of minutes (including the minutes 
                                        // which have not yet been transformed in hours)
    int auxSeconds = auxCurrentTime - auxMinutes*60;
    int auxHour = auxMinutes/60;
    auxMinutes = auxCurrentTime/60 - auxHour*60;
    if (auxHour == 24) auxHour = 0;  
  
    if (auxHour == fallHour && auxMinutes == fallMinutes && auxSeconds == fallSeconds) {
      fall();
      setRemainingTime();
    }
    else {
      // calculation of the remaining time that will be displayed on the lcd
      rHour = fallHour - auxHour;
      rMinutes = fallMinutes - auxMinutes;
      rSeconds = fallSeconds - auxSeconds;
    }
  }
}
void setup() {
  // set up for the lcd
  lcd.begin(16, 2);
  lcdWelcome();

  // set up for the button
  pinMode(pushButton, INPUT);

  // set up for the micro servo
  servo.attach(servoPin);
  
  // other set up
  Serial.begin(9600);
  
  // the current time (the variables hour, minutes and seconds) is initialized at 0 but 
  // the user can change his value
  hour = 0;
  minutes = 0;
  seconds = 0;

  setRemainingTime(); //sets the interval of the fall with the default values
}
void loop() { 
  // lcdState == 0 => the lcd displays the currentTime and the remaining time till the fall
  // (with the function lcdDisplayInit) and in this state we can verify if the button for the 
  // settings has been press
  if (lcdState == 0) { 
    buttonState = digitalRead(pushButton);
    if (buttonState) {
      lcd.clear();
      lcdState = 1; // the lcd has to display the settings 
      // to acces the settings, the user has to enter a password of 4 digits
      posPassword = 0; // the position in the password array has the default value equal to 0
      password[0] = password[1] = password[2] = password[3] = 0;
    }
  }
  
  
  if (!lcdState) lcdDisplayInit();
  else if(lcdState == 1) lcdEnterPassword();
    else lcdDisplaySettings();

  // if the current time (the variables hour, minutes and seconds) are not in the function setAndPrintTime(), 
  // where the user can change their values, the function passingTime() is called
  if (notChanging) {
    // the passingTime() function calculates the current time(hour, minutes and seconds) and 
    // the remaining time until the fall
    passingTime();
  }
}
