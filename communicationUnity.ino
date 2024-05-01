#include <QuadratureEncoder.h>

//script designed to control exoskeleton motors through character codes sent from Unity application

// Motor Driver Pins
const int M2directionPin = 5;  // Motor Driver Direction Pin
const int M2pwmPin = 4;        // Motor Driver PWM Control Pin
const int M2encoderAPin = 18; // Encoder channel A
const int M2encoderBPin = 19; // Encoder channel B
const int M1directionPin = 3;  // Motor Driver Direction Pin
const int M1pwmPin = 2;        // Motor Driver PWM Control Pin
const int M1encoderAPin = 20; // Encoder channel A
const int M1encoderBPin = 21; // Encoder channel B

int TargetAngle = 0;

Encoders M1encoder(M1encoderAPin, M1encoderBPin);

Encoders M2encoder(M2encoderAPin, M2encoderBPin);


// Function declarations
void handleData();
void moveToEquilibrium();
void setEquillibrium();
void calibrate();
void moveUp1();
void moveUp5();
void moveDown1();
void moveDown5();
void moveRight1();
void moveRight5();
void moveLeft1();
void moveLeft5();


//deeclare variables 
char val = 0;
int num;
int speed;
int repCount;
bool LeftOrRight;
int MaxUp;
int MaxDown;
int MaxLeft;
int MaxRight;
int maintainIndex;
unsigned long elapsedTime;
unsigned long startTime;

void setup() {

  //initialise motor driver pins
  pinMode(M1directionPin, OUTPUT);
  pinMode(M1pwmPin, OUTPUT);
  pinMode(M2directionPin, OUTPUT);
  pinMode(M2pwmPin, OUTPUT);

  //begin serial communication
  Serial.begin(9600);

  //initialise encoders
  pinMode(M1encoderAPin, INPUT_PULLUP);
  pinMode(M1encoderBPin, INPUT_PULLUP);
  pinMode(M2encoderAPin, INPUT_PULLUP);
  pinMode(M2encoderBPin, INPUT_PULLUP);
  
  
  // set PWM to 0 
  analogWrite(M1pwmPin, 0); // Adjust this value to change speed, range 0-255
  analogWrite(M2pwmPin, 0); 

}



void loop() {
  //if 4 digit code is recieved, handle data
  if (Serial.available() > 4) {
    handleData();
  }
}


void handleData() {
  // Read the first character of the incoming data
   if (Serial.peek() == '\n') {
    Serial.read(); // Discard the newline character
  } 

  //first character shows what movement to do or calibration/equillibrium setting
  char command = Serial.read();

  // Read the speed (second character), this will have a multiplier add on to the PWM signals
  speed = Serial.read() - '0'; // Convert char to int
  
  //check if going to be continuous mode
  if(Serial.peek() == 'A'){
    Serial.read();
    Serial.read();
    repCount = 9999;
  }

  //read repetitions count
  else{
    repCount = (Serial.read() - '0') * 10 + (Serial.read() - '0');
  }

  
  // Command motors

  switch (command) {

    //Up and down, plnatrflexion/dorsiflexion
    case 'a':

      //repeat up and down for as many times as repCount
      for (int i = 0; i < repCount; i++) {

        //emergency stop if 'd' command is recieved 
        if (Serial.available() > 0 && Serial.read() == 'd') {
          analogWrite(M1pwmPin, 0);
          return;
        }

        //move robot up until target is reached
        digitalWrite(M1directionPin, LOW);
        while (M1encoder.getEncoderCount() / 5281.1 * 360 < MaxUp) {

          //emergency stop if 'd' command is recieved 
          if (Serial.available() > 0 && Serial.read() == 'd') {
          analogWrite(M1pwmPin, 0);
          return;
        }
          //move robot up, higher PWM then others as it needs more power going up
          analogWrite(M1pwmPin, 100 + 15 * speed);
        }

        //stop motor
        analogWrite(M1pwmPin, 0);

        //hold maximum stretch for 3 seconds and maintain angle whilst
        startTime = millis();
        elapsedTime = 0;
        while (elapsedTime < 3000){
          Serial.println(elapsedTime);
          unsigned long currentTime = millis(); // Get the current time
          elapsedTime = currentTime - startTime;
          MaintainAngle(MaxUp, 1);
        }

        //switch direction
        digitalWrite(M1directionPin, HIGH);

        //move robot down until target is reached
        while (M1encoder.getEncoderCount() / 5281.1 * 360 > MaxDown) {

          //emergency stop if 'd' command is recieved 
          if (Serial.available() > 0 && Serial.read() == 'd') {
          analogWrite(M1pwmPin, 0);
          return;
        }

          //move down
          analogWrite(M1pwmPin, 50 + 15 * speed);
        }

        //stop the motor
        analogWrite(M1pwmPin, 0);

        //hold maximum stretch for 3 seconds and maintain angle whilst
        startTime = millis();
        elapsedTime = 0;
        while (elapsedTime < 3000){
          Serial.println(elapsedTime);
          unsigned long currentTime = millis(); // Get the current time
          elapsedTime = currentTime - startTime;
          MaintainAngle(MaxDown, 0);
        }

        //tell game 1 repetition is done
        Serial.write('1'); // Send '1' back to the game
      }

      //move to equillibirum
      moveToEquilibrium(); 
      Serial.flush();
      break;


    //right and left, abduction/adduction
    case 'b':

      //repeat for as many repetitions as rep count
      for (int i = 0; i < repCount; i++) {

        //emergency stop if 'd' character is recieved
        if (Serial.available() > 0 && Serial.read() == 'd') {
          analogWrite(M2pwmPin, 0);
          return;
        }

        //move robot right until target is reached
        digitalWrite(M2directionPin, HIGH);
        while (M2encoder.getEncoderCount() / 5281.1 * 360 < MaxRight) {

          //emergency stop if 'd' character is recieved
          if (Serial.available() > 0 && Serial.read() == 'd') {
            analogWrite(M2pwmPin, 0);
            return;
        }

          //move right
          analogWrite(M2pwmPin, 50 + 15 * speed);
        }

        //stop motor
        analogWrite(M1pwmPin, 0);
        
        //hold maximum stretch for 3 seconds and maintain angle whilst
        startTime = millis();
        elapsedTime = 0;
        while (elapsedTime < 3000){
          Serial.println(elapsedTime);
          unsigned long currentTime = millis(); // Get the current time
          elapsedTime = currentTime - startTime;
          MaintainAngle(MaxRight, 2);
        }

        //move robot left until target is reached
        digitalWrite(M2directionPin, LOW);
        while (M2encoder.getEncoderCount() / 5281.1 * 360 > MaxLeft) {

          //emergency stop if 'd' character is recieved
          if (Serial.available() > 0 && Serial.read() == 'd') {
            analogWrite(M2pwmPin, 0);
            return;
        }
          analogWrite(M2pwmPin, 50 + 15 * speed);
        }

        //stop motor
        analogWrite(M1pwmPin, 0);
        
        //hold maximum stretch for 3 seconds and maintain angle whilst
        startTime = millis();
        elapsedTime = 0;
        while (elapsedTime < 3000){
          Serial.println(elapsedTime);
          unsigned long currentTime = millis(); // Get the current time
          elapsedTime = currentTime - startTime;
          MaintainAngle(MaxLeft, 3);
        }

        //tell game 1 repetition is done
        Serial.write('1'); // Send '1' back to the game
      }

      //move back to equillibrium
      moveToEquilibrium(); 
      Serial.flush();
      break;

    //both movements simultaneously, plantarflexion/dorsiflexion and abduction/adduction
    case 'c':

      //repeat depending on rep count
      for (int i = 0; i < repCount; i++) {

        //emergency stop if 'd' character is recieved
        if (Serial.available() > 0 && Serial.read() == 'd') {
          analogWrite(M1pwmPin, 0);
          analogWrite(M2pwmPin, 0);
          return;
        }

        //set directions, up and right. move until both targets are reached
        digitalWrite(M1directionPin, LOW);
        digitalWrite(M2directionPin, LOW);
        while (M1encoder.getEncoderCount() / 5281.1 * 360 < MaxUp || M2encoder.getEncoderCount() / 5281.1 * 360 > MaxLeft) {

          //emergency stop if 'd' character is recieved
          if (Serial.available() > 0 && Serial.read() == 'd') {
            analogWrite(M1pwmPin, 0);
            analogWrite(M2pwmPin, 0);
            return;
        }

          //if not reached up target but has reached right target, only move up
          if (M1encoder.getEncoderCount() / 5281.1 * 360 < MaxUp && M2encoder.getEncoderCount() / 5281.1 * 360 <= MaxLeft) {
            analogWrite(M1pwmPin, 100 + 15 * speed);
            analogWrite(M2pwmPin, 0);
          } 

          //if not reached right target but has reached up target, only move right
          else if (M1encoder.getEncoderCount() / 5281.1 * 360 >= MaxUp && M2encoder.getEncoderCount() / 5281.1 * 360 > MaxLeft) {
            analogWrite(M1pwmPin, 0);
            analogWrite(M2pwmPin, 50 + 15 * speed);
          } 

          //if neither target reached move up and right
          else {
            analogWrite(M1pwmPin, 100 + 15 * speed);
            analogWrite(M2pwmPin, 50 + 15 * speed);
          }
        }

        //turn off motor
        analogWrite(M1pwmPin, 0);
        analogWrite(M2pwmPin, 0);
        
        //hold maximum stretch for 3 seconds and maintain angle whilst
        startTime = millis();
        elapsedTime = 0;
        while (elapsedTime < 3000){
          Serial.println(elapsedTime);
          unsigned long currentTime = millis(); // Get the current time
          elapsedTime = currentTime - startTime;
          MaintainAngle(MaxUp, 1);
          MaintainAngle(MaxLeft, 3);
        }

        //switch directions, down and left, move until both targets are reached
        digitalWrite(M1directionPin, HIGH);
        digitalWrite(M2directionPin, HIGH);
        while (M1encoder.getEncoderCount() / 5281.1 * 360 > MaxDown || M2encoder.getEncoderCount() / 5281.1 * 360 < MaxRight) {

          //emergency stop if 'd' character is recieved
          if (Serial.available() > 0 && Serial.read() == 'd') {
            analogWrite(M1pwmPin, 0);
            analogWrite(M2pwmPin, 0);
            return;
          }

          //if not reached down target but has reached left target, only move down
          if (M1encoder.getEncoderCount() / 5281.1 * 360 > MaxDown && M2encoder.getEncoderCount() / 5281.1 * 360 >= MaxRight) {
            analogWrite(M1pwmPin, 50 + 15 * speed);
            analogWrite(M2pwmPin, 0);
          } 

          //if not reached left target but has reached up target, only move left
          else if (M1encoder.getEncoderCount() / 5281.1 * 360 <= MaxDown && M2encoder.getEncoderCount() / 5281.1 * 360 < MaxRight) {
            analogWrite(M1pwmPin, 0);
            analogWrite(M2pwmPin, 50 + 15 * speed);
          } 

          //if neither target reached move down and left
          else {
            analogWrite(M1pwmPin, 50 + 15 * speed);
            analogWrite(M2pwmPin, 50 + 15 * speed);
          }
        }

        //turn off motor
        analogWrite(M1pwmPin, 0);
        analogWrite(M2pwmPin, 0);
        
        //hold maximum stretch for 3 seconds and maintain angle whilst
        startTime = millis();
        elapsedTime = 0;
        while (elapsedTime < 3000){
          Serial.println(elapsedTime);
          unsigned long currentTime = millis(); // Get the current time
          elapsedTime = currentTime - startTime;
          MaintainAngle(MaxDown, 0);
          MaintainAngle(MaxRight, 2);
        }

        //tell game 1 repetition is done
        Serial.write('1'); // Send '1' back to the game
      }

      //move to equillibrium
      moveToEquilibrium(); 
      Serial.flush();
      break;

    //option to flush buffer, shouldn't usually need 
    case 'd':
      Serial.flush();
      break;

    //enter setting of equillibrium
    case 'e':
      setEquillibrium();
      break;

    //enter calibration
    case 'f':
      calibrate();
      break;

    //choose left or right, g000 = left, g111 = right
    case 'g':

      //set to left foot
      if(speed == 0){
        LeftOrRight = true;
      }

      //set to right foot
      else{
        LeftOrRight = false;
        //Serial.println("foot set to right");
      }
      break;
  }
}


//used to set position of equillibrium when exoskeleton is powered on with footplate out of position
void setEquillibrium(){

    //control movements using 1 character commands sent from Unity, do it unitl 'X' character is recieved
    while(1){
      if(Serial.available() > 0){
        char command = Serial.read();

        //exit if 'X' character recieved
        if(command == 'X'){
          break;
        }

        //move different directions and discrete steps of either 1 or 5 degrees depending on recieved character
        switch (command) {
        case 'a':
          moveUp1();
          break;
        case 'b':
          moveUp5();
          break;
        case 'c':
          moveDown1();
          break;
        case 'd':
          moveDown5();
          break;
        case 'e':
          moveRight1();
          break;
        case 'f':
          moveRight5();
          break;
        case 'g':
          moveLeft1();
          break;
        case 'h':
          moveLeft5();
          break;
        }
      }
    }

    //once exited, reset encoder count so it becomes position of equillibrium
    M1encoder.setEncoderCount(0);
    M2encoder.setEncoderCount(0);

}


//calibration procedure designed to set maxium angle of stretch of inividuals ankle joint
//'P' charcater used to go back through calibration procedure, 'X' character used to go forwards through procedure
void calibrate(){

  int index = 0;

  //if index greater then 4 exit calibration
  while(index < 4){
    switch(index){
      case 0:

        //move to equillibrium and reset target angle
        moveToEquilibrium();
        TargetAngle = 0;
        while(1){

          //maintain angle set by user, 1 index tells function which angle is being set
          MaintainAngle(TargetAngle, 0);

          if(Serial.available() > 0){
            char command = Serial.read();

            //'P'command exits as its at first stage of procedure
            if(command == 'P'){
              index = 4;
              break;
            }
            
            //'X' command exits while loop, index increased after loop
            if(command == 'X'){
              break;
            }

            //switch command used to move robot in discret steps, target angle increases with it to be used in MaintainAngle function
            switch (command) {
              case 'a':
                TargetAngle--;
                moveDown1();
                break;
              case 'b':
                TargetAngle -= 5;
                moveDown5();
                break;
              case 'c':
                TargetAngle++;
                moveUp1();
                break;
              case 'd':
                TargetAngle += 5;
                moveUp5();
                break;
            }
          }
        }

        //sets MaxDown target angle once stage is complete
        MaxDown = (M1encoder.getEncoderCount() / 5281.1) * 360;

        //reset target angle
        TargetAngle = 0;
        moveToEquilibrium();

        //increase index to go to next stage
        index++;
        break;
      
      case 1:

        //move to equillibrium and reset target angle
        moveToEquilibrium();
        TargetAngle = 0;

        while(1){

          //maintain angle set by user, 2 index tells function which angle is being set
          MaintainAngle(TargetAngle, 1);

          if(Serial.available() > 0){
            char command = Serial.read();

            //goes to previous stage, take away 2 because 1 gets added at end of loop
            if(command == 'P'){
              index = index - 2;
              break;
            }

            //exits loop, index increased after loop
            if(command == 'X'){
              break;
            }

            //switch command used to move robot in discret steps, target angle increases with it to be used in MaintainAngle function
            switch (command) {
              case 'a':
                TargetAngle++;
                moveUp1();
                break;
              case 'b':
                TargetAngle += 5;
                moveUp5();
                break;
              case 'c':
                TargetAngle--;
                moveDown1();
                break;
              case 'd':
                TargetAngle -= 5;
                moveDown5();
                break;
            }
          }
        }

        //sets MaxDown target angle once stage is complete
        MaxUp = (M1encoder.getEncoderCount() / 5281.1) * 360;

        //reset target angle
        TargetAngle = 0;
        moveToEquilibrium();

        //increase index to go to next stage
        index++;
        break;

      case 2:

        //move to equillibrium and reset target angle
        moveToEquilibrium();
        TargetAngle = 0;

        //left or right changes which angle is being maintained
        if(!LeftOrRight){
          maintainIndex = 2;
          }
        else{
          maintainIndex = 3;
        }
        while(1){

          //maintain angle set by user, index tells function which angle is being set
          MaintainAngle(TargetAngle, maintainIndex);


          if(Serial.available() > 0){
            char command = Serial.read();

            //goes to previous stage, take away 2 because 1 gets added at end of loop
            if(command == 'P'){
              index = index - 2;
              break;
            }

            //exits loop, index increased after loop
            if(command == 'X'){
              break;
            }

            //switch command used to move robot in discret steps, target angle increases with it to be used in MaintainAngle function
            //which direction is being set depends on left or right foot beng used
            if(!LeftOrRight){
              switch (command) {
                case 'a':
                  TargetAngle++;
                  moveRight1();
                  break;
                case 'b':
                  TargetAngle += 5;
                  moveRight5();
                  break;
                case 'c':
                  TargetAngle--;
                  moveLeft1();
                  break;
                case 'd':
                  TargetAngle -= 5;
                  moveLeft5();
                  break;
                }
              }
            else{
              switch (command){
                case 'a':
                  TargetAngle--;
                  moveLeft1();
                  break;
                case 'b':
                  TargetAngle -= 5;
                  moveLeft5();
                  break;
                case 'c':
                  TargetAngle++;
                  moveRight1();
                  break;
                case 'd':
                  TargetAngle += 5;
                  moveRight5();
                  break;
                }
            }
          }
        }

        //sets either MaxRight or MaxLeft once stage is complete
        if(!LeftOrRight){
          MaxRight = (M2encoder.getEncoderCount() / 5281.1) * 360;
        }
        else{
          MaxLeft = (M2encoder.getEncoderCount() / 5281.1) * 360;
        }

        //reset target angle
        moveToEquilibrium();
        TargetAngle = 0;

        //increase index to go to next stage
        index++;
        break;

      case 3:

        //move to equillibrium and reset target angle
        moveToEquilibrium();
        TargetAngle = 0;

        //left or right changes which angle is being maintained
        if(!LeftOrRight){
          maintainIndex = 3;
        }
        else{
          maintainIndex = 2;
        }

        while(1){

          //maintain angle set by user, index tells function which angle is being set
          MaintainAngle(TargetAngle, maintainIndex);

          if(Serial.available() > 0){
            char command = Serial.read(); 

            //goes to previous stage, take away 2 because 1 gets added at end of loop
            if(command == 'P'){
              index = index - 2;
              break;
            }

            //exits loop, index increased after loop
            if(command == 'X'){
                break;
              }

            //switch command used to move robot in discret steps, target angle increases with it to be used in MaintainAngle function
            //which direction is being set depends on left or right foot beng used
            if(!LeftOrRight){
              switch (command){
                case 'a':
                  TargetAngle -= 1;
                  moveLeft1();
                  break;
                case 'b':
                  TargetAngle -= 5;
                  moveLeft5();
                  break;
                case 'c':
                  TargetAngle += 1;
                  moveRight1();
                  break;
                case 'd':
                  TargetAngle += 5;
                  moveRight5();
                  break;
                }
              }
            else{
              switch (command){
                case 'a':
                  TargetAngle += 1;
                  moveRight1();
                  break;
                case 'b':
                  TargetAngle += 5;
                  moveRight5();
                  break;
                case 'c':
                  TargetAngle -= 1;
                  moveLeft1();
                  break;
                case 'd':
                  TargetAngle -= 5;
                  moveLeft5();
                  break;
                }
            }
        }
      }

      //sets either MaxRight or MaxLeft once stage is complete
      if(!LeftOrRight){
        MaxLeft = (M2encoder.getEncoderCount() / 5281.1) * 360;
      }
      else{
        MaxRight = (M2encoder.getEncoderCount() / 5281.1) * 360;
      }

      //reset angle
      moveToEquilibrium();
      TargetAngle = 0;

      //go to next stage, exits if index = 4
      index++;
      break;
    }
  }

  //flush just to be sure no remaining data that may interfere with other things
  Serial.flush();
}

//moves up by 1 degree
void moveUp1() {
  long original_position = M1encoder.getEncoderCount() / 5281.1 * 360;
  digitalWrite(M1directionPin, LOW);
  while (1) {
    if(M1encoder.getEncoderCount() / 5281.1 * 360 >= original_position + 1){
      break;
    }
    analogWrite(M1pwmPin, 75);
  }
  analogWrite(M1pwmPin, 0);
}

//moves up by 5 degree
void moveUp5() {
  long original_position = M1encoder.getEncoderCount() / 5281.1 * 360;
  digitalWrite(M1directionPin, LOW);
  while (1) {
    if(M1encoder.getEncoderCount() / 5281.1 * 360 >= original_position + 5){
      break;
    }
    //Serial.println(M1encoder.getEncoderCount() / 5281.1 * 360);
    analogWrite(M1pwmPin, 75);
  }
  analogWrite(M1pwmPin, 0);
}

//moves down by 1 degree
void moveDown1() {
  long original_position = M1encoder.getEncoderCount() / 5281.1 * 360;
  digitalWrite(M1directionPin, HIGH);
  while (1) {
    if(M1encoder.getEncoderCount() / 5281.1 * 360 <= original_position - 1){
      break;
    }
    analogWrite(M1pwmPin, 75);
  }
  analogWrite(M1pwmPin, 0);
}

//moves down by 5 degree
void moveDown5() {
  long original_position = M1encoder.getEncoderCount() / 5281.1 * 360;
  digitalWrite(M1directionPin, HIGH);
  while (1) {
    if(M1encoder.getEncoderCount() / 5281.1 * 360 <= original_position - 5){
      break;
    }
    //Serial.println(M1encoder.getEncoderCount() / 5281.1 * 360);
    analogWrite(M1pwmPin, 75);
  }
  analogWrite(M1pwmPin, 0);
}

//moves right by 1 degree
void moveRight1() {
  long original_position = M2encoder.getEncoderCount() / 5281.1 * 360;
  digitalWrite(M2directionPin, HIGH);
  while (1) {
    if(M2encoder.getEncoderCount() / 5281.1 * 360 >= original_position + 1){
      break;
    }
    analogWrite(M2pwmPin, 75);
  }
  analogWrite(M2pwmPin, 0);
}

//moves right by 5 degree
void moveRight5() {
  long original_position = M2encoder.getEncoderCount() / 5281.1 * 360;
  digitalWrite(M2directionPin, HIGH);
  while (1) {
    if(M2encoder.getEncoderCount() / 5281.1 * 360 >= original_position + 5){
      break;
    }
    //Serial.println(M2encoder.getEncoderCount() / 5281.1 * 360);
    analogWrite(M2pwmPin, 75);
  }
  analogWrite(M2pwmPin, 0);
}

//moves left by 1 degree
void moveLeft1() {
  long original_position = M2encoder.getEncoderCount() / 5281.1 * 360;
  digitalWrite(M2directionPin, LOW);
  while (1) {
    if(M2encoder.getEncoderCount() / 5281.1 * 360 <= original_position - 1){
      break;
    }
    //Serial.println(M2encoder.getEncoderCount() / 5281.1 * 360);
    analogWrite(M2pwmPin, 75);
  }
  analogWrite(M2pwmPin, 0);
}

//moves left by 5 degree
void moveLeft5() {
  long original_position = M2encoder.getEncoderCount() / 5281.1 * 360;
  digitalWrite(M2directionPin, LOW);
  while (1) {
    if(M2encoder.getEncoderCount() / 5281.1 * 360 <= original_position - 5){
      break;
    }
    analogWrite(M2pwmPin, 75);
    //Serial.println(M2encoder.getEncoderCount() / 5281.1 * 360);
    //analogWrite(M2pwmPin, 100);
  }
  analogWrite(M2pwmPin, 0);
}

//moves robot back to position of equilibrium, goes through each case of either too far up, down, left or right
void moveToEquilibrium(){

  //exit if already at equillibrium
  if(M1encoder.getEncoderCount() == 0 && M2encoder.getEncoderCount() == 0){
    return;
  }

  //if too far up, move down until at 0
  else if(M1encoder.getEncoderCount() > 0){
    digitalWrite(M1directionPin, HIGH);
    while(1){
      if(M1encoder.getEncoderCount() <= 0){
        break;
      }
      analogWrite(M1pwmPin, 50 + 15 * speed);
    }
    analogWrite(M1pwmPin, 0);
  }

  //if too far down, move up until at 0
  else if(M1encoder.getEncoderCount() < 0){
    digitalWrite(M1directionPin, LOW);
    while(1){
      if(M1encoder.getEncoderCount() >= 0){
        break;
      }
      analogWrite(M1pwmPin, 50 + 15 * speed);
    }
    analogWrite(M1pwmPin, 0);
  }

  //if too far right, move left until at 0
  if(M2encoder.getEncoderCount() > 0){
    digitalWrite(M2directionPin, LOW);
    while(1){
      if(M2encoder.getEncoderCount() <= 0){
        break;
      }
      analogWrite(M2pwmPin, 50 + 15 * speed);
    }
    analogWrite(M2pwmPin, 0);
  }

  //if too far left, move right until at 0
  else if(M2encoder.getEncoderCount() < 0){
    digitalWrite(M2directionPin, HIGH);
    while(1){
      if(M2encoder.getEncoderCount() >= 0){
        break;
      }
      analogWrite(M2pwmPin, 50 + 15 * speed);
    }
    analogWrite(M2pwmPin, 0);
  }
}


//used to maintain angles during calibration
void MaintainAngle(int target, int index){
  switch(index){

    //maintain downwards angle
    case 0:
      digitalWrite(M1directionPin, HIGH);
      if(M1encoder.getEncoderCount() / 5281.1 * 360 > target){
        analogWrite(M1pwmPin, 100);
      }
      else{
        analogWrite(M1pwmPin, 0);
      }
    break;

    //maintain upwards angle
    case 1:
      digitalWrite(M1directionPin, LOW);
      if(M1encoder.getEncoderCount() / 5281.1 * 360 < target){
        analogWrite(M1pwmPin, 50);
      }
      else{
        analogWrite(M1pwmPin, 0);
      }
    break;

    //maintain right angle
    case 2:
      digitalWrite(M2directionPin, HIGH);
      if(M2encoder.getEncoderCount() / 5281.1 * 360 < target){
        analogWrite(M2pwmPin, 75);
      }
      else{
        analogWrite(M2pwmPin, 0);
      }
    break;

    //maintain left angle
    case 3:
      digitalWrite(M2directionPin, LOW);
      if(M2encoder.getEncoderCount() / 5281.1 * 360 > target){
        analogWrite(M2pwmPin, 75);
      }
      else{
        analogWrite(M2pwmPin, 0);
      }
    break;
  }
}
