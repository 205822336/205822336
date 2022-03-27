/*读取土壤湿度传感器的模拟量*/
float vpp = 0.0048828125;     //5/1023  换算电压的系数

void setup() {
  Serial.begin(9600);
  
}

void loop() {
  int counts = analogRead(A0);  //A0接模拟采样口
  Serial.print("voltage：");Serial.print(counts*vpp);Serial.print("V");
  Serial.print(" analog：");Serial.println(counts);
  delay(2000);  
}
