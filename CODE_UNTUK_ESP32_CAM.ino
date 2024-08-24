#include "esp_camera.h"
#include <WiFi.h>
#include <HardwareSerial.h>

#define CAMERA_MODEL_AI_THINKER
#define Relay 4
#define Red 13
#define Green 12
#define FLASH_GPIO 4

#include "camera_pins.h"

const char* ssid = "nama wifi"; 
const char* password = "password"; 

void startCameraServer();

boolean matchFace = false;
boolean activateRelay = false;
long prevMillis = 0;
int interval = 3000;

HardwareSerial mySerial(1); 

void setup() {
  pinMode(Relay, OUTPUT);
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(FLASH_GPIO, OUTPUT);
  
  digitalWrite(Relay, HIGH);
  digitalWrite(Red, HIGH);
  digitalWrite(Green, LOW);
  digitalWrite(FLASH_GPIO, LOW);
  
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 14, 15); 

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

  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();

  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  if(matchFace == true && activateRelay == false) {
    activateRelay = true;
    digitalWrite(Relay, LOW);
    digitalWrite(Green, HIGH);
    digitalWrite(Red, LOW);
    prevMillis = millis();
  }
  if(activateRelay == true && millis() - prevMillis > interval) {
    activateRelay = false;
    matchFace = false;
    digitalWrite(Relay, HIGH);
    digitalWrite(Green, LOW);
    digitalWrite(Red, HIGH);
  }

  if (mySerial.available() > 0) {
    String uid = mySerial.readStringUntil('\n');
    uid.trim(); 
    Serial.println("Received UID: " + uid);
    
    if (uid.equals("C3223628")) { 
      Serial.println("Authorized UID, opening door...");
      digitalWrite(Relay, LOW);
      delay(3000);
      digitalWrite(Relay, HIGH);
    } else {
      Serial.println("Unauthorized UID.");
    }
  }
}
