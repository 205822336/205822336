// This is an example of implementation using ESP8266
// Never connect the sensor direct to the ESP8266, sensor high level is 5V
// ADC of ESP8266 high level is 3.3
// To connect use a voltage divisor, where 5V will 3v3 on the middle point like
// this {{URL}}

/*
  MQUnifiedsensor Library - reading an MQSensor using ESP8266 board
  For this example wi will demonstrates the use a MQ135 sensor.
  Library originally added 01 may 2019
  by Miguel A Califa, Yersson Carrillo, Ghiordy Contreras, Mario Rodriguez
 
  Added ESP8266 example 
  29.03.2020
  Wiring:
  https://github.com/miguel5612/MQSensorsLib_Docs/blob/master/static/img/MQ_ESP8266.PNG
   This example code is in the public domain.
   MQUnifiedsensor库-使用ESP8266板读取MQSensor
  本例中，wi将演示MQ135传感器的使用。
  图书馆最初于2019年5月1日增加
  作者：米格尔·A·加利福尼亚、耶尔森·卡里略、吉奥迪·孔特雷拉斯、马里奥·罗德里格斯
  添加了ESP8266示例
  29.03.2020
  装电线：
  https://github.com/miguel5612/MQSensorsLib_Docs/blob/master/static/img/MQ_ESP8266.PNG
  此示例代码位于公共域中。
*/

//Include the library
#include <MQUnifiedsensor.h>
/************************Hardware Related Macros************************************/
#define         Board                   ("ESP8266")
#define         Pin                     (A0)  //Analog input 3 of your arduino
/***********************Software Related Macros************************************/
#define         Type                    ("MQ-135") //MQ135
#define         Voltage_Resolution      (3.3) // 3V3 <- IMPORTANT
#define         ADC_Bit_Resolution      (10) // For ESP8266
#define         RatioMQ135CleanAir        (60)
/*****************************Globals***********************************************/
MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
/*****************************Globals***********************************************/

void setup() {
  //Init the serial port communication - to debug the library
  Serial.begin(9600); //Init serial port

  //Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(4.8387); MQ135.setB(-2.68); // Configure the equation to to calculate Benzene concentration
  /*
    Exponential regression:
  Gas    | a      | b
  LPG    | 44771  | -3.245
  CH4    | 2*10^31| 19.01
  CO     | 521853 | -3.821
  Alcohol| 0.3934 | -1.504
  Benzene| 4.8387 | -2.68
  Hexane | 7585.3 | -2.849

  指数回归：
气体| a | b
液化石油气| 44771 |-3.245
CH4 | 2*10^31 | 19.01
CO | 521853 |-3.821
酒精| 0.3934 |-1.504
苯| 4.8387 |-2.68
己烷| 7585.3 |-2.849
  */

  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ135.init(); 
 
  /* 
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ135.setRL(10);
     //如果RL值不同于10K，请使用以下方法分配RL值：
    MQ135。setRL（10）；
  */
  /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
   // In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
  // and on clean air (Calibration conditions), setting up R0 value.
  // We recomend executing this routine only on setup in laboratory conditions.
  // This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  //说明：
  //在该例行程序中，传感器将在预热前测量传感器的电阻
  //在清洁空气（校准条件）下，设置R0值。
  //我们建议只在实验室条件下进行设置。
  //此例行程序不需要在每次重启时执行，您可以从eeprom加载R0值。
  //致谢：https://jayconsystems.com/blog/understanding-a-gas-sensor

  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("警告：连接问题，R0无限大（检测到断路），请检查接线和电源"); while(1);}
  if(calcR0 == 0){Serial.println("警告：发现连接问题，R0为零（模拟引脚对地短路），请检查接线和电源"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  MQ135.serialDebug(true);
}

void loop() {
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin 更新数据时，arduino将从模拟引脚读取电压
  MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup 传感器将使用之前或从设置中设置的模型、a和b值读取PPM浓度
  MQ135.serialDebug(); // Will print the table on the serial port 将在串行端口上打印表格
  delay(500); //Sampling frequency 采样频率
}
