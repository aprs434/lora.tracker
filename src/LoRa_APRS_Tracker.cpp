#include <APRS-Decoder.h>
#include <Arduino.h>
#include <LoRa.h>
#include <OneButton.h>
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <logger.h>

#include "BeaconManager.h"
#include "configuration.h"
#include "display.h"
#include "pins.h"
#include "power_management.h"

Configuration Config;
BeaconManager BeaconMan;

PowerManagement powerManagement;
OneButton       userButton = OneButton(BUTTON_PIN, true, true);

HardwareSerial ss(1);
TinyGPSPlus    gps;

void load_config();
void setup_lora();
void setup_gps();

char *ax25_base91enc(char *s, uint8_t n, uint32_t v);
String createDateString(time_t t);
String createTimeString(time_t t);
String getSmartBeaconState();
String padding(unsigned int number, unsigned int width);

static bool send_update = true;

static void handle_tx_click() {
  send_update = true;
}

static void handle_next_beacon() {
  BeaconMan.loadNextBeacon();
  show_display(BeaconMan.getCurrentBeaconConfig()->callsign, 2000);
}

// cppcheck-suppress unusedFunction
void setup() {
  Serial.begin(115200);

#ifdef TTGO_T_Beam_V1_0
  Wire.begin(SDA, SCL);
  if (!powerManagement.begin(Wire)) {
    logPrintlnI("AXP192 init done!");
  } else {
    logPrintlnE("AXP192 init failed!");
  }
  powerManagement.activateLoRa();
  powerManagement.activateOLED();
  powerManagement.activateGPS();
  powerManagement.activateMeasurement();
#endif

  delay(500);
  logPrintlnI("APRS 434 LoRa Tracker by Serge Y. Stroobandt, ON4AA");
  setup_display();

  show_display("APRS 434", "LoRa Tracker", "v0.4.0", "", "LESS BYTES,", "MORE RANGE", 2000);
  load_config();

  setup_gps();
  setup_lora();

  if (Config.ptt.active) {
    pinMode(Config.ptt.io_pin, OUTPUT);
    digitalWrite(Config.ptt.io_pin, Config.ptt.reverse ? HIGH : LOW);
  }

  // make sure wifi and bt is off as we don't need it:
  WiFi.mode(WIFI_OFF);
  btStop();

  if (Config.button.tx) {
    // attach TX action to user button (defined by BUTTON_PIN)
    userButton.attachClick(handle_tx_click);
  }

  if (Config.button.alt_message) {
    userButton.attachLongPressStart(handle_next_beacon);
  }

  logPrintlnI("Smart Beacon is " + getSmartBeaconState());
  show_display("INFO", "Smart Beacon is " + getSmartBeaconState(), 1000);
  logPrintlnI("setup done...");
  delay(500);
}

// cppcheck-suppress unusedFunction
void loop() {
  userButton.tick();

  if (Config.debug) {
    while (Serial.available() > 0) {
      char c = Serial.read();
      // Serial.print(c);
      gps.encode(c);
    }
  } else {
    while (ss.available() > 0) {
      char c = ss.read();
      // Serial.print(c);
      gps.encode(c);
    }
  }

  bool          gps_time_update     = gps.time.isUpdated();
  bool          gps_loc_update      = gps.location.isUpdated();
  static time_t nextBeaconTimeStamp = -1;

  static double       currentHeading          = 0;
  static double       previousHeading         = 0;
  static unsigned int rate_limit_message_text = 0;

  if (gps.time.isValid()) {
    setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());

    if (gps_loc_update && nextBeaconTimeStamp <= now()) {
      send_update = true;
      if (BeaconMan.getCurrentBeaconConfig()->smart_beacon.active) {
        currentHeading = gps.course.deg();
        // enforce message text on slowest Config.smart_beacon.slow_rate
        rate_limit_message_text = 0;
      } else {
        // enforce message text every n's Config.beacon.timeout frame
        if (BeaconMan.getCurrentBeaconConfig()->timeout * rate_limit_message_text > 30) {
          rate_limit_message_text = 0;
        }
      }
    }
  }

  static double   lastTxLat       = 0.0;
  static double   lastTxLng       = 0.0;
  static double   lastTxdistance  = 0.0;
  static uint32_t txInterval      = 60000L; // Initial 60 secs internal
  static uint32_t lastTxTime      = millis();
  static int      speed_zero_sent = 0;

  static bool   BatteryIsConnected   = false;
  static String batteryVoltage       = "";
  static String batteryChargeCurrent = "";
#ifdef TTGO_T_Beam_V1_0
  static unsigned int rate_limit_check_battery = 0;
  if (!(rate_limit_check_battery++ % 60))
    BatteryIsConnected = powerManagement.isBatteryConnect();
  if (BatteryIsConnected) {
    batteryVoltage       = String(powerManagement.getBatteryVoltage(), 2);
    batteryChargeCurrent = String(powerManagement.getBatteryChargeDischargeCurrent(), 0);
  }
#endif

  if (!send_update && gps_loc_update && BeaconMan.getCurrentBeaconConfig()->smart_beacon.active) {
    uint32_t lastTx = millis() - lastTxTime;
    currentHeading  = gps.course.deg();
    lastTxdistance  = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), lastTxLat, lastTxLng);
    if (lastTx >= txInterval) {
      // Trigger Tx Tracker when Tx interval is reach
      // Will not Tx if stationary bcos speed < 5 and lastTxDistance < 20
      if (lastTxdistance > 20) {
        send_update = true;
      }
    }

    if (!send_update) {
      // Get headings and heading delta
      double headingDelta = abs(previousHeading - currentHeading);

      if (lastTx > BeaconMan.getCurrentBeaconConfig()->smart_beacon.min_bcn * 1000) {
        // Check for heading more than 25 degrees
        if (headingDelta > BeaconMan.getCurrentBeaconConfig()->smart_beacon.turn_min && lastTxdistance > BeaconMan.getCurrentBeaconConfig()->smart_beacon.min_tx_dist) {
          send_update = true;
        }
      }
    }
  }

  if (send_update && gps_loc_update) {
    send_update = false;

    nextBeaconTimeStamp = now() + (BeaconMan.getCurrentBeaconConfig()->smart_beacon.active ? BeaconMan.getCurrentBeaconConfig()->smart_beacon.slow_rate : (BeaconMan.getCurrentBeaconConfig()->timeout * SECS_PER_MIN));

    APRSMessage msg;
    msg.setSource(BeaconMan.getCurrentBeaconConfig()->callsign);
    msg.setPath(BeaconMan.getCurrentBeaconConfig()->path);
    msg.setDestination("AP");

    float Tlat, Tlon;
    float Tspeed=0, Tcourse=0;
    Tlat    = gps.location.lat();
    Tlon    = gps.location.lng();
    Tcourse = gps.course.deg();
    Tspeed  = gps.speed.knots();

    uint32_t aprs_lat, aprs_lon;
    aprs_lat = 900000000 - Tlat * 10000000;
    aprs_lat = aprs_lat / 26 - aprs_lat / 2710 + aprs_lat / 15384615;
    aprs_lon = 900000000 + Tlon * 10000000 / 2;
    aprs_lon = aprs_lon / 26 - aprs_lon / 2710 + aprs_lon / 15384615;

    String Ns, Ew, helper;
    if(Tlat < 0) { Ns = "S"; } else { Ns = "N"; }
    if(Tlat < 0) { Tlat= -Tlat; }

    if(Tlon < 0) { Ew = "W"; } else { Ew = "E"; }
    if(Tlon < 0) { Tlon= -Tlon; }

    String infoField = "!";    // Data Type ID
    infoField += BeaconMan.getCurrentBeaconConfig()->overlay;

    char helper_base91[] = {"0000\0"};
    int i;
    ax25_base91enc(helper_base91, 4, aprs_lat);
    for (i=0; i<4; i++) {
      infoField += helper_base91[i];
    }
    ax25_base91enc(helper_base91, 4, aprs_lon);
    for (i=0; i<4; i++) {
      infoField += helper_base91[i];
    }
    
    infoField += BeaconMan.getCurrentBeaconConfig()->symbol;

    ax25_base91enc(helper_base91, 1, (uint32_t) Tcourse/4 );
    infoField += helper_base91[0];
    ax25_base91enc(helper_base91, 1, (uint32_t) (log1p(Tspeed)/0.07696));
    infoField += helper_base91[0];
    infoField += "\x47";    // Compression Type (T) Byte = \b001 00 110 = 38; 38 + 33 = 72 = \x47 = G

    int speed_int = max(0, min(999, (int)gps.speed.knots()));
    if (speed_int == 0) {
      /* speed is 0.
         we send 3 packets with speed zero (so our friends know we stand still).
         After that, we save airtime by not sending speed/course 000/000.
         Btw, even if speed we really do not move, measured course is changeing
         (-> no useful / even wrong info)
      */
      if (speed_zero_sent < 3) {
        speed_zero_sent += 1;
      }
    } else {
      speed_zero_sent = 0;
    }

    msg.getBody()->setData(infoField);
    String data = msg.encode();
    logPrintlnD(data);
    show_display(" << TX >>", data);

    if (Config.ptt.active) {
      digitalWrite(Config.ptt.io_pin, Config.ptt.reverse ? LOW : HIGH);
      delay(Config.ptt.start_delay);
    }

    LoRa.beginPacket();    // Explicit LoRa Header
    // Custom LoRa Header:
    LoRa.write('<');
    LoRa.write(0xFF);
    LoRa.write(0x01);
    // APRS Data:
    LoRa.write((const uint8_t *)data.c_str(), data.length());
    LoRa.endPacket();

    if (BeaconMan.getCurrentBeaconConfig()->smart_beacon.active) {
      lastTxLat       = gps.location.lat();
      lastTxLng       = gps.location.lng();
      previousHeading = currentHeading;
      lastTxdistance  = 0.0;
      lastTxTime      = millis();
    }

    if (Config.ptt.active) {
      delay(Config.ptt.end_delay);
      digitalWrite(Config.ptt.io_pin, Config.ptt.reverse ? HIGH : LOW);
    }
  }

  if (gps_time_update) {

    show_display(BeaconMan.getCurrentBeaconConfig()->callsign, createDateString(now()) + " " + createTimeString(now()),
                 String("Next TX:   ") + (BeaconMan.getCurrentBeaconConfig()->smart_beacon.active ? "~" : "") + createTimeString(nextBeaconTimeStamp),
                 String("Sats: ") + gps.satellites.value() + " HDOP: " + gps.hdop.hdop(),
                 String(gps.location.lat(), 4) + " " + String(gps.location.lng(), 4),
                 String("Smart Beacon: " + getSmartBeaconState()));

    if (BeaconMan.getCurrentBeaconConfig()->smart_beacon.active) {
      // Change the Tx internal based on the current speed
      int curr_speed = (int)gps.speed.kmph();
      if (curr_speed < BeaconMan.getCurrentBeaconConfig()->smart_beacon.slow_speed) {
        txInterval = BeaconMan.getCurrentBeaconConfig()->smart_beacon.slow_rate * 1000;
      } else if (curr_speed > BeaconMan.getCurrentBeaconConfig()->smart_beacon.fast_speed) {
        txInterval = BeaconMan.getCurrentBeaconConfig()->smart_beacon.fast_rate * 1000;
      } else {
        /* Interval inbetween low and high speed
           min(slow_rate, ..) because: if slow rate is 300s at slow speed <=
           10km/h and fast rate is 60s at fast speed >= 100km/h everything below
           current speed 20km/h (100*60/20 = 300) is below slow_rate.
           -> In the first check, if curr speed is 5km/h (which is < 10km/h), tx
           interval is 300s, but if speed is 6km/h, we are landing in this
           section, what leads to interval 100*60/6 = 1000s (16.6min) -> this
           would lead to decrease of beacon rate in between 5 to 20 km/h. what
           is even below the slow speed rate.
        */
        txInterval = min(BeaconMan.getCurrentBeaconConfig()->smart_beacon.slow_rate, BeaconMan.getCurrentBeaconConfig()->smart_beacon.fast_speed * BeaconMan.getCurrentBeaconConfig()->smart_beacon.fast_rate / curr_speed) * 1000;
      }
    }
  }

  if ((Config.debug == false) && (millis() > 5000 && gps.charsProcessed() < 10)) {
    logPrintlnE("No GPS frames detected! Try to reset the GPS Chip with this "
                "firmware: https://github.com/lora-aprs/TTGO-T-Beam_GPS-reset");
    show_display("No GPS frames detected!", "Try to reset the GPS Chip", "https://github.com/lora-aprs/TTGO-T-Beam_GPS-reset", 2000);
  }
}


/// FUNCTIONS ///


void load_config() {
  ConfigurationManagement confmg("/tracker.json");
  Config = confmg.readConfiguration();
  BeaconMan.loadConfig(Config.beacons);
  if (BeaconMan.getCurrentBeaconConfig()->callsign == "NOCALL-10") {
    logPrintlnE("You have to change your settings in 'data/tracker.json' and "
                "upload it via \"Upload File System image\"!");
    show_display("ERROR", "You have to change your settings in 'data/tracker.json' and "
                          "upload it via \"Upload File System image\"!");
    while (true) {
    }
  }
}

void setup_lora() {
  logPrintlnI("Set SPI pins!");
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  logPrintlnI("Set LoRa pins!");
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);

  long freq = Config.lora.frequencyTx;
  logPrintI("frequency: ");
  logPrintlnI(String(freq));
  if (!LoRa.begin(freq)) {
    logPrintlnE("Starting LoRa failed!");
    show_display("ERROR", "Starting LoRa failed!");
    while (true) {
    }
  }
  LoRa.setSpreadingFactor(Config.lora.spreadingFactor);
  LoRa.setSignalBandwidth(Config.lora.signalBandwidth);
  LoRa.setCodingRate4(Config.lora.codingRate4);
  LoRa.enableCrc();

  LoRa.setTxPower(Config.lora.power);
  logPrintlnI("LoRa init done!");
  show_display("INFO", "LoRa init done!", 2000);
}

void setup_gps() {
  ss.begin(9600, SERIAL_8N1, GPS_TX, GPS_RX);
}

char *ax25_base91enc(char *s, uint8_t n, uint32_t v)
{
  /* Creates a Base-91 representation of the value in v in the string */
  /* pointed to by s, n-characters long. String length should be n+1. */

  for(s += n, *s = '\0'; n; n--)
  {
    *(--s) = v % 91 + 33;
    v /= 91;
  }

  return(s);
}

String createDateString(time_t t) {
  return String(padding(year(t), 4) + "-" + padding(month(t), 2) + "-" + padding(day(t), 2));
}

String createTimeString(time_t t) {
  return String(padding(hour(t), 2) + ":" + padding(minute(t), 2) + ":" + padding(second(t), 2));
}

String getSmartBeaconState() {
  if (BeaconMan.getCurrentBeaconConfig()->smart_beacon.active) {
    return "On";
  }
  return "Off";
}

String padding(unsigned int number, unsigned int width) {
  String result;
  String num(number);
  if (num.length() > width) {
    width = num.length();
  }
  for (unsigned int i = 0; i < width - num.length(); i++) {
    result.concat('0');
  }
  result.concat(num);
  return result;
}