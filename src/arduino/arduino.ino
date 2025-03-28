/*
 * @Author: ELEGOO
 * @Date: 2019-10-22 11:59:09
 * @LastEditTime: 2020-06-28 14:55:26
 * @LastEditors: Changhua
 * @Description: SmartRobot robot tank
 * @FilePath:
 */
#include <avr/wdt.h>
#include "DeviceDriverSet_xxx0.h"
#include "ApplicationFunctionSet_xxx0.h"

void setup()
{
  wdt_disable();
  Application_FunctionSet.ApplicationFunctionSet_Init();
}

void loop()
{
  delay(10);
  Application_FunctionSet.ApplicationFunctionSet_Tracking();
  Application_FunctionSet.ApplicationFunctionSet_SensorDataUpdate();
}
