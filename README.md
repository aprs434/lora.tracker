# APRS 434: Extends your LoRa range by saving bytes.
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
- TTGO T-Beam V0.7 433 MHz SX1278
- TTGO T-Beam V1 433 MHz SX1278

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


## LoRa i-Gate
⚠ Currently, the APRS&nbsp;434 tracker is still compatible with the [i-gate developped by Peter Buchegger, OE5BPA](https://github.com/lora-aprs/LoRa_APRS_iGate). However, this will soon change as more LoRa frame compression is added.
We feel confident that trackers with the proposed APRS&nbsp;434 compressed LoRa frame will eventually become dominant because of the longer range merit. To smooth out the transition, an i‑gate capable of understanding both formats (APRS&nbsp;434&nbsp;and OE5BPA) will be developed.

