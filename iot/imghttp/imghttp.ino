#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_client.h"
#include <ArduinoJson.h>
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory

//wifi信息
const char* ssid = "mochen";
const char* password = "12345678";

/***************mochen.top***************/
//String serverName = "192.168.1.XXX";   // REPLACE WITH YOUR Raspberry Pi IP ADDRESS  替换为您的树莓派 IP 地址
String serverName = "img.mochen.top";   // OR REPLACE WITH YOUR DOMAIN NAME   或替换为您的域名
String serverPath = "/upload.php";     // The default serverPath should be upload.php  默认服务器路径应为上载.php
const int serverPort = 80;
WiFiClient client;
static esp_err_t take_send_photo();//推送函数
/*********************巴法云需要修改的地方**********************/
int capture_interval = 30*1000;      // 默认20秒上传一次，可更改（本项目是自动上传，如需条件触发上传，在需要上传的时候，调用take_send_photo()即可）
const char*  uid = "d9855e89bd49313921fa42387cbdac52";    //用户私钥，巴法云控制台获取
const char*  topic = "mypicture";     //主题名字，可在控制台新建
const char*  wechatMsg = "";          //如果不为空，会推送到微信，可随意修改，修改为自己需要发送的消息
const char*  wecomMsg = "";          //如果不为空，会推送到企业微信，推送到企业微信的消息，可随意修改，修改为自己需要发送的消息
const char*  urlPath = "";           //如果不为空，会生成自定义图片链接，自定义图片上传后返回的图片url，url前一部分为巴法云域名，第二部分：私钥+主题名的md5值，第三部分为设置的图片链接值。
String UID = "d9855e89bd49313921fa42387cbdac52";  //用户私钥，可在控制台获取,修改为自己的UID
String TOPIC = "img002";         //主题名字，可在控制台新建
const int LED_Pin = 4;              //单片机LED引脚值，D2是NodeMcu引脚命名方式，其他esp8266型号将D2改为自己的引脚
//巴法云服务器地址默认即可
#define TCP_SERVER_ADDR "bemfa.com"
//服务器端口，tcp创客云端口8344
#define TCP_SERVER_PORT "8344"
/********************************************************/
const char*  post_url = "http://images.bemfa.com/upload/v1/upimages.php"; // 默认上传地址
static String httpResponseString;//接收服务器返回信息
bool internet_connected = false;
long current_millis;
long last_capture_millis = 0;


//最大字节数
#define MAX_PACKETSIZE 512
//设置心跳值30s
#define KEEPALIVEATIME 30*1000
//相关函数初始化
//连接WIFI
void doWiFiTick();
void startSTA();
//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);
//led 控制函数
void turnOnLed();
void turnOffLed();

//tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
String TcpClient_Buff = "";
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;//心跳
unsigned long preTCPStartTick = 0;//连接
bool preTCPConnected = false;

// define the number of bytes you want to access 定义要访问的字节数
#define EEPROM_SIZE 60
// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const int timerInterval = 40000;    // 每个 HTTP POST 映像之间的时间
unsigned long previousMillis = 0;   // 上次发送图像的时间
const int timeinterval = 50000;    // 每次SD保存之间的时间
unsigned long pretime = 0;   //上次SD保存图像的时间
int pictureNumber = 0;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);
  
  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  
  WiFi.mode(WIFI_STA);
//  Serial.println();
//  Serial.print("Connecting to ");
//  Serial.println(ssid);
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) {
//    Serial.print(".");
    delay(500);
  }
//  Serial.println();
//  Serial.print("ESP32-CAM IP Address: ");
//  Serial.println(WiFi.localIP());

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers init 具有高规格以预分配更大的缓冲区
  if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality 0-63 数字越小，质量越高
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
//    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  pinMode(LED_Pin,OUTPUT);//初始化LED
  sendPhoto(); 
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= timerInterval) {
    sendPhoto();
    previousMillis = currentMillis;
  }//mochen.top
  
  current_millis = millis();
  if (current_millis - last_capture_millis > capture_interval) { // Take another picture
    last_capture_millis = millis();
    take_send_photo();
  }//巴法云

//  unsigned long cmillis = millis();
//  if (cmillis - pretime >= timeinterval) {
//    sdcome();
//    pretime = cmillis;
//  }//保存到SD卡
  
//  doWiFiTick();
//  doTCPClientTick();
}

String sendPhoto() //图片推送到mochen.top
{
  String getAll;
  String getBody;

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
//    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  
//  Serial.println("Connecting to server: " + serverName);

  if (client.connect(serverName.c_str(), serverPort)) {
//    Serial.println("Connection successful!");    
    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;
  
    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    client.println();
    client.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0; n<fbLen; n=n+1024) {
      if (n+1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client.write(fbBuf, remainder);
      }
    }   
    client.print(tail);
    
    esp_camera_fb_return(fb);
    
    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + timoutTimer) > millis()) {
//      Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length()==0) { state=true; }
          getAll = "";
        }
        else if (c != '\r') { getAll += String(c); }
        if (state==true) { getBody += String(c); }
        startTimer = millis();
      }
      if (getBody.length()>0) { break; }
    }
//    Serial.println();
    client.stop();
//    Serial.println(getBody);
  }
  else {
    getBody = "Connection to " + serverName +  " failed.";
//    Serial.println(getBody);
  }
  return getBody;
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
  if (evt->event_id == HTTP_EVENT_ON_DATA)
  {
    httpResponseString.concat((char *)evt->data);
  }
  return ESP_OK;
}


/********推送图片到巴法云*********/
static esp_err_t take_send_photo()
{
//  Serial.println("Taking picture...");
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;

  fb = esp_camera_fb_get();
  if (!fb) {
//    Serial.println("Camera capture failed");
    return ESP_FAIL;
  }

  httpResponseString = "";
  esp_http_client_handle_t http_client;
  esp_http_client_config_t config_client = {0};
  config_client.url = post_url;
  config_client.event_handler = _http_event_handler;
  config_client.method = HTTP_METHOD_POST;
  http_client = esp_http_client_init(&config_client);
  esp_http_client_set_post_field(http_client, (const char *)fb->buf, fb->len);//设置http发送的内容和长度
  esp_http_client_set_header(http_client, "Content-Type", "image/jpg"); //设置http头部字段
  esp_http_client_set_header(http_client, "Authorization", uid);        //设置http头部字段
  esp_http_client_set_header(http_client, "Authtopic", topic);          //设置http头部字段
  esp_http_client_set_header(http_client, "wechatmsg", wechatMsg);      //设置http头部字段
  esp_http_client_set_header(http_client, "wecommsg", wecomMsg);        //设置http头部字段
  esp_http_client_set_header(http_client, "picpath", urlPath);          //设置http头部字段
  esp_err_t err = esp_http_client_perform(http_client);//发送http请求
  if (err == ESP_OK) {
    //json数据解析
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, httpResponseString);
    if (error) {
//      Serial.print(F("deserializeJson() failed: "));
//      Serial.println(error.c_str());
    }
    String url = doc["url"];
//    Serial.println(url);//打印获取的URL
  }
  esp_http_client_cleanup(http_client);
  esp_camera_fb_return(fb);
}

void sdcome()//保存到SD卡
{

    if(!SD_MMC.begin()){//初始化SD卡
    Serial.println("SD Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
  pinMode(LED_Pin,OUTPUT);//初始化LED
  
  camera_fb_t * fb = NULL;
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

  // Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) +".jpg";

  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path.c_str());
  
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();
  esp_camera_fb_return(fb); 
//  rtc_gpio_hold_en(GPIO_NUM_4);
  delay(2000);
//  Serial.println("Going to sleep now");
//  delay(2000);
//  esp_deep_sleep_start();
//  Serial.println("This will never be printed");
}

/*
  *发送数据到TCP服务器
 */
//void sendtoTCPServer(String p){
//  
//  if (!TCPclient.connected()) 
//  {
//    Serial.println("Client is not readly");
//    return;
//  }
//  TCPclient.print(p);
//  Serial.println("[Send to TCPServer]:String");
//  Serial.println(p);
//}
//
//
///*
//  *初始化和服务器建立连接
//*/
//void startTCPClient(){
//  if(TCPclient.connect(TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT))){
//    Serial.print("\nConnected to server:");
//    Serial.printf("%s:%d\r\n",TCP_SERVER_ADDR,atoi(TCP_SERVER_PORT));
//    
//    String tcpTemp="";  //初始化字符串
//    tcpTemp = "cmd=1&uid="+UID+"&topic="+TOPIC+"\r\n"; //构建订阅指令
//    sendtoTCPServer(tcpTemp); //发送订阅指令
//    tcpTemp="";//清空
//    
//    preTCPConnected = true;
//    preHeartTick = millis();
//    TCPclient.setNoDelay(true);
//  }
//  else{
//    Serial.print("Failed connected to server:");
//    Serial.println(TCP_SERVER_ADDR);
//    TCPclient.stop();
//    preTCPConnected = false;
//  }
//  preTCPStartTick = millis();
//}


///*
//  *检查数据，发送心跳
//*/
//void doTCPClientTick(){
// //检查是否断开，断开后重连
//   if(WiFi.status() != WL_CONNECTED) return;
//
//
//  if (!TCPclient.connected()) {//断开重连
//  if(preTCPConnected == true){
//
//    preTCPConnected = false;
//    preTCPStartTick = millis();
//    Serial.println();
//    Serial.println("TCP Client disconnected.");
//    TCPclient.stop();
//  }
//  else if(millis() - preTCPStartTick > 1*1000)//重新连接
//    startTCPClient();
//  }
//  else
//  {
//    if (TCPclient.available()) {//收数据
//      char c =TCPclient.read();
//      TcpClient_Buff +=c;
//      TcpClient_BuffIndex++;
//      TcpClient_preTick = millis();
//      
//      if(TcpClient_BuffIndex>=MAX_PACKETSIZE - 1){
//        TcpClient_BuffIndex = MAX_PACKETSIZE-2;
//        TcpClient_preTick = TcpClient_preTick - 200;
//      }
//      preHeartTick = millis();
//    }
//    if(millis() - preHeartTick >= KEEPALIVEATIME){//保持心跳
//      preHeartTick = millis();
//      Serial.println("--Keep alive:");
//      sendtoTCPServer("cmd=0&msg=keep\r\n");
//    }
//  }
//  if((TcpClient_Buff.length() >= 1) && (millis() - TcpClient_preTick>=200))
//  {//data ready
//    TCPclient.flush();
//    Serial.println("Buff");
//    Serial.println(TcpClient_Buff);
//    if((TcpClient_Buff.indexOf("&msg=on") > 0)) {
//      turnOnLed();
//    }else if((TcpClient_Buff.indexOf("&msg=off") > 0)) {
//      turnOffLed();
//    }
//   TcpClient_Buff="";
//   TcpClient_BuffIndex = 0;
//  }
//}
//
//void startSTA(){
//  WiFi.disconnect();
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(ssid, password); 
//}
//
//
//
//  WiFiTick
//  检查是否需要初始化WiFi
//  检查WiFi是否连接上，若连接成功启动TCP Client
//  控制指示灯
//*/
//void doWiFiTick(){
//  static bool startSTAFlag = false;
//  static bool taskStarted = false;
//  static uint32_t lastWiFiCheckTick = 0;
//
//  if (!startSTAFlag) {
//    startSTAFlag = true;
//    startSTA();
//    Serial.printf("Heap size:%d\r\n", ESP.getFreeHeap());
//  }
//
//  //未连接1s重连
//  if ( WiFi.status() != WL_CONNECTED ) {
//    if (millis() - lastWiFiCheckTick > 1000) {
//      lastWiFiCheckTick = millis();
//    }
//  }
//  //连接成功建立
//  else {
//    if (taskStarted == false) {
//      taskStarted = true;
//      Serial.print("\r\nGet IP Address: ");
//      Serial.println(WiFi.localIP());
//      startTCPClient();
//    }
//  }
//}

//打开灯泡
void turnOnLed(){
  Serial.println("Turn ON");
  analogWrite(LED_Pin, 10);
}
//关闭灯泡
void turnOffLed(){
  Serial.println("Turn OFF");
    analogWrite(LED_Pin, 10);
}
