//LingShun Lab
 
int input1 = D2; // 定义uno的pin 5 向 input1 输出 
int input2 = D3; // 定义uno的pin 6 向 input2 输出
 
 
void setup() {
//  Serial.begin (9600);
//初始化各IO,模式为OUTPUT 输出模式
pinMode(input1,OUTPUT);
pinMode(input2,OUTPUT);
}
 
void loop() {
  //forward 向前转
  digitalWrite(input1,HIGH); //给高电平
  digitalWrite(input2,LOW);  //给低电平
  delay(1000);   //延时1秒
 
 //stop 停止
 digitalWrite(input1,LOW);
 digitalWrite(input2,LOW);  
 delay(500);  //延时0.5秒
}
