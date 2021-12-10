/*
 * multiple_accessories.ino
 *
 *  Created on: 2020-05-16
 *      Author: Mixiaoxiao (Wang Bin)
 *
 *
 * This example is a bridge (aka a gateway) which contains multiple accessories.
 *
 * This example includes 6 sensors:
 * 1. Temperature Sensor (HAP section 8.41)
 * 2. Humidity Sensor (HAP section 8.20)
 * 3. Light Sensor (HAP section 8.24)
 * 4. Contact Sensor (HAP section 8.9)
 * 5. Motion Sensor (HAP section 8.28)
 * 6. Occupancy Sensor (HAP section 8.29)
 *
 * You should:
 * 1. read and use the Example01_TemperatureSensor with detailed comments
 *    to know the basic concept and usage of this library before other examples。
 * 2. erase the full flash or call homekit_storage_reset() in setup()
 *    to remove the previous HomeKit pairing storage and
 *    enable the pairing with the new accessory of this new HomeKit example.
 */

#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"


#include "DHT.h"
 
#define DHTPIN D1 // pino que estamos conectado
#define DHTTYPE DHT11 // DHT 11
#define PIN_SWITCH D5

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

DHT dht(DHTPIN, DHTTYPE);

bool automatico;
bool currentState;


//==============================
// HomeKit setup and loop
//==============================

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_temperature;
extern "C" homekit_characteristic_t cha_humidity;
extern "C" homekit_characteristic_t cha_switch_on1;
extern "C" homekit_characteristic_t cha_switch_on2;


void cha_switch_on1_setter(const homekit_value_t value)
{
  cha_switch_on1.value.bool_value = value.bool_value; //sync the value
  currentState = value.bool_value;
}

void cha_switch_on2_setter(const homekit_value_t value)
{
  cha_switch_on2.value.bool_value = value.bool_value; //sync the value
  automatico = value.bool_value;
}


void setup() {
	Serial.begin(115200);
  pinMode(PIN_SWITCH, OUTPUT);
  digitalWrite(PIN_SWITCH, HIGH);

  cha_switch_on1.setter = cha_switch_on1_setter;
  cha_switch_on2.setter = cha_switch_on2_setter;

  automatico = true;
  currentState = false;
  dht.begin();
	wifi_connect(); // in wifi_info.h
	//homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
	my_homekit_setup();
}

void loop() {
	my_homekit_loop();
  if (!WiFi.isConnected()) {
     wifi_connect(); 
  }
	delay(10);
}

// Called when the value is read by iOS Home APP
homekit_value_t cha_programmable_switch_event_getter() {
	// Should always return "null" for reading, see HAP section 9.75
	return HOMEKIT_NULL_CPP();
}

void my_homekit_setup() {
	arduino_homekit_setup(&config);
}

static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;

void my_homekit_loop() {
	arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_report_millis) {
		// report sensor values every 10 seconds
		next_report_millis = t + 10 * 1000;
		my_homekit_report();
	}
	if (t > next_heap_millis) {
		// Show heap info every 5 seconds
		next_heap_millis = t + 5 * 1000;
		LOG_D("Free heap: %d, HomeKit clients: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	}
}

void my_homekit_report() {
	// FIXME, read your real sensors here.
  float h = dht.readHumidity();
  float t = dht.readTemperature();

	cha_temperature.value.float_value = t;
	homekit_characteristic_notify(&cha_temperature, cha_temperature.value);

	cha_humidity.value.float_value = h;
	homekit_characteristic_notify(&cha_humidity, cha_humidity.value);

  if (automatico) {
    if (t > 28)  {
      digitalWrite(PIN_SWITCH, LOW);
      cha_switch_on1.value.bool_value = true;
      homekit_characteristic_notify(&cha_switch_on1, cha_switch_on1.value);
    } else {
      digitalWrite(PIN_SWITCH, HIGH);
      cha_switch_on1.value.bool_value = false;
      homekit_characteristic_notify(&cha_switch_on1, cha_switch_on1.value);
    }
  }
  else {
    if (currentState) {
      digitalWrite(PIN_SWITCH, LOW);
      cha_switch_on1.value.bool_value = true;
      homekit_characteristic_notify(&cha_switch_on1, cha_switch_on1.value);
    } else {
      digitalWrite(PIN_SWITCH, HIGH);
      cha_switch_on1.value.bool_value = false;
      homekit_characteristic_notify(&cha_switch_on1, cha_switch_on1.value);
    }    
  } 
	LOG_D("t %.1f, h %.1f, auto %b, current %b", t, h, automatico, currentState );
}
