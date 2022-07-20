#include <Arduino.h>
const String ver = "1.0.8";
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1327_VISIONOX_128X96_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 14, /* data=*/ 5, /* cs=*/ 15, /* dc=*/ 16, /* reset=*/ 17);
//U8G2_SSD1327_VISIONOX_128X96_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 15, /* dc=*/ 16, /* reset=*/ 17);
#define fan_x 16
#define fan_y 16
static const unsigned char fan[] U8X8_PROGMEM = {
  0xC0, 0x03, 0xC0, 0x07, 0x80, 0x23, 0x8E, 0x63, 0x9E, 0x71, 0xFF, 0xFB, 0x78, 0x7E, 0x30, 0x0C, 0x30, 0x0C, 0x7E, 0x1E, 0xDF, 0xFF, 0x8E, 0x79, 0xC6, 0x71, 0xC0, 0x01, 0xE0, 0x03, 0xC0, 0x03
};
#define fan1_x 16
#define fan1_y 16
static const unsigned char fan1[] U8X8_PROGMEM = {
  0x20, 0x04, 0x78, 0x1C, 0x78, 0x1C, 0x70, 0x1E, 0xE0, 0x0F, 0xC2, 0x07, 0x6F, 0xC6, 0x3F, 0xFC, 0x3F, 0xFC, 0x63, 0xF6, 0xE0, 0x43, 0xF0, 0x07, 0x78, 0x0E, 0x38, 0x3E, 0x38, 0x1E, 0x20, 0x04
};
#define fan2_x 16
#define fan2_y 16
static const unsigned char fan2[] U8X8_PROGMEM = {
  0xC0, 0x03, 0xC0, 0x07, 0x80, 0x03, 0x8E, 0x63, 0x9E, 0x71, 0xFF, 0xFB, 0x78, 0x7E, 0x30, 0x0C, 0x30, 0x0C, 0x7E, 0x1E, 0xDF, 0xFF, 0x8E, 0x79, 0xC6, 0x71, 0xC4, 0x01, 0xE0, 0x03, 0xC0, 0x03
};
#define fan3_x 16
#define fan3_y 16
static const unsigned char fan3[] U8X8_PROGMEM = {
  0x20, 0x04, 0x78, 0x1C, 0x7C, 0x1C, 0x70, 0x1E, 0xE0, 0x0F, 0xC2, 0x07, 0x6F, 0xC6, 0x3F, 0xFC, 0x3F, 0xFC, 0x63, 0xF6, 0xE0, 0x43, 0xF0, 0x07, 0x78, 0x0E, 0x38, 0x1E, 0x38, 0x1E, 0x20, 0x04
};

#define xline_x 103
#define xline_y 4
static const unsigned char xline[] U8X8_PROGMEM = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x02, 0x08, 0x20, 0x80, 0x00, 0x02, 0x08, 0x20, 0x80, 0x00, 0x02, 0x88, 0x00, 0x02, 0x08, 0x20, 0x80, 0x00,
  0x02, 0x08, 0x20, 0x80, 0x00, 0x02, 0x88, 0x00, 0x02, 0x08, 0x20, 0x80, 0x00, 0x02, 0x08, 0x20, 0x80, 0x00, 0x02, 0x88
};

#define yline_x 4
#define yline_y 49
static const unsigned char yline[] U8X8_PROGMEM = {
  0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xFF, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xFF, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xFF, 0xF1, 0xF1,
  0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xFF, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1
};

const byte green = 23;
const byte org = 22;
const byte red = 19;
const byte top = 27;

const byte computer_fg = 26; //主板fg
const byte computer_pwm = 25; //主板pwm
const byte computer_12v = 34;
const byte computer_power_on = 13;//电脑开机继电器
const byte fan_fg = 35; //
const byte screen_pwm = 21; //
const byte bl_pwm = 18; //

String ssid = ""; //wifi名只能用2.4G信号
String pswd = ""; //wifi密码
const char* html_template = "<!DOCTYPE html><html><head>[[meta]]<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>.start{background-color: #4078f2;} .stop{background-color:#b22222;} #btn_scan{margin: 20px 0;padding: 10px;border-radius: 5px;text-align: center;color: #fff;} #result{border:2px solid #4078f2;border-radius: 5px;padding: 5px;height: 300px;} input[type='submit']{width: 100%;padding: 10px;font-size: 20px;background-color: #4078f2;border: 0;border-radius: 5px;color: #fff;letter-spacing: 4px;}input[type='text']{width: calc(100% - 14px);padding: 5px;margin: 0;font-size: 16px;border: 2px solid;border-radius: 5px;color: #df5000;margin: 4px 0px;} input[type='text']:valid{color:#4078f2;} body{margin: 20px;} h2{margin: 50px 0 0 0;} h3{font-size: 0.5em;font-weight: normal;margin: 5px 0 0 0;} p{margin:10px 0;}</style><title>[[title]]</title></head><body>[[body]]</body></html>";
const char* html_body_wificonfig = "<h1>设置</h1><form name='input' action='/' method='POST'><p>WIFI SSID 2.4G:<br><input type='text' name='ssid' value='[[ssid]]'></p><p>PASSWORD:<br><input type='text' name='pswd' value='[[pswd]]'></p><p><input type='submit' value='保存设置'></p><p><a href='/update'>固件更新</a></p></form>";
const char* serverIndex =
  "<script src='https://apps.bdimg.com/libs/jquery/2.1.4/jquery.min.js'></script><p><a href='/'>返回</a></p>"
  "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'><p>上传BIN文件：<input type='file' name='update'></p><p><input type='submit' value='开始更新'></p></form>"
  "<p id='prg'>progress: 0%</p>"
  "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "if(per<1){$('#prg').html('上传进度: ' + Math.round(per*100) + '%');}else{$('#prg').html('上传成功，正在更新，大约需要2分钟......');}"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "$('#prg').html('更新完成！');"
  "},"
  "error: function (a, b, c) {"
  "}"
  "});"
  "});"
  "</script>";

bool computer_is_on = 0; //保存电脑启动状态
String computer_is_on_s = "off";
unsigned int fgcount = 0;
unsigned int pwmcount = 0;

unsigned int computer_pwm_value = 10;
unsigned int computer_pwm_min = 0; //设定最低转速信号频率
unsigned int computer_pwm_max = 1000; //设定最高转速信号频率

unsigned int fan_pwm_min = 174; //设定最低转速信号频率
unsigned int fan_pwm_max = 850; //设定最高转速信号频率
unsigned int fan_pwm_current = fan_pwm_min;

unsigned int fan_fpm = 0; //保存计算出的风扇转速
unsigned long t1 = 0;
String s_fan_fpm = "";
int get_temperature = 0;

int fanf = 0;
String line1 = "";
String line2 = "";
int istr = 10;
unsigned long t = 0;
String ss = "";
String intchars = "";
String chars = "";
int points[99];

int set_temperature_fan[6] = { -100, -100, 0, 100, 20, 20};
String tempstr = "";
int set_temperature_fan_pos = 0;
bool fan_pwm_i = 0;
bool xiaomi_fg_i = 0;
String screen_output[30];
int screen_output_set = 0;
int screen_output_get = 0;
String screenmsg = "";
void set_screen_output(String s)
{
  screen_output[screen_output_set] = s;
  screen_output_set += 1;
  if (screen_output_set == 30)
  {
    screen_output_set = 0;
  }
}
int computer_pwm_i = 0;
int computer_off_count = 0;
int computer_on_count = 0;
String scmd = "";
String pcmd = "";

WebServer server(80);
unsigned long long1 = (unsigned long)((ESP.getEfuseMac() & 0xFFFF0000) >> 16);
unsigned long long2 = (unsigned long)((ESP.getEfuseMac() & 0x0000FFFF));
String espid = String(long1, HEX) + String(long2, HEX);
String AP_NAME = "3014DIY_" + espid;
IPAddress apIP(192, 168, 4, 1);//esp8266-AP-IP地址

int oState = -1;
int fanfg = 0;
const byte fan_pwm = 32;

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
void readconfig() {
  File f = SPIFFS.open("/config.txt", FILE_READ);
  if (f) {
    String s_config = f.readString();
    //Serial.print("readconfig ");
    //Serial.println(s_config);
    ssid = getValue(s_config, '|', 0);
    pswd = getValue(s_config, '|', 1);
    f.close();
  }
}
void saveconfig() {
  detachInterrupt(digitalPinToInterrupt(fan_fg));
  detachInterrupt(digitalPinToInterrupt(computer_pwm));
  File dataFile = SPIFFS.open("/config.txt", FILE_WRITE);
  String s_config = ssid + "|" + pswd;
  //Serial.print("saveconfig ");
  //Serial.println(s_config);
  dataFile.print(s_config);
  dataFile.close();
  delay(1000);
  ESP.restart();
}
int fgcount_t = 0;
int poweron_time = 0;
String cmd = "";
String val = "";


void initWebServer(void) {
  server.on("/", HTTP_GET, []() {
    String html;
    html += html_template;
    html.replace("[[title]]", "WIFI CONFIG");
    html.replace("[[meta]]", "");
    html.replace("[[body]]", html_body_wificonfig);
    html.replace("[[ssid]]", ssid);
    html.replace("[[pswd]]", "");
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", html);
  });
  server.on("/", HTTP_POST, []() {
    ssid = server.arg("ssid");
    pswd = server.arg("pswd");
    String html;
    html += html_template;
    html.replace("[[title]]", "WIFI CONFIG");
    html.replace("[[meta]]", "");
    html.replace("[[body]]", "<h1>保存成功</h1><a href='/'>返回</a>");
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", html); //返回保存成功页面
    delay(500);
    saveconfig();
  });
  server.on("/update", HTTP_GET, []() {
    String html;
    html += html_template;
    html.replace("[[title]]", "固件更新");
    html.replace("[[meta]]", "");
    html.replace("[[body]]", serverIndex);
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", html);
  });
  server.on("/update", HTTP_POST, []() {
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      //Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        //Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}
void initSoftAP(void) {
  //Serial.println("initSoftAP");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (WiFi.softAP(AP_NAME.c_str())) {
  }
}
long lastT = 0;
long upT = 0;
long downT = 0;
int cpwm = 0;
int chg = 0;
ICACHE_RAM_ATTR void computerpwm() {
  if (lastT > 0) {
    if (digitalRead(computer_pwm)) {
      downT += micros() - lastT;
    } else {
      upT += micros() - lastT;
    }
  }
  chg += 1;
  if (chg > 1000) {
    cpwm = 1000 * upT / (downT + upT);
    upT = 0;
    downT = 0;
    chg = 0;
  }
  lastT = micros();
}
int outfg = 0;
bool cfg = 0;
ICACHE_RAM_ATTR void fanfgcout() {
  fgcount += 1;
  outfg += 1;
  if (outfg == 6) {
    digitalWrite(computer_fg, cfg);
    cfg = cfg ^ 1;
    outfg = 0;
  }
}
String ip = "";
void setup(void) {
  pinMode(computer_pwm, INPUT_PULLUP);
  pinMode(computer_12v, INPUT);
  pinMode(computer_power_on, OUTPUT);
  digitalWrite(computer_power_on, LOW);
  pinMode(computer_fg, OUTPUT);
  pinMode(screen_pwm, OUTPUT);
  digitalWrite(screen_pwm, 0);
  pinMode(green, OUTPUT);
  //pinMode(green, INPUT);
  pinMode(red, OUTPUT);
  pinMode(org, OUTPUT);
  pinMode(top, OUTPUT);
  pinMode(bl_pwm, OUTPUT);
  //digitalWrite(bl_pwm, 1);
  digitalWrite(green, 0);
  digitalWrite(red, 0);
  digitalWrite(org, 0);

  Serial.begin(9600);
  delay(200);
  //pinMode(fan_pwm, OUTPUT);
  set_screen_output("3014DIY");
  set_screen_output("Version " + ver);

  pinMode(fan_fg, INPUT_PULLUP);
  ledcAttachPin(bl_pwm, 1);
  ledcSetup(1, 1000, 8);
  ledcWrite(1, 20);

  ledcAttachPin(fan_pwm, 0);
  ledcSetup(0, 174 / 2, 8);
  while (!SPIFFS.begin(true)) {
    //Serial.print("...");
  }
  readconfig();
  //Serial.println("SPIFFS ok");
  if (ssid != "")
  {
    set_screen_output(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);
    //Serial.println("WiFi Connecting");
    WiFi.begin(ssid.c_str(), pswd.c_str());
    int count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      count++;
      //Serial.println(".");
      if (count > 20)
      {
        //initSoftAP();
        break;
      }
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      ip = WiFi.localIP().toString();
      //Serial.println(ip);
      set_screen_output(WiFi.localIP().toString());
    }
    else
    {
      initSoftAP();
    }

  }
  else
  {
    initSoftAP();
  }

  initWebServer();

  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFontDirection(0);
  u8g2.setDisplayRotation(U8G2_R0);
  u8g2.setFlipMode(1);
  delay(200);
  //Serial.println("attachInterrupt");
  //
  attachInterrupt(digitalPinToInterrupt(fan_fg), fanfgcout, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(computer_pwm), computerpwm, CHANGE);
  set_screen_output("");
}
int s;
int st = 0;
unsigned long debouncdDelay4 = 2000;
unsigned long lastDebounceTime4 = 0;
unsigned long debouncdDelay = 5000;
unsigned long lastDebounceTime = 0;
unsigned long debouncdDelay2 = 1000;
unsigned long lastDebounceTime2 = 0;
int losttime = 0;
String txtMsg = "";
int lastpwm=0;
void loop() {
  if (lastDebounceTime > millis()) {
    lastDebounceTime = millis();
  }
  if (lastDebounceTime2 > millis()) {
    lastDebounceTime2 = millis();
  }
  if (lastDebounceTime4 > millis()) {
    lastDebounceTime4 = millis();
  }
  if (millis() - lastDebounceTime2 > debouncdDelay2)
  {
    fanfg = 1000 * fgcount * 60 / 2 / 12 / (millis() - lastDebounceTime2);
    s_fan_fpm = String(fanfg);
    fgcount = 0;
    if (digitalRead(computer_12v) == 1) {
      computer_is_on = 1;
      computer_is_on_s = "on";
      computer_off_count = 0;
      computer_on_count += 1;
      ledcWrite(1, 100);

    } else {
      computer_on_count = 0;
      if (computer_off_count < 5) {
        computer_off_count += 1;
      } else {
        computer_is_on = 0;
        computer_is_on_s = "off";

      }
    }

    
    int scount = 0;
    int hcount = 0;
    int lcount = 0;
    bool lastinp = 0;
    bool getinp = 0;
    for (int i = 0; i < 2000; i++) {
      getinp = digitalRead(computer_pwm);
      if (lastinp != getinp) {
        scount += 1;
      }
      if (scount > 2 && scount < 13) {
        if (getinp == 0) {
          lcount += 1;
        } else {
          hcount += 1;
        }
      }
      lastinp = getinp;
    }
    if(lcount+hcount>0){
      cpwm=1000*hcount/(lcount+hcount);
    }
    if(cpwm>lastpwm){
      lastpwm=cpwm;
    }else{
      if(lastpwm-cpwm>50){
        lastpwm=cpwm;
      }
    }
    
    if (lastpwm > 0) {
      fan_pwm_current = lastpwm * 850 / 1000;
    }
    //Serial.println(lastpwm);
    if (fan_pwm_current < 174) {
      fan_pwm_current = 174;
    }
    if (poweron_time < 30) {
      fan_pwm_current = 174;
    }
    if (computer_is_on_s == "off") {
      poweron_time = 0;
      ledcWrite(0, 0);
      ledcWrite(1, 5);
    } else {
      if (poweron_time < 100) {
        poweron_time += 1;
      }
      ledcSetup(0, fan_pwm_current / 2, 8);
      ledcWrite(0, 100);
    }
    //Serial.print("fan_pwm_current ");
    
    //Serial.print(" computer_is_on ");
    //Serial.println(computer_is_on_s);
    lastDebounceTime2 = millis();
  }
  server.handleClient();

  if (computer_is_on == 1) {
    while (Serial.available()) {
      int s;
      s = Serial.read();
      if (s == 0x3C) {
        istr = 0;
        line1 = "";
      } else if (s == 0x3E) {
        istr = 10;
      } else if (s == 0x28) {
        istr = 1;
        line2 = "";
      } else if (s == 0x29) {
        istr = 10;
      } else if (s == 0x5B) {
        istr = 2;
        chars = "";
        intchars = "";
      } else if (s == 0x5D) {
        istr = 10;
        for (int i = 1; i < 99; i++) {
          points[99 - i] = points[99 - i - 1];
        }
        points[0] = intchars.toInt();
      } else if (s == 0x7B) {
        istr = 3;
        set_temperature_fan_pos = 0;
      } else if (s == 0x7D) {
        istr = 10;
      } else {
        if (istr == 0) {
          line1 += (char)s;
        } else if (istr == 1) {
          line2 += (char)s;
        } else if (istr == 2) {
          if (isDigit(s)) {
            intchars += (char)s;
          } else if (s == 0x20) {
          } else {
            chars += (char)s;
          }
        } else if (istr == 3) {
          if (isDigit(s)) {
            tempstr += (char)s;
          } else if ((char)s == '|') {
            set_temperature_fan[set_temperature_fan_pos] = tempstr.toInt();
            tempstr = "";
            set_temperature_fan_pos += 1;
          }
        }
      }

      delay(2);
    }

    if ( t < millis() ) {
      t = millis() + 100;
      if (fanf < 3) {
        fanf += 1;
      } else {
        fanf = 0;
      }
    }
    if (millis() - lastDebounceTime4 > debouncdDelay4)
    {
      lastDebounceTime4 = millis();
      if (screen_output_get != screen_output_set)
      {
        if (screen_output[screen_output_get] != "")
        {
          screenmsg = screen_output[screen_output_get];
        }
        screen_output_get += 1;
        if (screen_output_get == 30)
        {
          screen_output_get = 0;
        }
      }
    }

    u8g2.firstPage();
    do {
      if (screen_output_get == screen_output_set)
      {
        for (int i = 0; i < 99; i++) {
          u8g2.drawPixel(99 - i, 49 - points[i] / 2);
        }
        u8g2.drawXBMP(0, 49, xline_x, xline_y, xline);
        u8g2.drawXBMP(99, 0, yline_x, yline_y, yline);
        u8g2.setFont(u8g2_font_unifont_h_symbols);
        u8g2.drawUTF8(106, 10, "99");
        u8g2.drawUTF8(106, 30, chars.c_str());
        u8g2.drawUTF8(106, 50, "0");
        u8g2.setFont(u8g2_font_helvR08_tn);
        if (fanf == 0) {
          u8g2.drawXBMP(110, 65, fan_x, fan_y, fan);
        }
        if (fanf == 1) {
          u8g2.drawXBMP(110, 65, fan1_x, fan1_y, fan1);
        }
        if (fanf == 2) {
          u8g2.drawXBMP(110, 65, fan2_x, fan2_y, fan2);
        }
        if (fanf == 3) {
          u8g2.drawXBMP(110, 65, fan3_x, fan3_y, fan3);
        }
        //u8g2.drawUTF8(105, 96, String(fanf).c_str());

        u8g2.drawUTF8(105 + 3 * (4 - s_fan_fpm.length()), 96, s_fan_fpm.c_str());
        u8g2.setFont(u8g2_font_unifont_h_symbols);
        //u8g2.drawUTF8(0, 76,String(digitalRead(computer_12v)).c_str());
        u8g2.drawUTF8(0, 76, line1.c_str());
        u8g2.drawUTF8(0, 94, line2.c_str());
      } else {
        if (screenmsg == "3014DIY") {
          u8g2.setFont(u8g2_font_bubble_tr);
          u8g2.drawUTF8(1, 60, screenmsg.c_str());
        } else {
          u8g2.setFont(u8g2_font_unifont_h_symbols);
          u8g2.drawUTF8((128 - screenmsg.length() * 10) / 2, 56, screenmsg.c_str());
        }
      }
    } while ( u8g2.nextPage() );
  } else {
    while (Serial.available()) {
      Serial.read();
    }
    u8g2.clearDisplay();
  }
}
