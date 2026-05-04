# AirPoint 🖱️

> **ESP32-CAM Air Mouse via BLE HID** — Controlla il cursore del PC muovendo la mano nell'aria, senza superfici fisiche.

[![Platform](https://img.shields.io/badge/platform-ESP32--CAM-blue)](https://docs.espressif.com/projects/esp-idf/)
[![Protocol](https://img.shields.io/badge/wireless-BLE%205.0%20HID-teal)](https://www.bluetooth.com/specifications/specs/hid-over-gatt-profile-1-0/)
[![Framework](https://img.shields.io/badge/framework-Arduino-orange)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

---

## Panoramica

AirPoint è un dispositivo IoT embedded che trasforma l'ESP32-CAM in un mouse wireless tramite Bluetooth Low Energy. Il giroscopio dell'MPU-6050 rileva il movimento della mano, un pipeline di filtraggio digitale (dead zone + EMA) rimuove il rumore, e il risultato viene trasmesso al PC come evento HID standard — senza driver da installare.

```
MPU-6050 → Dead Zone → Filtro EMA → BLE HID → PC / Mac / Linux
   IMU        rumore      smoothing    GATT      nativo
```

**Architettura:** Edge Computing puro. Tutta la logica risiede sull'ESP32, latenza < 10 ms.

---

## Hardware

|  Componente |                  Ruolo                  |          Pin           |
|-------------|-----------------------------------------|------------------------|
| ESP32-CAM   | MCU + BLE 5.0 (SoC dual-core 240 MHz)   |           -            |
| MPU-6050    | IMU 6-DOF (gyro + accel)                | SDA→GPIO14, SCL→GPIO15 |
| Pulsante SX | Click sinistro                          | GPIO13 (INPUT_PULLUP)  |
| Pulsante DX | Click destro                            | GPIO12 (INPUT_PULLUP)  |

### Schema di collegamento

```
ESP32-CAM          MPU-6050
GPIO14 (SDA) ───── SDA
GPIO15 (SCL) ───── SCL
3.3V         ───── VCC
GND          ───── GND

GPIO13 ── [BTN SX] ── GND
GPIO12 ── [BTN DX] ── GND
```

---

## Software

### Dipendenze

Installa tramite Arduino Library Manager:

|                             Libreria                             | Versione testata |
|------------------------------------------------------------------|------------------|
| [ESP32 BLE Mouse](https://github.com/T-vK/ESP32-BLE-Mouse)       |      0.3.x       |
| [Adafruit MPU6050](https://github.com/adafruit/Adafruit_MPU6050) |       2.x        |
| Adafruit Unified Sensor                                          |       1.x        |

### Parametri configurabili

```cpp
#define SPEED        14      // Velocità cursore (1-30)
#define DEAD_ZONE    0.05f   // Soglia rumore giroscopio (rad/s)
#define SMOOTH_FACTOR 0.35f  // Coefficiente filtro EMA (0.1-1.0)
#define SDA_PIN      14
#define SCL_PIN      15
#define LEFTBUTTON   13
#define RIGHTBUTTON  12
```

### Tuning rapido

|        Problema        |               Soluzione               |
|------------------------|---------------------------------------|
| Cursore trema da fermo | Aumenta `DEAD_ZONE` (es. 0.08)        |
| Movimento troppo lento | Aumenta `SPEED` (es. 16-18)           |
| Movimento a scatti     | Diminuisci `SMOOTH_FACTOR` (es. 0.20) |
| Troppa inerzia         | Aumenta `SMOOTH_FACTOR` (es. 0.45)    |

---

## Pipeline di Elaborazione del Segnale

```
Lettura gyro (100 Hz)
       │
       ▼
  Dead Zone          → azzera se |val| < 0.05 rad/s
  (elimina tremore)
       │
       ▼
  Filtro EMA         → y[n] = 0.35·x[n] + 0.65·y[n-1]
  (smoothing)
       │
       ▼
  Mapping int8       → constrain(val × SPEED, -127, 127)
  (range HID)
       │
       ▼
  BLE HID TX         → bleMouse.move(x, y)
```

---

## Installazione

1. **Clona il repository**
   ```bash
   git clone https://github.com/aCommonProgrammer/AirPoint.git
   ```

2. **Apri `AirPoint.ino`** in Arduino IDE

3. **Installa le dipendenze** (Library Manager)

4. **Configura la board:**
   - Board: `AI Thinker ESP32-CAM`
   - Upload Speed: `115200`
   - Partition Scheme: `Huge APP (3MB No OTA)`

5. **Carica** — collega l'ESP32-CAM con l'adattatore FTDI (GPIO0 a GND durante il flash)

6. **Connetti via BLE** — cerca "ESP32 Bluetooth Mouse" nelle impostazioni Bluetooth del PC

---

## Struttura del Repository

```
airpoint/
├── airpoint.ino          # Firmware principale
├── README.md
```

---

## Contesto IoT

AirPoint è sviluppato come progetto per il corso di **Reti per l'Internet of Things** (laurea triennale). Dimostra concretamente:

- **Comunicazioni WPAN** — BLE 5.0 con profilo HID over GATT, topologia Piconet
- **Edge Computing** — elaborazione on-device, latenza < 10 ms, nessun cloud
- **Efficienza energetica** — MPU sleep mode, loop rate 100 Hz, dead zone per ridurre TX BLE
- **Gestione del rumore** — pipeline dead zone + EMA per dati inerziali
- **Architettura IoT** — sensore → MCU → wireless → host, livelli 1-3 del IoT Reference Model

---

## Licenza

MIT [LICENSE](LICENSE)
