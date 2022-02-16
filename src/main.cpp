/*
	==== WATER BRAIN ESP32 ====
*/

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

uint8_t broadcastAddressConductor[]		= {0x02, 0x50, 0xC5, 0xAC, 0xC9, 0xE5}; // MAC Board 4 / ESP32
uint8_t broadcastAddressWaterBrain[]	= {0x12, 0xB4, 0x92, 0x12, 0x27, 0xFA}; // MAC Board 1 / ESP8266

// Sensors readings
int temperature;
int level;
int ph;
int ec;
// float temperature;
// float level;
// float ph;
// float ec;
typedef struct struct_message {
		int temperature;
		int level;
		int ph;
		int ec;
		// float temperature;
		// float level;
		// float ph;
		// float ec;
} struct_message;
struct_message WaterBrainReadings;

// Orders
int orderMeasures;
int orderSetup;
typedef struct struct_order {
		int measures;
		int setup;
} struct_order;
struct_order incomingOrders;

String success;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
void getReadings();

void setup() {
	// Init Serial Monitor
	Serial.begin(115200);
 
	// Set device as a Wi-Fi Station
	WiFi.mode(WIFI_STA);

	esp_wifi_set_mac(WIFI_IF_STA, &broadcastAddressWaterBrain[0]);
	Serial.print("[NEW] ESP32 Board MAC Address: ");
	Serial.println(WiFi.macAddress());

	// Init ESP-NOW
	if (esp_now_init() != ESP_OK) {
		Serial.println("Error initializing ESP-NOW");
		return;
	}

	// Once ESPNow is successfully Init, we will register for Send CB to
	// get the status of Trasnmitted packet
	esp_now_register_send_cb(OnDataSent);
	
	// Register peer
	esp_now_peer_info_t peerInfo;
	memcpy(peerInfo.peer_addr, broadcastAddressConductor, 6);
	peerInfo.channel = 0;	
	peerInfo.encrypt = false;
	
	// Add peer				
	if (esp_now_add_peer(&peerInfo) != ESP_OK){
		Serial.println("Failed to add peer");
		return;
	}
	// Register for a callback function that will be called when data is received
	esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
	Serial.print("\r\nLast Packet Send Status: ");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
	if (status ==0){
		success = "Delivery Success :)";
	}
	else{
		success = "Delivery Fail :(";
	}
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
	memcpy(&incomingOrders, incomingData, sizeof(incomingOrders));
	Serial.print("Bytes received: ");
	Serial.println(len);
	orderMeasures	= incomingOrders.measures;
	orderSetup		= incomingOrders.setup;
	Serial.printf("[RCV-WBR] Measures: %i\n", orderMeasures);
	Serial.printf("[RCV-WBR] Setup: %i\n", orderSetup);
	if (orderMeasures == 1) {
		getReadings();
	}
	if (orderSetup == 1) {
		Serial.println("SETUP MODE!");
	}
}

void getReadings() {
	WaterBrainReadings.temperature	= random(15, 25);
	WaterBrainReadings.level		= random(35, 45);
	WaterBrainReadings.ph			= random(0, 14);
	WaterBrainReadings.ec			= random(100, 200);
	Serial.printf("Temperature: %i\n", WaterBrainReadings.temperature);
	Serial.printf("Level: %i\n", WaterBrainReadings.level);
	Serial.printf("pH: %i\n", WaterBrainReadings.ph);
	Serial.printf("EC: %i\n", WaterBrainReadings.ec);

	// Send message via ESP-NOW
	esp_err_t result = esp_now_send(broadcastAddressConductor, 
		(uint8_t *) &WaterBrainReadings, sizeof(WaterBrainReadings));
	if (result == ESP_OK) {
		Serial.println("Sent with success");
	} else {
		Serial.println("Error sending the data");
	}
}