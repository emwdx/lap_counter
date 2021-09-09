#include <ComponentObject.h>
#include <RangeSensor.h>
#include <SparkFun_VL53L1X.h>
#include <vl53l1x_class.h>
#include <vl53l1_error_codes.h>


/*
  VL53L1X Code
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 4th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SparkFun labored with love to create this code. Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/14667

  This example prints the distance to an object.

  Are you getting weird readings? Be sure the vacuum tape has been removed from the sensor.
*/

#include <Wire.h>

#include <M5Stack.h>
//Optional interrupt and shutdown pins.
#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3

#define START_UP 0
#define IN_RANGE 1
#define MED_RANGE 2
#define LONG_RANGE 3
#define CLEAR_RANGE 4
#define LAP_COMPLETE 5
#define PASS_COMPLETE 6

#define IN_RANGE_BOUNDARY 80.0
#define MED_RANGE_BOUNDARY 130.0
#define LONG_RANGE_BOUNDARY 150.0
#define CLEAR_BOUNDARY 180.0

int lap_count = 0;
int pass_count = 0;
int state = START_UP;
float current_distance = 0.0;
boolean m5init = false;
float old_lap_count = -10.0;
float old_pass_count = -10.0;

SFEVL53L1X distanceSensor;
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

void setup(void)
{
  
 M5.begin();  //Init M5Core.  初始化 M5Core
  M5.Power.begin(); //Init Power module.  初始化电源模块
  
  M5.Lcd.setTextColor(GREEN);
  
  //Serial.begin(115200);
  //Serial.println("VL53L1X Qwiic Test");

  Wire.begin();

for(int i = 0;i<10;i++){
  if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    
      
  }

}
  
  Serial.println("Sensor online!");
  //Start M5Stack functions
 
  
  //delay(1000);
}

float get_distance_cm(void){
  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
  return float(distance) / 10.0;
}

void update_system(){
  current_distance = get_distance_cm();
}

int evaluate_state(int state){
  switch(state){
    case START_UP:
      return IN_RANGE;
      break;
    case LAP_COMPLETE:
      return IN_RANGE;
     break;
    case PASS_COMPLETE:
      return IN_RANGE;
      break;
    case IN_RANGE:
      if(current_distance > MED_RANGE_BOUNDARY && current_distance <= LONG_RANGE_BOUNDARY){
        return MED_RANGE;
      }
      else if (current_distance > LONG_RANGE_BOUNDARY){
        return LONG_RANGE;
      }
      
      break;
    case MED_RANGE:
      if(current_distance > LONG_RANGE_BOUNDARY && current_distance < CLEAR_BOUNDARY){
        return LONG_RANGE;
      }
      
      else if(current_distance < IN_RANGE_BOUNDARY){
        return LAP_COMPLETE;
      }
      else if(current_distance > CLEAR_BOUNDARY){
        return PASS_COMPLETE;
      }
      break;
    case LONG_RANGE:
      if(current_distance < MED_RANGE_BOUNDARY && current_distance > IN_RANGE_BOUNDARY){
        return MED_RANGE;
      }
    
      if(current_distance <= IN_RANGE_BOUNDARY){
        return PASS_COMPLETE;
      }
      break;
     
    default:
      Serial.println("weird...something unexpected happened");
      
      break;
    }
    return state;
  
}
void react_to_state(int state){
  
  switch(state){
  case LAP_COMPLETE:
    lap_count++;
    break;
  case PASS_COMPLETE:
    pass_count++;
    break;
  default:
  /*
    Serial.print(current_distance);
    Serial.print(",");
    Serial.print(lap_count);
    Serial.print(",");
    Serial.print(state);
 */
    break;
  }
  if(lap_count != old_lap_count || pass_count != old_pass_count){
      M5.Lcd.fillScreen(0);
    
    M5.Lcd.setTextSize(50);
    M5.Lcd.setCursor(10,0);
    M5.Lcd.print(String(lap_count));
    M5.Lcd.setCursor(10,100);
    M5.Lcd.print(String(pass_count));
    if(lap_count != old_lap_count){
      old_lap_count = lap_count;
    }
    if(pass_count != old_pass_count){
    old_pass_count = pass_count;
    }
  }
  
}


void loop(void)
{
  
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  
  //Serial.print("Distance(cm): ");
  //Serial.print(get_distance_cm());

  update_system();
  state = evaluate_state(state);
  react_to_state(state);
  //Serial.println();
  M5.update();
  delay(10);
  
  
}
