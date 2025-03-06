#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <arduino.h>

/*Motor*/
class Motor
{
public:
    void Motor_Init(void);
#if _Test_DeviceDriverSet
    void Motor_Test(void);
#endif
    void Motor_control(boolean direction_A, uint8_t speed_A, // A组电机参数
                       boolean direction_B, uint8_t speed_B, // B组电机参数
                       boolean controlED                     // AB使能允许 true
    );                                                       // 电机控制
    
private:
#define PIN_Motor_PWMA 5
#define PIN_Motor_PWMB 6
#define PIN_Motor_BIN_1 8
#define PIN_Motor_AIN_1 7
#define PIN_Motor_STBY 3

public:
#define speed_Max 255
#define direction_just true
#define direction_back false
#define direction_void 3

#define Duration_enable true
#define Duration_disable false
#define control_enable true
#define control_disable false
};

#endif