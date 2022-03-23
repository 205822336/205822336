#include <SPI.h>
#include <U8g2lib.h>
#include <SimpleDHT.h>
int pinDHT11 = D5;
SimpleDHT11 dht11(pinDHT11);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
 void initdisplay()
{
    u8g2.begin();
    u8g2.enableUTF8Print();
}

const unsigned char du[] U8X8_PROGMEM={
0x80,0x00,0x00,0x01,0xFC,0x7F,0x44,0x04,0x44,0x04,0xFC,0x3F,0x44,0x04,0x44,0x04,
0xC4,0x07,0x04,0x00,0xF4,0x0F,0x24,0x08,0x42,0x04,0x82,0x03,0x61,0x0C,0x1C,0x70,/*度*/
};

const unsigned char wen[] U8X8_PROGMEM = {0x00,0x00,0xC4,0x1F,0x48,0x10,0x48,0x10,0xC1,0x1F,0x42,0x10,0x42,0x10,0xC8,0x1F,
0x08,0x00,0xE4,0x3F,0x27,0x25,0x24,0x25,0x24,0x25,0x24,0x25,0xF4,0x7F,0x00,0x00};

const unsigned char shi[] U8X8_PROGMEM = {0x00,0x00,0xE4,0x1F,0x28,0x10,0x28,0x10,0xE1,0x1F,0x22,0x10,0x22,0x10,0xE8,0x1F,0x88,0x04,0x84,0x04,0x97,0x24,0xA4,0x14,0xC4,0x0C,0x84,0x04,0xF4,0x7F,0x00,0x00};

void setup() {
  Serial.begin(9600);
  initdisplay();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.setCursor(16, 14);
  u8g2.print("实时");
  u8g2.drawXBM(16*3, 0, 16, 16, wen);
  u8g2.drawXBM(16*4, 0, 16, 16, shi);
  u8g2.drawXBM(16*5, 0, 16, 16, du);
  u8g2.sendBuffer();

}


void loop() {
//   read without samples.
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
    return;
  }
  
  Serial.print("Sample OK: ");
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.println(" H");
  u8g2.drawXBM(0, 16, 16, 16, wen);
  u8g2.drawXBM(0, 16*2, 16, 16, shi);
  u8g2.drawXBM(16, 16*2, 16, 16, du);
  u8g2.drawXBM(16, 16, 16, 16, du);
  u8g2.setCursor(16*3, 16*2);
  u8g2.print((int)temperature);
  u8g2.setCursor(16*3, 16*3);
  u8g2.print((int)humidity);
  u8g2.sendBuffer();
  delay(3000);
//  u8g2.drawXBM(111, 49, 16, 16, liu);
//  u8g2.drawXBM(111, 49, 16, 16, liu);

}
