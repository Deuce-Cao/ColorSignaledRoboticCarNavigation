/*
 * @Author: ELEGOO
 * @Date: 2019-10-22 11:59:09
 * @LastEditTime: 2020-12-29 16:04:05
 * @LastEditors: Changhua
 * @Description: Smart Robot Car V4.0
 * @FilePath: 
 */
#ifndef _ApplicationFunctionSet_xxx0_H_
#define _ApplicationFunctionSet_xxx0_H_

#include <Arduino.h>

class ApplicationFunctionSet
{
public:
  void ApplicationFunctionSet_Init(void);
  void ApplicationFunctionSet_Bootup(void);
  void ApplicationFunctionSet_RGB(void);
  void ApplicationFunctionSet_Expression(void);
  void ApplicationFunctionSet_Tracking(void);           //Line Tracking Mode
  void ApplicationFunctionSet_Servo(uint8_t Set_Servo); //Servo Control
  void ApplicationFunctionSet_Rocker(void);             //Rocker Control Mode
  void ApplicationFunctionSet_Standby(void);            //Standby Mode
  void ApplicationFunctionSet_KeyCommand(void);         //Mode Switch Button
  void ApplicationFunctionSet_SensorDataUpdate(void);   //Sensor Data Update
  void PerformSequence(void); //Perform Sequence
  void ApplicationFunctionSet_IRrecv(void);
  void turnAround(void); // Test function for turning around

private:
  /*Sensor Raw Value*/
  volatile float VoltageData_V;        //Battery Voltage Value
  volatile uint16_t UltrasoundData_mm; //Ultrasonic Sensor Value (mm)
  volatile uint16_t UltrasoundData_cm; //Ultrasonic Sensor Value (cm)
  volatile int TrackingData_L;         //Line Tracking Module Value (Left)
  volatile int TrackingData_M;         //Line Tracking Module Value (Middle)
  volatile int TrackingData_R;         //Line Tracking Module Value (Right)
  /*Sensor Status*/
  boolean VoltageDetectionStatus = false;
  boolean UltrasoundDetectionStatus = false;
  boolean TrackingDetectionStatus_R = false;
  boolean TrackingDetectionStatus_M = false;
  boolean TrackingDetectionStatus_L = false;

public:
  boolean Car_LeaveTheGround = true;

  /*Sensor Threshold Setting*/
  const float VoltageDetection = 7.00;
  uint8_t Rocker_CarSpeed = 250;
  uint8_t Rocker_temp;

public:
  uint8_t TrackingDetection_S = 250;
  uint16_t TrackingDetection_E = 850;
  uint16_t TrackingDetection_V = 950;
};
extern ApplicationFunctionSet Application_FunctionSet;
#endif
