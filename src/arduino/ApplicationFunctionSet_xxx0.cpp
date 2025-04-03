/*
 * @Author: ELEGOO
 * @Date: 2019-10-22 11:59:09
 * @LastEditTime: 2021-01-05 09:30:14
 * @LastEditors: Changhua
 * @Description: Smart Robot Car V4.0
 * @FilePath:
 */
#include <avr/wdt.h>
// #include <hardwareSerial.h>
#include <SoftwareSerial.h>
#include <stdio.h>
#include <string.h>
#include "ApplicationFunctionSet_xxx0.h"
#include "DeviceDriverSet_xxx0.h"

#include "ArduinoJson-v6.11.1.h" //ArduinoJson
#include "MPU6050_getdata.h"

#define _is_print 1
#define _Test_print 0

ApplicationFunctionSet Application_FunctionSet;
/*Hardware device object list*/
MPU6050_getdata AppMPU6050getdata;
DeviceDriverSet_RBGLED AppRBG_LED;
DeviceDriverSet_Key AppKey;
DeviceDriverSet_ITR20001 AppITR20001;
DeviceDriverSet_Voltage AppVoltage;

DeviceDriverSet_Motor AppMotor;
DeviceDriverSet_ULTRASONIC AppULTRASONIC;
DeviceDriverSet_Servo AppServo;
DeviceDriverSet_IRrecv AppIRrecv;
/*f(x) int */
static boolean
function_xxx(long x, long s, long e) // f(x)
{
  if (s <= x && x <= e)
    return true;
  else
    return false;
}
static void
delay_xxx(uint16_t _ms)
{
  wdt_reset();
  for (unsigned long i = 0; i < _ms; i++)
  {
    delay(1);
  }
}

/*Movement Direction Control List*/
enum SmartRobotCarMotionControl
{
  Forward,       //(1)
  Backward,      //(2)
  Left,          //(3)
  Right,         //(4)
  LeftForward,   //(5)
  LeftBackward,  //(6)
  RightForward,  //(7)
  RightBackward, //(8)
  stop_it        //(9)
}; // direction方向:（1）、（2）、 （3）、（4）、（5）、（6）

/*Mode Control List*/
enum SmartRobotCarFunctionalModel
{
  Standby_mode, /*Standby Mode*/
  Rocker_mode,
  TraceBased_mode, /*Line Tracking Mode*/
  Sequence_mode,   /*Sequence Mode*/
  Test_mode,
};

enum SystemCondition
{
  RED,
  GREEN,
  BLUE
}; // System condition for sequence control

// Global variables
uint8_t sequenceStep = 0;                      // Track the current step in the sequence
unsigned long sequenceTimestamp = 0;           // For non-blocking timing
bool destinationReached = false;               // Flag to indicate if the destination is reached
unsigned long lastSerialReadTime = 0;          // Timestamp of the last Serial read
const unsigned long serialReadInterval = 2000; // Minimum interval (in milliseconds) between valid Serial reads

/*Application Management list*/
struct Application_xxx
{
  SmartRobotCarMotionControl Motion_Control;
  SmartRobotCarFunctionalModel Functional_Mode;
  SystemCondition SystemCondition; // System condition for sequence control
};
Application_xxx Application_SmartRobotCarxxx0;

bool ApplicationFunctionSet_SmartRobotCarLeaveTheGround(void);
void ApplicationFunctionSet_SmartRobotCarLinearMotionControl(SmartRobotCarMotionControl direction, uint8_t directionRecord, uint8_t speed, uint8_t Kp, uint8_t UpperLimit);
void ApplicationFunctionSet_SmartRobotCarMotionControl(SmartRobotCarMotionControl direction, uint8_t is_speed);

void ApplicationFunctionSet::ApplicationFunctionSet_Init(void)
{
  bool res_error = true;
  Serial.begin(115200);
  AppVoltage.DeviceDriverSet_Voltage_Init();
  AppMotor.DeviceDriverSet_Motor_Init();
  AppServo.DeviceDriverSet_Servo_Init(90);
  AppKey.DeviceDriverSet_Key_Init();
  AppRBG_LED.DeviceDriverSet_RBGLED_Init(20);
  AppIRrecv.DeviceDriverSet_IRrecv_Init();
  AppULTRASONIC.DeviceDriverSet_ULTRASONIC_Init();
  AppITR20001.DeviceDriverSet_ITR20001_Init();
  res_error = AppMPU6050getdata.MPU6050_dveInit();
  AppMPU6050getdata.MPU6050_calibration();
  Serial.println("System Initialization Complete!");
  // while (Serial.read() >= 0)
  // {
  //   /*Clear serial port buffer...*/
  // }
  Application_SmartRobotCarxxx0.Functional_Mode = Standby_mode;
}

/*ITR20001 Check if the car leaves the ground*/
static bool ApplicationFunctionSet_SmartRobotCarLeaveTheGround(void)
{
  if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() > Application_FunctionSet.TrackingDetection_V &&
      AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() > Application_FunctionSet.TrackingDetection_V &&
      AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() > Application_FunctionSet.TrackingDetection_V)
  {
    Application_FunctionSet.Car_LeaveTheGround = false;
    return false;
  }
  else
  {
    Application_FunctionSet.Car_LeaveTheGround = true;
    return true;
  }
}
/*
  Straight line movement control：For dual-drive motors, due to frequent motor coefficient deviations and many external interference factors,
  it is difficult for the car to achieve relative Straight line movement. For this reason, the feedback of the yaw control loop is added.
  direction：only forward/backward
  directionRecord：Used to update the direction and position data (Yaw value) when entering the function for the first time.
  speed：the speed range is 0~255
  Kp：Position error proportional constant（The feedback of improving location resuming status，will be modified according to different mode），improve damping control.
  UpperLimit：Maximum output upper limit control
*/
static void ApplicationFunctionSet_SmartRobotCarLinearMotionControl(SmartRobotCarMotionControl direction, uint8_t directionRecord, uint8_t speed, uint8_t Kp, uint8_t UpperLimit)
{
  static float Yaw; // Yaw
  static float yaw_So = 0;
  static uint8_t en = 110;
  static unsigned long is_time;
  if (en != directionRecord || millis() - is_time > 10)
  {
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_void, /*speed_A*/ 0,
                                           /*direction_B*/ direction_void, /*speed_B*/ 0, /*controlED*/ control_enable); // Motor control
    AppMPU6050getdata.MPU6050_dveGetEulerAngles(&Yaw);
    is_time = millis();
  }
  // if (en != directionRecord)
  if (en != directionRecord || Application_FunctionSet.Car_LeaveTheGround == false)
  {
    en = directionRecord;
    yaw_So = Yaw;
  }
  // Add proportional constant Kp to increase rebound effect
  int R = (Yaw - yaw_So) * Kp + speed;
  if (R > UpperLimit)
  {
    R = UpperLimit;
  }
  else if (R < 10)
  {
    R = 10;
  }
  int L = (yaw_So - Yaw) * Kp + speed;
  if (L > UpperLimit)
  {
    L = UpperLimit;
  }
  else if (L < 10)
  {
    L = 10;
  }
  if (direction == Forward) // Forward
  {
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ R,
                                           /*direction_B*/ direction_just, /*speed_B*/ L, /*controlED*/ control_enable);
  }
  else if (direction == Backward) // Backward
  {
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ L,
                                           /*direction_B*/ direction_back, /*speed_B*/ R, /*controlED*/ control_enable);
  }
}
/*
  Movement Direction Control:
  Input parameters:     1# direction:Forward（1）、Backward（2）、 Left（3）、Right（4）、LeftForward（5）、LeftBackward（6）、RightForward（7）RightBackward（8）
                        2# speed(0--255)
*/
static void ApplicationFunctionSet_SmartRobotCarMotionControl(SmartRobotCarMotionControl direction, uint8_t is_speed)
{
  ApplicationFunctionSet Application_FunctionSet;
  static uint8_t directionRecord = 0;
  uint8_t Kp, UpperLimit;
  uint8_t speed = is_speed;
  // Control mode that requires straight line movement adjustment（Car will has movement offset easily in the below mode，the movement cannot achieve the effect of a relatively straight direction
  // so it needs to add control adjustment）
  switch (Application_SmartRobotCarxxx0.Functional_Mode)
  {
  case Rocker_mode:
    Kp = 10;
    UpperLimit = 255;
    break;
  case TraceBased_mode:
    Kp = 10;
    UpperLimit = 255;
    break;
  default:
    Kp = 10;
    UpperLimit = 255;
    break;
  }
  switch (direction)
  {
  case /* constant-expression */
      Forward:
    /* code */
    if (Application_SmartRobotCarxxx0.Functional_Mode == TraceBased_mode)
    {
      AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ speed,
                                             /*direction_B*/ direction_just, /*speed_B*/ speed, /*controlED*/ control_enable); // Motor control
    }
    else
    { // When moving forward, enter the direction and position approach control loop processing
      ApplicationFunctionSet_SmartRobotCarLinearMotionControl(Forward, directionRecord, speed, Kp, UpperLimit);
      directionRecord = 1;
    }

    break;
  case /* constant-expression */ Backward:
    /* code */
    if (Application_SmartRobotCarxxx0.Functional_Mode == TraceBased_mode)
    {
      AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ speed,
                                             /*direction_B*/ direction_back, /*speed_B*/ speed, /*controlED*/ control_enable); // Motor control
    }
    else
    { // When moving backward, enter the direction and position approach control loop processing
      ApplicationFunctionSet_SmartRobotCarLinearMotionControl(Backward, directionRecord, speed, Kp, UpperLimit);
      directionRecord = 2;
    }

    break;
  case /* constant-expression */ Left:
    /* code */
    directionRecord = 3;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ speed,
                                           /*direction_B*/ direction_back, /*speed_B*/ speed, /*controlED*/ control_enable); // Motor control
    break;
  case /* constant-expression */ Right:
    /* code */
    directionRecord = 4;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ speed,
                                           /*direction_B*/ direction_just, /*speed_B*/ speed, /*controlED*/ control_enable); // Motor control
    break;
  case /* constant-expression */ LeftForward:
    /* code */
    directionRecord = 5;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ speed,
                                           /*direction_B*/ direction_just, /*speed_B*/ speed / 2, /*controlED*/ control_enable); // Motor control
    break;
  case /* constant-expression */ LeftBackward:
    /* code */
    directionRecord = 6;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ speed,
                                           /*direction_B*/ direction_back, /*speed_B*/ speed / 2, /*controlED*/ control_enable); // Motor control
    break;
  case /* constant-expression */ RightForward:
    /* code */
    directionRecord = 7;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ speed / 2,
                                           /*direction_B*/ direction_just, /*speed_B*/ speed, /*controlED*/ control_enable); // Motor control
    break;
  case /* constant-expression */ RightBackward:
    /* code */
    directionRecord = 8;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ speed / 2,
                                           /*direction_B*/ direction_back, /*speed_B*/ speed, /*controlED*/ control_enable); // Motor control
    break;
  case /* constant-expression */ stop_it:
    /* code */
    directionRecord = 9;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_void, /*speed_A*/ 0,
                                           /*direction_B*/ direction_void, /*speed_B*/ 0, /*controlED*/ control_enable); // Motor control

    break;
  default:
    directionRecord = 10;
    break;
  }
}
void ApplicationFunctionSet::turnAround(void)
{
  if (Application_SmartRobotCarxxx0.Functional_Mode == Test_mode)
  {
    Serial.println("Test mode: Turning around...");
    ApplicationFunctionSet_SmartRobotCarMotionControl(Left, 100);
    delay(650);                                                    // Increase the delay time for each iteration
    ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0); // Stop the car
    delay(1000);
  }
}

/*
 Robot car update sensors' data:Partial update (selective update)
*/
void ApplicationFunctionSet::ApplicationFunctionSet_SensorDataUpdate(void)
{

  // AppMotor.DeviceDriverSet_Motor_Test();
  { /*Battery voltage status update*/
    static unsigned long VoltageData_time = 0;
    static int VoltageData_number = 1;
    if (millis() - VoltageData_time > 10) // read and update the data per 10ms
    {
      VoltageData_time = millis();
      VoltageData_V = AppVoltage.DeviceDriverSet_Voltage_getAnalogue();
      if (VoltageData_V < VoltageDetection)
      {
        VoltageData_number++;
        if (VoltageData_number == 500) // Continuity to judge the latest voltage value multiple
        {
          VoltageDetectionStatus = true;
          VoltageData_number = 0;
        }
      }
      else
      {
        VoltageDetectionStatus = false;
      }
    }
  }

  // { /*value updation for the ultrasonic sensor：for the Obstacle Avoidance mode*/
  //   AppULTRASONIC.DeviceDriverSet_ULTRASONIC_Get(&UltrasoundData_cm /*out*/);
  //   UltrasoundDetectionStatus = function_xxx(UltrasoundData_cm, 0, ObstacleDetection);
  // }

  { /*value updation for the IR sensors on the line tracking module：for the line tracking mode*/
    TrackingData_R = AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R();
    TrackingDetectionStatus_R = function_xxx(TrackingData_R, TrackingDetection_S, TrackingDetection_E);
    TrackingData_M = AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M();
    TrackingDetectionStatus_M = function_xxx(TrackingData_M, TrackingDetection_S, TrackingDetection_E);
    TrackingData_L = AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L();
    TrackingDetectionStatus_L = function_xxx(TrackingData_L, TrackingDetection_S, TrackingDetection_E);
    // ITR20001 Check if the car leaves the ground
    ApplicationFunctionSet_SmartRobotCarLeaveTheGround();
  }

  // acquire timestamp
  // static unsigned long Test_time;
  // if (millis() - Test_time > 200)
  // {
  //   Test_time = millis();
  //   //AppITR20001.DeviceDriverSet_ITR20001_Test();
  // }
}
/*
  Startup operation requirement：
*/
void ApplicationFunctionSet::ApplicationFunctionSet_Bootup(void)
{
  Application_SmartRobotCarxxx0.Functional_Mode = Standby_mode;
}

/*RBG_LED set*/
void ApplicationFunctionSet::ApplicationFunctionSet_RGB(void)
{
  static unsigned long getAnalogue_time = 0;
  FastLED.clear(true);
  if (true == VoltageDetectionStatus) // Act on low power state？
  {
    if ((millis() - getAnalogue_time) > 3000)
    {
      getAnalogue_time = millis();
    }
  }
  unsigned long temp = millis() - getAnalogue_time;
  if (function_xxx((temp), 0, 500) && VoltageDetectionStatus == true)
  {
    switch (temp)
    {
    case /* constant-expression */ 0 ... 49:
      /* code */
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Red);
      break;
    case /* constant-expression */ 50 ... 99:
      /* code */
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Black);
      break;
    case /* constant-expression */ 100 ... 149:
      /* code */
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Red);
      break;
    case /* constant-expression */ 150 ... 199:
      /* code */
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Black);
      break;
    case /* constant-expression */ 200 ... 249:
      /* code */
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Red);
      break;
    case /* constant-expression */ 250 ... 299:
      /* code */
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Red);
      break;
    case /* constant-expression */ 300 ... 349:
      /* code */
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Black);
      break;
    case /* constant-expression */ 350 ... 399:
      /* code */
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Red);
      break;
    case /* constant-expression */ 400 ... 449:
      /* code */
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Black);
      break;
    case /* constant-expression */ 450 ... 499:
      /* code */
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Red);
      break;
    default:
      break;
    }
  }
  else if (((function_xxx((temp), 500, 3000)) && VoltageDetectionStatus == true) || VoltageDetectionStatus == false)
  {
    switch (Application_SmartRobotCarxxx0.Functional_Mode) // Act on mode control sequence
    {
    case /* constant-expression */ Standby_mode:
      /* code */
      {
        if (VoltageDetectionStatus == true)
        {
          AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Red);
          delay(30);
          AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Black);
          delay(30);
        }
        else
        {
          static uint8_t setBrightness = 0;
          static boolean et = false;
          static unsigned long time = 0;

          if ((millis() - time) > 10)
          {
            time = millis();
            if (et == false)
            {
              setBrightness += 1;
              if (setBrightness == 100)
                et = true;
            }
            else if (et == true)
            {
              setBrightness -= 1;
              if (setBrightness == 0)
                et = false;
            }
          }
          // AppRBG_LED.leds[1] = CRGB::Blue;
          AppRBG_LED.leds[0] = CRGB::Violet;
          FastLED.setBrightness(setBrightness);
          FastLED.show();
        }
      }
      break;
    case /* constant-expression */ TraceBased_mode:
      /* code */
      {
        AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Green);
      }
      break;
    case /* constant-expression */ Rocker_mode:
      /* code */
      {
        AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 2 /*Traversal_Number*/, CRGB::Violet);
      }
      break;
    default:
      break;
    }
  }
}

/*Rocker control mode*/
void ApplicationFunctionSet::ApplicationFunctionSet_Rocker(void)
{
  if (Application_SmartRobotCarxxx0.Functional_Mode == Rocker_mode)
  {
    ApplicationFunctionSet_SmartRobotCarMotionControl(Application_SmartRobotCarxxx0.Motion_Control /*direction*/, Rocker_CarSpeed /*speed*/);
  }
}

/*Line tracking mode*/
void ApplicationFunctionSet::ApplicationFunctionSet_Tracking(void)
{
  static boolean timestamp = true;
  static boolean BlindDetection = true;
  static unsigned long MotorRL_time = 0;
  if (Application_SmartRobotCarxxx0.Functional_Mode == TraceBased_mode)
  {
    if (Car_LeaveTheGround == false) // Check if the car leaves the ground
    {
      ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0);
      return;
    }

    // int getAnaloguexxx_L = AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L();
    // int getAnaloguexxx_M = AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M();
    // int getAnaloguexxx_R = AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R();
#if _Test_print
    static unsigned long print_time = 0;
    if (millis() - print_time > 500)
    {
      print_time = millis();
      Serial.print("ITR20001_getAnaloguexxx_L=");
      Serial.println(getAnaloguexxx_L);
      Serial.print("ITR20001_getAnaloguexxx_M=");
      Serial.println(getAnaloguexxx_M);
      Serial.print("ITR20001_getAnaloguexxx_R=");
      Serial.println(getAnaloguexxx_R);
    }
#endif

    // **Destination/Origin Detection Logic**
    if (TrackingDetectionStatus_M && TrackingDetectionStatus_L && TrackingDetectionStatus_R)
    {
      // All sensors detect the line, indicating a marker
      if (!destinationReached)
      {
        Serial.println("Destination detected!");
        destinationReached = true; // Set the destination flag
        if (sequenceStep == 0)     // Check if the sequence is already running
        {
          Application_SmartRobotCarxxx0.Functional_Mode = Standby_mode;
        }
        else
        {
          Application_SmartRobotCarxxx0.Functional_Mode = Sequence_mode; // Change to sequence mode
        }
      }
    }

    if (TrackingDetectionStatus_M)
    {
      /*Achieve straight and uniform speed movement*/
      ApplicationFunctionSet_SmartRobotCarMotionControl(Forward, 100);
      timestamp = true;
      BlindDetection = true;
    }
    else if (TrackingDetectionStatus_R)
    {
      /*Turn right*/
      ApplicationFunctionSet_SmartRobotCarMotionControl(Right, 100);
      timestamp = true;
      BlindDetection = true;
    }
    else if (TrackingDetectionStatus_L)
    {
      /*Turn left*/
      ApplicationFunctionSet_SmartRobotCarMotionControl(Left, 100);
      timestamp = true;
      BlindDetection = true;
    }
    else ////The car is not on the black line. execute Blind scan
    {
      if (timestamp == true) // acquire timestamp
      {
        timestamp = false;
        MotorRL_time = millis();
        ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0);
      }
      /*Blind Detection*/
      if ((function_xxx((millis() - MotorRL_time), 0, 200) || function_xxx((millis() - MotorRL_time), 1600, 2000)) && BlindDetection == true)
      {
        ApplicationFunctionSet_SmartRobotCarMotionControl(Right, 100);
      }
      else if (((function_xxx((millis() - MotorRL_time), 200, 1600))) && BlindDetection == true)
      {
        ApplicationFunctionSet_SmartRobotCarMotionControl(Left, 100);
      }
      else if ((function_xxx((millis() - MotorRL_time), 3000, 3500))) // Blind Detection ...s ?
      {
        BlindDetection = false;
        ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0);
      }
    }
  }
  else if (false == timestamp)
  {
    BlindDetection = true;
    timestamp = true;
    MotorRL_time = 0;
  }
}

/*Servo motor control*/
void ApplicationFunctionSet::ApplicationFunctionSet_Servo(uint8_t Set_Servo)
{
  static int z_angle = 9;
  static int y_angle = 9;
  uint8_t temp_Set_Servo = Set_Servo;

  switch (temp_Set_Servo)
  {
  case 1 ... 2:
  {
    if (1 == temp_Set_Servo)
    {
      y_angle -= 1;
    }
    else if (2 == temp_Set_Servo)
    {
      y_angle += 1;
    }
    if (y_angle <= 3) // minimum control
    {
      y_angle = 3;
    }
    if (y_angle >= 11) // maximum control
    {
      y_angle = 11;
    }
    AppServo.DeviceDriverSet_Servo_controls(/*uint8_t Servo--y*/ 2, /*unsigned int Position_angle*/ y_angle);
  }
  break;

  case 3 ... 4:
  {
    if (3 == temp_Set_Servo)
    {
      z_angle += 1;
    }
    else if (4 == temp_Set_Servo)
    {
      z_angle -= 1;
    }

    if (z_angle <= 1) // minimum control
    {
      z_angle = 1;
    }
    if (z_angle >= 17) // maximum control
    {
      z_angle = 17;
    }
    AppServo.DeviceDriverSet_Servo_controls(/*uint8_t Servo--z*/ 1, /*unsigned int Position_angle*/ z_angle);
  }
  break;
  case 5:
    AppServo.DeviceDriverSet_Servo_controls(/*uint8_t Servo--y*/ 2, /*unsigned int Position_angle*/ 9);
    AppServo.DeviceDriverSet_Servo_controls(/*uint8_t Servo--z*/ 1, /*unsigned int Position_angle*/ 9);
    break;
  default:
    break;
  }
}
/*Standby mode*/
void ApplicationFunctionSet::ApplicationFunctionSet_Standby(void)
{
  static bool is_ED = true;
  static uint8_t cout = 0;
  static unsigned long lastReceivedMillis = 0; // Store the last received data

  if (Application_SmartRobotCarxxx0.Functional_Mode == Standby_mode)
  {
    ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0);

    if (true == is_ED) // Used to zero yaw raw data
    {
      static unsigned long timestamp;
      if (millis() - timestamp > 20)
      {
        timestamp = millis();
        if (ApplicationFunctionSet_SmartRobotCarLeaveTheGround())
        {
          cout += 1;
        }
        else
        {
          cout = 0;
        }
        if (cout > 10)
        {
          is_ED = false;
          AppMPU6050getdata.MPU6050_calibration();
        }
      }
    }

    // Check for incoming serial data
    if (Serial.available() > 0)
    {
      Serial.print("Last Received Time: ");
      Serial.println(lastReceivedMillis);                   // Print the last received time
      String receivedString = Serial.readStringUntil('\n'); // Read until newline
      receivedString.trim();                                // Remove any leading/trailing whitespace
      Serial.println(receivedString);                       // Print the received string
      // Check if the data is new
      // get the time embedded in the string from new received string like "{RED,123456789}"
      unsigned long receivedMillis = receivedString.substring(receivedString.indexOf(',') + 1, receivedString.indexOf('}')).toInt();
      unsigned long timepassed = receivedMillis - lastReceivedMillis;
      Serial.print("Time passed: ");
      Serial.println(timepassed); // Print the last received time
      if (timepassed > 2000)      // Compare the millisecond values
      {
        // Process the received string
        if (receivedString.startsWith("{RED"))
        {
          Application_SmartRobotCarxxx0.SystemCondition = RED;
          Serial.println("System Condition: RED");
          Application_SmartRobotCarxxx0.Functional_Mode = Sequence_mode; // Start the sequence
        }
        else if (receivedString.startsWith("{GREEN"))
        {
          Application_SmartRobotCarxxx0.SystemCondition = GREEN;
          Serial.println("System Condition: GREEN");
          Application_SmartRobotCarxxx0.Functional_Mode = Sequence_mode; // Start the sequence
        }
        else if (receivedString.startsWith("{BLUE"))
        {
          Application_SmartRobotCarxxx0.SystemCondition = BLUE;
          Serial.println("System Condition: BLUE");
          Application_SmartRobotCarxxx0.Functional_Mode = Sequence_mode; // Start the sequence
        }
        else
        {
          Serial.println("Unknown command received");
        }
      }
      else
      {
        Serial.println("Ignoring old Serial data");
      }
      lastReceivedMillis = receivedMillis; // Update the last received time
    }
  }
}

/*Sequence control function*/
void ApplicationFunctionSet::PerformSequence(void)
{
  if (Application_SmartRobotCarxxx0.Functional_Mode == Sequence_mode)
  {
    switch (sequenceStep)
    {
    case 0: // Step 1: Turn to a specific direction
      Serial.println("Step 1: Turning to a specific direction...");
      if (Application_SmartRobotCarxxx0.SystemCondition == RED)
      {
        Serial.println("Condition 1: Turning left...");
        ApplicationFunctionSet_SmartRobotCarMotionControl(Left, 100);
      }
      else if (Application_SmartRobotCarxxx0.SystemCondition == GREEN)
      {
        Serial.println("Condition 2: Turning right...");
        ApplicationFunctionSet_SmartRobotCarMotionControl(Right, 100);
      }
      else if (Application_SmartRobotCarxxx0.SystemCondition == BLUE)
      {
        Serial.println("Condition 3: No turning...");
      }
      delay(650);                                                    // Wait for 1 second
      ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0); // Stop the car
      delay(500);
      sequenceStep++;
      break;

    case 1: // Step 2: Move a small distance
      Serial.println("Step 2: Moving a small distance...");
      ApplicationFunctionSet_SmartRobotCarMotionControl(Forward, 100); // Move forward
      delay(500);                                                      // Wait for 2 seconds
      ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0);   // Stop the car
      delay(500);
      sequenceStep++;
      break;

    case 2: // Step 3: Line tracking until destination
      Serial.println("Step 3: Line tracking until destination...");
      if (destinationReached) // Replace with actual condition
      {
        sequenceStep++;
        sequenceTimestamp = millis();                                  // Reset the timestamp
        Application_SmartRobotCarxxx0.Functional_Mode = Sequence_mode; // Return to sequence mode
        Serial.println("Step 4: Waiting for 5 seconds...");
      }
      else
      {
        Application_SmartRobotCarxxx0.Functional_Mode = TraceBased_mode; // Switch to line tracking mode
        // Wait for the destination to be reached (handled in line tracking logic)
      }
      break;

    case 3:                                                          // Step 4: Wait for a certain time
      ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0); // Stop the car
      if (millis() - sequenceTimestamp > 5000)                       // Wait for 5 seconds
      {
        sequenceStep++;
        Serial.println("Step 4: Waited for 5 seconds...");
      }
      break;

    case 4: // Step 5: Turn 180 degrees
      Serial.println("Step 5: Turning 180 degrees...");
      ApplicationFunctionSet_SmartRobotCarMotionControl(Left, 100);  // Turn left
      delay(1250);                                                   // Wait for 2 seconds
      ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0); // Stop the car
      delay(500);
      destinationReached = false;
      sequenceStep++;
      break;

    case 5: // Step 6: Line tracking back to origin
      Serial.println("Step 6: Line tracking back to origin...");
      if (destinationReached) // Replace with actual condition
      {
        sequenceStep++;
        Application_SmartRobotCarxxx0.Functional_Mode = Sequence_mode; // Return to sequence mode
      }
      else
      {
        Application_SmartRobotCarxxx0.Functional_Mode = TraceBased_mode; // Switch to line tracking mode
        // Wait for the destination to be reached (handled in line tracking logic)
      }
      break;

    case 6: // Step 7: Move forward a small distance
      Serial.println("Step 7: Moving forward a small distance...");
      ApplicationFunctionSet_SmartRobotCarMotionControl(Forward, 100); // Move forward
      if (Application_SmartRobotCarxxx0.SystemCondition == BLUE)
      {
        delay(750);
      }
      else
      {
        delay(600);
      }                                                              // Wait for 1.5 seconds
      ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0); // Stop the car
      sequenceStep++;
      break;

    case 7: // Step 8: Turn to initial direction
      Serial.println("Step 8: Turning to initial direction...");
      if (Application_SmartRobotCarxxx0.SystemCondition == RED)
      {
        ApplicationFunctionSet_SmartRobotCarMotionControl(Left, 100); // Turn right
        delay(650);
      }
      else if (Application_SmartRobotCarxxx0.SystemCondition == GREEN)
      {
        ApplicationFunctionSet_SmartRobotCarMotionControl(Right, 100); // Turn left
        delay(650);
      }
      else if (Application_SmartRobotCarxxx0.SystemCondition == BLUE)
      {
        ApplicationFunctionSet_SmartRobotCarMotionControl(Left, 100); // Turn 180
        delay(1250);
      }
      ApplicationFunctionSet_SmartRobotCarMotionControl(stop_it, 0); // Stop the car
      delay(500);
      destinationReached = false;
      sequenceStep++;
      break;

    case 8: // Step 9: Enter standby mode
      Serial.println("Step 9: Returning to standby...");
      Application_SmartRobotCarxxx0.Functional_Mode = Standby_mode; // Switch to standby mode
      sequenceStep = 0;                                             // Reset the sequence
      break;

    default:
      Application_SmartRobotCarxxx0.Functional_Mode = Standby_mode;
      sequenceStep = 0;
      break;
    }
  }
}

/*Key command*/
void ApplicationFunctionSet::ApplicationFunctionSet_KeyCommand(void)
{
  uint8_t get_keyValue;
  static uint8_t temp_keyValue = keyValue_Max;
  AppKey.DeviceDriverSet_key_Get(&get_keyValue);

  if (temp_keyValue != get_keyValue)
  {
    temp_keyValue = get_keyValue; // Serial.println(get_keyValue);
    switch (get_keyValue)
    {
    case /* constant-expression */ 1:
      /* code */
      Application_SmartRobotCarxxx0.Functional_Mode = TraceBased_mode;
      break;
    case /* constant-expression */ 4:
      /* code */
      Application_SmartRobotCarxxx0.Functional_Mode = Standby_mode;
      sequenceStep = 0;
      break;
    default:

      break;
    }
  }
}
/*Infrared remote control*/
void ApplicationFunctionSet::ApplicationFunctionSet_IRrecv(void)
{
  uint8_t IRrecv_button;
  static bool IRrecv_en = false;
  if (AppIRrecv.DeviceDriverSet_IRrecv_Get(&IRrecv_button /*out*/))
  {
    IRrecv_en = true;
    // Serial.println(IRrecv_button);
  }
  if (true == IRrecv_en)
  {
    switch (IRrecv_button)
    {
    case /* constant-expression */ 1:
      /* code */
      Application_SmartRobotCarxxx0.Motion_Control = Forward;
      break;
    case /* constant-expression */ 2:
      /* code */
      Application_SmartRobotCarxxx0.Motion_Control = Backward;
      break;
    case /* constant-expression */ 3:
      /* code */
      Application_SmartRobotCarxxx0.Motion_Control = Left;
      break;
    case /* constant-expression */ 4:
      /* code */
      Application_SmartRobotCarxxx0.Motion_Control = Right;
      break;
    case /* constant-expression */ 5:
      /* code */
      Application_SmartRobotCarxxx0.Functional_Mode = Standby_mode;
      sequenceStep = 0;
      break;
    case /* constant-expression */ 6:
      /* code */ Application_SmartRobotCarxxx0.Functional_Mode = TraceBased_mode;
      break;

    case /* constant-expression */ 7:
      /* code */ Application_SmartRobotCarxxx0.Functional_Mode = Test_mode;
      break;
    case /* constant-expression */ 9:
      /* code */ if (Application_SmartRobotCarxxx0.Functional_Mode == TraceBased_mode) // Adjust the threshold of the line tracking module to adapt the actual environment
      {
        if (TrackingDetection_S < 600)
        {
          TrackingDetection_S += 10;
        }
      }

      break;
    case /* constant-expression */ 10:
      /* code */ if (Application_SmartRobotCarxxx0.Functional_Mode == TraceBased_mode)
      {
        TrackingDetection_S = 250;
      }
      break;
    case /* constant-expression */ 11:
      /* code */ if (Application_SmartRobotCarxxx0.Functional_Mode == TraceBased_mode)
      {
        if (TrackingDetection_S > 30)
        {
          TrackingDetection_S -= 10;
        }
      }
      break;
    case /* constant-expression */ 12:
    {
      if (Rocker_CarSpeed < 255)
      {
        Rocker_CarSpeed += 5;
      }
    }
    break;
    case /* constant-expression */ 13:
    {
      Rocker_CarSpeed = 250;
    }
    break;
    case /* constant-expression */ 14:
    {
      if (Rocker_CarSpeed > 50)
      {
        Rocker_CarSpeed -= 5;
      }
    }
    break;
    default:
      Application_SmartRobotCarxxx0.Functional_Mode = Standby_mode;
      break;
    }
    /*achieve time-limited control on movement direction part*/
    if (IRrecv_button < 5)
    {
      Application_SmartRobotCarxxx0.Functional_Mode = Rocker_mode;
      if (millis() - AppIRrecv.IR_PreMillis > 300)
      {
        IRrecv_en = false;
        Application_SmartRobotCarxxx0.Functional_Mode = Standby_mode;
        AppIRrecv.IR_PreMillis = millis();
      }
    }
    else
    {
      IRrecv_en = false;
      AppIRrecv.IR_PreMillis = millis();
    }
  }
}
