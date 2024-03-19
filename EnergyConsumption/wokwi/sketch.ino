#include <WiFi.h>
#include <esp_now.h>

/*VALENTINA VILLA: 10705748
  SIMONE DI IENNO: 10938038
  ALEX SPAGNI: 10922295*/

/*SKETCH OF THE IMPLEMENTATION:
We are using ESP32 with the ultrasonic sensor. We have connected TRIG to the 12 pin and ECHO to the 14. Trig is used 
to start the measurement and Echo is used to get the distance. That's why TRIG is set to OUTPUT and ECHO to INPUT. After configuring esp-now Wifi,
we have defined the sending and receiving callbacks called whenever a message is sent and received over the network. In case of sending, the function simply
prints the Status, while after receiving, the message is printed. So the esp-32 triggers the sensor and reads values got from the ECHO pin, then depending on
the distance, it sends the "OCCUPIED/FREE" string to the sink node address. Subsequently, before turning off the wifi and going to deep sleep state, the esp-32 waits
(through delay(100)) in order to increase the possibility of receiving the message sent before. After that period it sleeps for X seconds and then wakes up.

We used millis() function to print the timestamps useful for exercise 2*/

#define TRIG 12
#define ECHO 14
#define uS_TO_S_FACTOR 100000 //Conversion factor for micro seconds to seconds
#define X 53  //LEADER'S ID CODE: 10705748 => 48MOD50 + 5 = 53

// Sink node address (in that case is the ESP32)
uint8_t address[] = {0x8C, 0xAA, 0xB5, 0x84, 0xFB, 0x90};

esp_now_peer_info_t peerInfo;

// Sending callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent" : "Error");
}

// Receiving Callback
void OnDataRecv(const uint8_t * mac, const uint8_t *data, int len) {
  Serial.print("Message received: ");
  char receivedString[len];
  memcpy(receivedString, data, len);
  Serial.println(String(receivedString));
}

void setup() {
  Serial.begin(115200);

  Serial.println("Start idle phase");
  Serial.println(millis());

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  Serial.println("start measurement phase");
  Serial.println(millis());

  // Let's start the measurement
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  // Read result of the measurement:
  int duration = pulseIn(ECHO, HIGH);
  int distance = duration/58;

  // Print of distance
  Serial.print("Distance in CM: ");
  Serial.println(distance);

  Serial.println("finish measurement phase");
  Serial.println(millis());

  // Setup of esp-now Wifi
  Serial.println("start up wi_fi phase");
  Serial.println(millis());

  Serial.println("Enabling wifi");
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  memcpy(peerInfo.peer_addr, address, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Send message based on distance
  Serial.println("start transmission phase");
  Serial.println(millis());
  String message = "";
  if(distance < 50) {message = "OCCUPIED";}
  else {message = "FREE";}
  esp_now_send(address, (uint8_t*)message.c_str(), message.length() + 1);

  Serial.println("end trasmission phase");
  Serial.println(millis());

  // Delay to ensure ESP32 doesn't sleep before receiving availability parking message
  delay(100);

  // Disable Wi-Fi before going to sleep
  Serial.println("Disabling Wi-Fi");
  WiFi.mode(WIFI_OFF);

  //REMEMBER TO REMOVE 100 MILLISECONDS FROM THIS POINT ON
  Serial.println("end wi_fi phase");
  Serial.println(millis());

 // Enable timer wakeup for X seconds
  esp_sleep_enable_timer_wakeup(X * uS_TO_S_FACTOR);
  Serial.flush();

  Serial.println("Finish idle phase");
  Serial.println(millis());

  Serial.println("++++Going to sleep now++++");
  Serial.println(millis());
  
  // Go to sleep now
  esp_deep_sleep_start();
}

void loop() {}