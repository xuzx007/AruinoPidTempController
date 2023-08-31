#include "max6675.h"
#include <Wire.h>
#include <PIDController.h>
#include <EEPROM.h>

PIDController pid; //缩写

String comdata = "";//用来存储输入的字符

int soPin = 4; // SO=Serial Out
int csPin = 5; // CS = chip select pin
int sckPin = 6; // SCK = Serial clock pin
MAX6675 thermocouple(sckPin, csPin, soPin);

#define pwmPin 3 // 设定pwm output针脚

float temperature_read = 0.0; //用来存储感器温度
float set_temperature = 30; //设定截至温度
float e_temp = 1.1; //中转eeprom温度
int EEaddress = 2; //eeprom地址

void setup() {
  Serial.begin(9600); //建立串口通讯bit per second (baud rates)
  pinMode(pwmPin, OUTPUT); //设置pin 3为输出
  TCCR2B = TCCR2B & B11111000 | 0x03; // pin 3 and 11 PWM frequency of 980.39 Hz
  pid.begin(); //开始PID，并设置一些参数
  pid.setpoint(150);
  pid.tune(30, 0.7, 200);
  pid.limit(0,255); //限定pid区间
}

void loop() {
  while (Serial.available() > 0) { //用循环语句把串口输入逐一字节存到comdata
    comdata += char(Serial.read());
    delay(2);
  }
  
  if (comdata.length() > 0){ //如果comdata不为空就运行
    set_temperature = comdata.toFloat(); //将comdata里的字符串转为浮点数，并为设定温度
    comdata = ""; //清空comdata
    EEPROM.put(EEaddress, set_temperature); //把设定温度以浮点数的方式保存到eeprom里
    Serial.println("new temperature will be set."); //告诉用户，温度设置成功
  }

  EEPROM.get(EEaddress, e_temp); //读取eeprom
  set_temperature = e_temp; //使用eeprom里的数为设定温度，掉电后这个温度也不会变
  
  temperature_read = thermocouple.readCelsius(); //读取传感器温度，并存起来
  int output = pid.compute(temperature_read); //pid方法计算pwm输出
  pid.setpoint(set_temperature);
  analogWrite(pwmPin, output); //输出pwm值
  Serial.print("set temperature: ");
  Serial.println(set_temperature); // wanted temperature
  Serial.print("eeprom: ");
  Serial.println(e_temp);
  Serial.print("current temp.: ");
  Serial.println(thermocouple.readCelsius(), 1); //curent temperature 
  Serial.print("pWM output: ");
  Serial.println(output); // pwm output
  Serial.print("\n\n\n");
  delay(1998);
}


// Arduino EEPROM读写实例 https://www.songbingjia.com/nginx/show-178599.html
// Temperature PID controller - Arduino http://electronoobs.com/eng_arduino_tut24.php
// PIDArduino https://github.com/DonnyCraft1/PIDArduino
//  Arduino PID Temperature Controller using MAX6675 K-Thermocouple to Digital Converter IC https://circuitdigest.com/microcontroller-projects/arduino-pid-temperature-controller#:~:text=As%20the%20name%20suggests%20a,the%20current%20temperature%20and%20setpoint
//Arduino学习笔记A6 - Arduino串口接收字符串 https://www.geek-workshop.com/thread-158-1-1.html
//Interfacing MAX6675 Thermocouple Module with Arduino https://electropeak.com/learn/interfacing-max6675-k-type-thermocouple-module-with-arduino/
