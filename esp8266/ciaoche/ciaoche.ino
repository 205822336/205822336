#include <ESP8266WiFi.h>
#define server_ip "bemfa.com" //巴法云服务器地址默认即可
#define server_port "8344" //服务器端口，tcp创客云端口8344
#include <Servo.h>
Servo myservo;  // 定义Servo对象来控制
int pospin = D3;
int in1=D5,in2=D6;
//********************需要修改的部分*******************//

#define wifi_name  "mochen"     //WIFI名称，区分大小写，不要写错
#define wifi_password   "12345678"  //WIFI密码
String UID = "d9855e89bd49313921fa42387cbdac52";  //用户私钥，可在控制台获取,修改为自己的UID
String TOPIC = "che";         //主题名字，可在控制台新建

//**************************************************//
//最大字节数
#define MAX_PACKETSIZE 512
//设置心跳值30s
#define KEEPALIVEATIME 5*1000
//tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
String TcpClient_Buff = "";//初始化字符串，用于接收服务器发来的数据
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;//心跳
unsigned long preTCPStartTick = 0;//连接
bool preTCPConnected = false;
//相关函数初始化
//连接WIFI
void doWiFiTick();
void startSTA();

//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);


/*
  *发送数据到TCP服务器
 */
void sendtoTCPServer(String p){
  if (!TCPclient.connected()) 
  {
    Serial.println("Client is not readly");
    return;
  }
  TCPclient.print(p);
}


/*
  *初始化和服务器建立连接
*/
void startTCPClient(){
  if(TCPclient.connect(server_ip, atoi(server_port))){
    Serial.print("\nConnected to server:");
    Serial.printf("%s:%d\r\n",server_ip,atoi(server_port));
    
    String tcpTemp="";  //初始化字符串
    tcpTemp = "cmd=1&uid="+UID+"&topic="+TOPIC+"\r\n"; //构建订阅指令
    sendtoTCPServer(tcpTemp); //发送订阅指令
    tcpTemp="";//清空
    /*
     //如果需要订阅多个主题，可再次发送订阅指令
      tcpTemp = "cmd=1&uid="+UID+"&topic="+主题2+"\r\n"; //构建订阅指令
      sendtoTCPServer(tcpTemp); //发送订阅指令
      tcpTemp="";//清空
     */
    
    preTCPConnected = true;
    preHeartTick = millis();
    TCPclient.setNoDelay(true);
  }
  else{
    Serial.print("Failed connected to server:");
    Serial.println(server_ip);
    TCPclient.stop();
    preTCPConnected = false;
  }
  preTCPStartTick = millis();
}

int len,shu,xiang;
bool f=0;
void chuli(String Msg)//处理接收到的数据
{
     f=0;
   shu=0;
   xiang=0;
    for(int i=0;i<len;i++)
    {
      if((Msg[i]>='0'&&Msg[i]<='9')&&f==0)
      {
        shu=(Msg[i]-'0')+shu*10;
      }
      if(Msg[i]=='#')f=1;
      if((Msg[i]>='0'&&Msg[i]<='9')&&f==1)
      {
        xiang=(Msg[i]-'0')+xiang*10;
      }
    }
    Serial.print("车速");Serial.println(shu);   //打印车速
    Serial.print("方向");Serial.println(xiang);   //打印方向
}
int spe=50;
void chesu()
{
    if(shu==50)//停车
    {
      analogWrite(in1, LOW);
      digitalWrite(in2,LOW);
      Serial.println("停车");
    }
    else if(shu>50)//前进
    {
      Serial.print("shu:");Serial.println(shu);
      spe=map(shu-50,0,50,0,255);//x =map(value, fromLow, fromHigh, toLow, toHigh);
      analogWrite(in1, spe);
      digitalWrite(in2,LOW);
      Serial.print("spe:");Serial.println(spe);
//      analogWrite(in1, spe);
    }
    else if(shu<50)//后退
    {
      digitalWrite(in1,LOW);
      analogWrite(in2, spe);
      spe=map(shu,0,50,0,255);
      Serial.print("spe:");Serial.println(spe);
    }
    else
    {
//      Serial.print("shu:");Serial.println(shu);
//      spe=map(shu-50,0,50,0,255);//x =map(value, fromLow, fromHigh, toLow, toHigh);
      analogWrite(in1, LOW);
      digitalWrite(in2,LOW);
      Serial.println("停车");
//      Serial.println(spe);
//      analogWrite(in1, spe);
    }
}
/*
  *检查数据，发送心跳
*/
void doTCPClientTick(){
  len=0;
 //检查是否断开，断开后重连
  if(WiFi.status() != WL_CONNECTED) return;
  if (!TCPclient.connected()) {//断开重连
  if(preTCPConnected == true){
    preTCPConnected = false;
    preTCPStartTick = millis();
    Serial.println();
    Serial.println("TCP Client disconnected.");
    TCPclient.stop();
  }
  else if(millis() - preTCPStartTick > 1*1000)//重新连接
       digitalWrite(in1, LOW);//停车
      digitalWrite(in2,LOW);
    startTCPClient();
  }
  else
  {
    if (TCPclient.available()) {//收数据
      char c =TCPclient.read();
      TcpClient_Buff +=c;
      TcpClient_BuffIndex++;
      TcpClient_preTick = millis();
      
      if(TcpClient_BuffIndex>=MAX_PACKETSIZE - 1){
        TcpClient_BuffIndex = MAX_PACKETSIZE-2;
        TcpClient_preTick = TcpClient_preTick - 200;
      }
      preHeartTick = millis();
    }
    if(millis() - preHeartTick >= KEEPALIVEATIME){//保持心跳
      preHeartTick = millis();
      Serial.println("--Keep alive:");
      sendtoTCPServer("ping\r\n"); //发送心跳，指令需\r\n结尾，详见接入文档介绍
    }
  }
  if((TcpClient_Buff.length() >= 1) && (millis() - TcpClient_preTick>=200))
  {
    TCPclient.flush();
    Serial.print("Rev string: ");
    TcpClient_Buff.trim(); //去掉首位空格
    Serial.println(TcpClient_Buff); //打印接收到的消息
    String getTopic = "";
    String getMsg = "";
    if(TcpClient_Buff.length() > 15){//注意TcpClient_Buff只是个字符串，在上面开头做了初始化 String TcpClient_Buff = "";
          //此时会收到推送的指令，指令大概为 cmd=2&uid=xxx&topic=light002&msg=off
//          int topicIndex = TcpClient_Buff.indexOf("&topic=")+7; //c语言字符串查找，查找&topic=位置，并移动7位，不懂的可百度c语言字符串查找
          int msgIndex = TcpClient_Buff.indexOf("&msg=");//c语言字符串查找，查找&msg=位置
//          getTopic = TcpClient_Buff.substring(topicIndex,msgIndex);//c语言字符串截取，截取到topic,不懂的可百度c语言字符串截取
          getMsg = TcpClient_Buff.substring(msgIndex+5);//c语言字符串截取，截取到消息
//          Serial.print("topic:------");
//          Serial.println(getTopic); //打印截取到的主题值
          Serial.print("msg:--------");
          Serial.println(getMsg);   //打印截取到的消息值
   }
   len=getMsg.length();
   if(len>=3)
   {
    chuli(getMsg);
    myservo.write(xiang);
    Serial.println(xiang);
   chesu();
   Serial.println(shu);
   }
   
    
   TcpClient_Buff="";
   TcpClient_BuffIndex = 0;
  }
}
/*
  *初始化wifi连接
*/
void startSTA(){
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_name, wifi_password);
}



/**************************************************************************
                                 WIFI
***************************************************************************/
/*
  WiFiTick
  检查是否需要初始化WiFi
  检查WiFi是否连接上，若连接成功启动TCP Client
  控制指示灯
*/
void doWiFiTick(){
  static bool startSTAFlag = false;
  static bool taskStarted = false;
  static uint32_t lastWiFiCheckTick = 0;

  if (!startSTAFlag) {
    startSTAFlag = true;
    startSTA();
  }

  //未连接1s重连
  if ( WiFi.status() != WL_CONNECTED ) {
    if (millis() - lastWiFiCheckTick > 1000) {
      lastWiFiCheckTick = millis();
    }
  }
  //连接成功建立
  else {
    if (taskStarted == false) {
      taskStarted = true;
      Serial.print("\r\nGet IP Address: ");
      Serial.println(WiFi.localIP());
      startTCPClient();
    }
  }
}


// 初始化，相当于main 函数
void setup() {
  Serial.begin(9600);
  myservo.attach(pospin);  // 控制线连接数字9
  myservo.write(xiang);  
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);
  Serial.println("Beginning...");
}

//循环
void loop() {
  doWiFiTick();
  doTCPClientTick();
}
