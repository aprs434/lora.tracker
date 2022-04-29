# APRS 434 — Extend your LoRa range by saving bytes.
Welcome to the home of **APRS 434 tracker**, the 434 MHz LoRa APRS amateur radio GPS tracker that **extends range by saving bytes.**

Unlike other ham radio LoRa APRS trackers, this tracker aims at **deploying LoRa the way it was intended;** namely by being frugal about the number of bytes put on air. Doing so, results in a number of benefits:

- Increased battery life,
- Higher chances of good packet reception,
- Hence, increased range,
- Lower probability of packet collisions,
- Therefore, more channel capacity.

To [**learn more**](https://aprs434.github.io) about APRS 434 data link compression, visit [aprs434.github.io](https://aprs434.github.io).


## Supported Tracker Hardware
The **APRS 434** LoRa tracker firmware is developed for the relatively cheap Espressif ESP32-based LoRa GPS trackers made by TTGO. These are available from Aliexpress, Amazon or eBay.

Supported 433 MHz LoRa GPS tracker hardware:
- TTGO T-Beam v0.7 433 MHz SX1278
- TTGO T-Beam v1 433 MHz SX1278

> ⚠ Please, make sure to order a 433 MHz version!

![TTGO T-Beam](pics/Tracker.png)


## Firmware Compilation and Configuration

### Quick Start Guides
- [German](https://www.lora-aprs.info/docs/LoRa_APRS_iGate/quick-start-guide/)
- [French](http://www.f5kmy.fr/spip.php?article509)

### Compilation
The best success is to use PlatformIO (and it is the only platform where I can support you). 

- Go to [PlatformIO](https://platformio.org/) download and install the IDE. 
- If installed open the IDE, go to the left side and klick on 'extensions' then search for 'PatformIO' and install.
- When installed click 'the ant head' on the left and choose import the project on the right.
- Just open the folder and you can compile the Firmware.

### Configuration
- You can find all nessesary settings to change for your configuration in **data/tracker.json**.
- The `button_tx` setting enables manual triggering of the beacon using the middle button on the T-Beam.
- To upload it to your board you have to do this via **Upload File System image** in PlatformIO!
- To find the 'Upload File System image' click the PlatformIO symbol (the little alien) on the left side, choos your configuration, click on 'Platform' and search for 'Upload File System image'.


## LoRa APRS i-Gate
⚠ Currently, the APRS&nbsp;434 tracker is still compatible with the [i-gate developed by Peter Buchegger, OE5BPA](https://github.com/lora-aprs/LoRa_APRS_iGate). However, this will soon change as more LoRa frame compression is added.
We feel confident that trackers with the proposed APRS&nbsp;434 compressed LoRa frame will eventually become dominant because of the longer range merit. To smooth out the transition, an i‑gate capable of understanding both formats (APRS&nbsp;434&nbsp;and OE5BPA) will be developed.


## Development Road Map

### Data Link Layer

|tracker<br/>firmware|completed|feature|LoRa payload|compatible with [OE5BPA i‑gate](https://github.com/lora-aprs/LoRa_APRS_iGate)|
|:------------------:|:-------:|:-----:|:----------:|:--------------------------------------------------------------------------------:|
|v0.0|✓|original [OE5BPA tracker](https://github.com/lora-aprs/LoRa_APRS_Tracker)|113 bytes|✓|
|v0.1|✓|byte-saving [`tracker.json`](https://github.com/aprs434/lora.tracker/blob/master/data/tracker.json)|87 bytes|✓|
|v0.2|✓|fork of the [OE5BPA tracker](https://github.com/lora-aprs/LoRa_APRS_Tracker) with significantly less transmitted bytes|44 bytes|✓|
|v0.3|✓|[Base91](https://en.wikipedia.org/wiki/List_of_numeral_systems#Standard_positional_numeral_systems) compression of the location, course and speed data|31 bytes|✓|
|||random time jitter between fixed interval packets to avoid repetitive [collisions](https://en.wikipedia.org/wiki/Collision_domain)|30 bytes|✓|
|||tracker and i-gate with frame address compression,<br/>no custom header in payload|20 bytes|✗|

⚠ Currently, the APRS&nbsp;434 tracker is still compatible with the [i-gate developed by Peter Buchegger, OE5BPA](https://github.com/lora-aprs/LoRa_APRS_iGate). However, this will soon change as more LoRa frame compression is added.
We feel confident that trackers with the proposed APRS&nbsp;434 compressed LoRa frame will eventually become dominant because of the longer range merit. To smooth out the transition, an i‑gate capable of understanding both formats (APRS&nbsp;434&nbsp;and OE5BPA) will be developed.

### Tracker Hardware

|tracker<br/>firmware|completed|feature|
|:------------------:|:-------:|:-----:|
|||coordinates displayed on screen|
|||reduced power consumption through [SH1106 OLED sleep](https://bengoncalves.wordpress.com/2015/10/01/oled-display-and-arduino-with-power-save-mode/)|
|||button press to activate OLED screen|
|||ESP32 power reduction|

### Messaging
At first, only uplink messaging to an i-gate will be considered. This is useful for status updates, [SOTA self‑spotting](https://www.sotaspots.co.uk/Aprs2Sota_Info.php), or even emergencies.

On the other hand, bidirectional messaging requires time division multiplexing between the up- and downlink, based on precise GPS timing. That is because channel isolation between different up- and downlink frequencies probably would require costly and bulky resonant cavities.

|tracker<br/>firmware|completed|feature|
|:------------------:|:-------:|:-----:|
|||add a [library](https://web.archive.org/web/20190316204938/http://cliffle.com/project/chatpad/arduino/) for the [Xbox 360 Chatpad](https://nuxx.net/gallery/v/acquired_stuff/xbox_360_chatpad/) keyboard|
|||[support](https://www.hackster.io/scottpowell69/lora-qwerty-messenger-c0eee6) for the [M5Stack CardKB Mini](https://shop.m5stack.com/products/cardkb-mini-keyboard) keyboard|

### WiFi Geolocation
TBD
