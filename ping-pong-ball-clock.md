# Ping-Pong-Ball Uhr - Projektbeschreibung

**Autor:** Paul Simmen mit Vibe (GLM-5-2), 14.09.2026

## 📌 Projektübersicht

Eine kreative Uhr, die die Zeit mit 9 Ping-Pong-Bällen (40mm Durchmesser) anzeigt. Jeder Ball kann in verschiedenen Farben leuchten (Rot, Grün, Blau) und repräsentiert einen Zeitwert. Die Basis der Anzeige ist die Zahl **4**, wobei die Werte in einer 3x3-Matrix angeordnet sind.

Zusätzlich zur Ping-Pong-Ball-Anzeige wird die Zeit auf einer **TM1637 6-Zahlen 7-Segment-Anzeige** (0,36 Zoll) angezeigt.

### 🎯 Ziele

- Zeitdarstellung mit Ping-Pong-Bällen
- Internet-Zeitsynchronisation (NTP)
- Steuerung mit Raspberry Pi Zero WH oder ESP32S3
- 5V/3A Stromversorgung
- Klare visuelle Anzeige mit Farben

---

## 📦 Materialliste


| Komponente        | Menge | Beschreibung                                   |
| ----------------- | ----- | ---------------------------------------------- |
| Ping-Pong-Bälle   | 9     | 40mm Durchmesser, Standard-Tischtennisbälle    |
| RGB-LEDs          | 9     | WS2812B Neopixel (individuell ansteuerbar)     |
| Mikrocontroller   | 1     | Raspberry Pi Zero WH **oder** ESP32S3          |
| 7-Segment-Anzeige | 1     | TM1637 6-Zahlen, 0,36 Zoll                     |
| Netzteil          | 1     | 5V, 3A                                         |
| Gehäuse           | 1     | Für mechanische Fixierung der Bälle            |
| Verdrahtung       | -     | Jumper-Kabel, Lötmaterial                      |
| Widerstände       | 1-2   | 220-470Ω für LED-Strombegrenzung (falls nötig) |
| Breadboard        | 1     | Für Prototypenaufbau                           |


---

## 🔢 Matrix-Anordnung &amp; Zeitlogik

### Matrix-Layout

Die 9 Ping-Pong-Bälle sind in einer 3x3-Matrix angeordnet:

```
+-------+-------+-------+
| 48    | 12    | 3     |  Zeile 1
+-------+-------+-------+
| 32    | 8     | 2     |  Zeile 2
+-------+-------+-------+
| 16    | 4     | 1     |  Zeile 3
+-------+-------+-------+
   Spalte 1  Spalte 2  Spalte 3
```

### Zeitzerlegung

**Stunden (0-23):**

- Werte: {1, 2, 3, 4, 16}
- Beispiel: 19h = 16 + 3

**Minuten (0-59):**

- Werte: {1, 2, 3, 4, 8, 16, 48}
- Beispiel: 50m = 48 + 2

### Farbkodierung


| Farbe  | Bedeutung                        |
| ------ | -------------------------------- |
| 🔴 Rot  | Stundenkomponente                |
| 🟢 Grün | Minutenkomponente                |
| 🔵 Blau | (optional: Sekunden oder Status) |
| 🟡 Gelb | Stunden + Minuten                |


**Beispiele:**

- **6:00** → 4h + 2h → Bälle (3,2)=4 und (2,3)=2 leuchten **rot**
- **6:11** → 4h + 2h + 8m + 3m → Bälle (3,2)=4, (2,3)=2 (rot), (2,2)=8, (1,3)=3 (grün)
- **6:20** → 4h + 2h + 16m + 4m → Bälle (3,2)=4, (2,3)=2 (rot), (3,1)=16, (3,2)=4 (grün)
- **21:21** → 16h + 4h + 1h + 16m + 4m + 1m → Bälle (3,1)=16, (3,2)=4, (3,3)=1 (rot+grün)

---

## 🔌 Schaltplan

### Verbindungsschema (für beide Mikrocontroller)

```
┌─────────────────────────────────────────────────────────────┐
│                    Mikrocontroller (RasPi/ESP32)                │
└─────────────────────────────────────────────────────────────┘
                              │
                              ├── 5V ────┬─────────────┐
                              │           ├── TM1637 VCC │
                              │           └─────────────┘
                              │
                              ├── GND ──┬─────────────┐
                              │           ├── TM1637 GND │
                              │           └─────────────┘
                              │
                              ├── GPIO ──┬──────────────────────┐
                              │           ├── TM1637 CLK (z.B. GPIO 5)│
                              │           ├── TM1637 DIO (z.B. GPIO 6)│
                              │           └──────────────────────┘
                              │
                              ├── GPIO ──┬──────────────────────┐
                              │           ├── Neopixel DATA (z.B. GPIO 18)│
                              │           └──────────────────────┘
                              │
                              └── 5V ────┬─────────────┐
                                          ├── Neopixel VCC │
                                          └─────────────┘
```

### Pin-Belegung


| Komponente    | Raspberry Pi Zero WH | ESP32S3 |
| ------------- | -------------------- | ------- |
| TM1637 CLK    | GPIO 5 (Pin 29)      | GPIO 5  |
| TM1637 DIO    | GPIO 6 (Pin 31)      | GPIO 6  |
| Neopixel DATA | GPIO 18 (Pin 12)     | GPIO 18 |
| 5V            | Pin 2 oder 4         | 5V Pin  |
| GND           | Pin 6, 9, 14, etc.   | GND Pin |


---

## 💻 Software-Implementierung

### Algorithmus zur Zeitzerlegung

```python
# Matrix-Definition
MATRIX = {
    (1, 1): 48, (1, 2): 12, (1, 3): 3,
    (2, 1): 32, (2, 2): 8,  (2, 3): 2,
    (3, 1): 16, (3, 2): 4,  (3, 3): 1
}

# Wert-zu-Position Mapping
VALUE_TO_POSITION = {v: k for k, v in MATRIX.items()}

def decompose_hours(h):
    """Zerlegt Stunden in Komponenten"""
    components = []
    for value in [16, 4, 3, 2, 1]:
        if h >= value:
            components.append(value)
            h -= value
    return components

def decompose_minutes(m):
    """Zerlegt Minuten in Komponenten"""
    components = []
    for value in [48, 16, 8, 4, 3, 2, 1]:
        if m >= value:
            components.append(value)
            m -= value
    return components

def get_led_states(h, m):
    """Ermittelt, welche LEDs in welcher Farbe leuchten sollen"""
    hour_components = decompose_hours(h)
    minute_components = decompose_minutes(m)
    
    states = {pos: 0 for pos in MATRIX.keys()}  # 0=aus, 1=rot, 2=grün, 3=gelb
    
    for value in hour_components:
        pos = VALUE_TO_POSITION[value]
        states[pos] = states.get(pos, 0) | 1  # Rot setzen
    
    for value in minute_components:
        pos = VALUE_TO_POSITION[value]
        states[pos] = states.get(pos, 0) | 2  # Grün setzen
    
    return states
```

---

### Code für Raspberry Pi Zero WH (Python)

```python
# ping_pong_clock_rpi.py
# Autor: Paul Simmen mit Vibe (GLM-5-2), 14.09.2026

import time
import datetime
import requests
from rpi_ws281x import PixelStrip, Color
import board
import digitalio
import adafruit_tm1637

# Konfiguration
LED_COUNT = 9
LED_PIN = 18  # GPIO18
LED_FREQ_HZ = 800000
LED_DMA = 10
LED_BRIGHTNESS = 255
LED_INVERT = False

# Matrix-Definition (Zeile, Spalte): Wert
MATRIX = {
    (1, 1): 48, (1, 2): 12, (1, 3): 3,
    (2, 1): 32, (2, 2): 8,  (2, 3): 2,
    (3, 1): 16, (3, 2): 4,  (3, 3): 1
}

# Wert zu Position
VALUE_TO_POS = {v: k for k, v in MATRIX.items()}

# Position zu LED-Index (Zeile-weise)
POS_TO_LED = {
    (1,1): 0, (1,2): 1, (1,3): 2,
    (2,1): 3, (2,2): 4, (2,3): 5,
    (3,1): 6, (3,2): 7, (3,3): 8
}

# Farbdefinitionen
COLOR_OFF = Color(0, 0, 0)
COLOR_RED = Color(255, 0, 0)
COLOR_GREEN = Color(0, 255, 0)
COLOR_YELLOW = Color(255, 255, 0)

def decompose_hours(h):
    """Zerlegt Stunden in Komponenten"""
    components = []
    for value in [16, 4, 3, 2, 1]:
        if h >= value:
            components.append(value)
            h -= value
    return components

def decompose_minutes(m):
    """Zerlegt Minuten in Komponenten"""
    components = []
    for value in [48, 16, 8, 4, 3, 2, 1]:
        if m >= value:
            components.append(value)
            m -= value
    return components

def get_time_from_internet():
    """Holt die aktuelle Zeit vom Internet"""
    try:
        response = requests.get("http://worldtimeapi.org/api/timezone/Europe/Zurich")
        data = response.json()
        dt = datetime.datetime.fromisoformat(data["datetime"].replace("Z", "+00:00"))
        return dt.hour, dt.minute, dt.second
    except:
        # Fallback auf Systemzeit
        now = datetime.datetime.now()
        return now.hour, now.minute, now.second

def update_leds(strip, hour_components, minute_components):
    """Aktualisiert die LED-Anzeige"""
    # Alle LEDs ausschalten
    for i in range(LED_COUNT):
        strip.setPixelColor(i, COLOR_OFF)
    
    # Stunden-Komponenten (rot)
    for value in hour_components:
        if value in VALUE_TO_POS:
            pos = VALUE_TO_POS[value]
            led_idx = POS_TO_LED[pos]
            current = strip.getPixelColor(led_idx)
            # Rot hinzufügen
            strip.setPixelColor(led_idx, Color(255, current[1], current[2]))
    
    # Minuten-Komponenten (grün)
    for value in minute_components:
        if value in VALUE_TO_POS:
            pos = VALUE_TO_POS[value]
            led_idx = POS_TO_LED[pos]
            current = strip.getPixelColor(led_idx)
            # Grün hinzufügen
            strip.setPixelColor(led_idx, Color(current[0], 255, current[2]))
    
    strip.show()

def main():
    # Initialisierung
    print("Ping-Pong-Ball Uhr - Raspberry Pi Version")
    print("Autor: Paul Simmen mit Vibe (GLM-5-2)")
    
    # LED-Strip initialisieren
    strip = PixelStrip(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS)
    strip.begin()
    
    # TM1637 initialisieren
    clk = digitalio.DigitalInOut(board.D5)
    dio = digitalio.DigitalInOut(board.D6)
    display = adafruit_tm1637.TM1637(clk, dio)
    display.brightness = 5
    
    try:
        while True:
            # Zeit holen
            h, m, s = get_time_from_internet()
            
            # Zeit zerlegen
            hour_components = decompose_hours(h)
            minute_components = decompose_minutes(m)
            
            # LEDs aktualisieren
            update_leds(strip, hour_components, minute_components)
            
            # TM1637 anzeigen
            display.show(f"{h:02d}{m:02d}")
            
            # Warten
            time.sleep(1)
            
    except KeyboardInterrupt:
        # Alles ausschalten
        for i in range(LED_COUNT):
            strip.setPixelColor(i, COLOR_OFF)
        strip.show()
        display.show("    ")
        print("\nProgramm beendet.")

if __name__ == "__main__":
    main()
```

**Abhängigkeiten für Raspberry Pi:**

```bash
sudo apt-get update
sudo apt-get install python3-pip
pip3 install rpi_ws281x requests adafruit-circuitpython-tm1637
```

---

### Code für ESP32S3 (MicroPython)

```python
# ping_pong_clock_esp32.py
# Autor: Paul Simmen mit Vibe (GLM-5-2), 14.09.2026

import time
import ntptime
from machine import Pin, PWM
import network
import socket
import struct

# Konfiguration
LED_COUNT = 9
LED_PIN = 18  # GPIO18 für Neopixel

# Matrix-Definition
MATRIX = {
    (1, 1): 48, (1, 2): 12, (1, 3): 3,
    (2, 1): 32, (2, 2): 8,  (2, 3): 2,
    (3, 1): 16, (3, 2): 4,  (3, 3): 1
}

VALUE_TO_POS = {v: k for k, v in MATRIX.items()}
POS_TO_LED = {
    (1,1): 0, (1,2): 1, (1,3): 2,
    (2,1): 3, (2,2): 4, (2,3): 5,
    (3,1): 6, (3,2): 7, (3,3): 8
}

# TM1637 Pins
TM1637_CLK = 5
TM1637_DIO = 6

def decompose_hours(h):
    """Zerlegt Stunden in Komponenten"""
    components = []
    for value in [16, 4, 3, 2, 1]:
        if h >= value:
            components.append(value)
            h -= value
    return components

def decompose_minutes(m):
    """Zerlegt Minuten in Komponenten"""
    components = []
    for value in [48, 16, 8, 4, 3, 2, 1]:
        if m >= value:
            components.append(value)
            m -= value
    return components

def connect_wifi(ssid, password):
    """Verbindet mit WLAN"""
    sta_if = network.WLAN(network.STA_IF)
    if not sta_if.isconnected():
        sta_if.active(True)
        sta_if.connect(ssid, password)
        while not sta_if.isconnected():
            time.sleep(0.1)
    print("WLAN verbunden")

def get_ntp_time():
    """Holt die Zeit über NTP"""
    try:
        ntptime.settime()
        t = time.localtime()
        return t[3], t[4], t[5]  # h, m, s
    except:
        t = time.localtime()
        return t[3], t[4], t[5]

# TM1637 Implementierung für ESP32
class TM1637:
    def __init__(self, clk_pin, dio_pin, brightness=5):
        self.clk = Pin(clk_pin, Pin.OUT)
        self.dio = Pin(dio_pin, Pin.OUT)
        self.brightness = brightness
        self.start()
        self.write_data(0x8F)  # Aktivieren mit max. Helligkeit
        self.stop()
    
    def start(self):
        self.dio.value(1)
        self.clk.value(1)
        time.sleep_us(2)
        self.dio.value(0)
    
    def stop(self):
        self.dio.value(0)
        self.clk.value(1)
        time.sleep_us(2)
        self.dio.value(1)
    
    def write_byte(self, data):
        for i in range(8):
            self.clk.value(0)
            time.sleep_us(2)
            self.dio.value((data >> i) & 1)
            self.clk.value(1)
            time.sleep_us(2)
        self.clk.value(0)
        self.dio.value(1)
        time.sleep_us(2)
        self.clk.value(1)
        time.sleep_us(2)
        self.dio.value(0)
    
    def write_data(self, data):
        self.start()
        self.write_byte(data)
        self.stop()
    
    def show(self, text):
        # Einfache Implementierung - zeigt nur die ersten 4 Zeichen
        segments = [
            0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,  # 0-7
            0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71   # 8-9, A-F
        ]
        
        digits = []
        for char in text[:6]:
            if char.isdigit():
                digits.append(segments[int(char)])
            else:
                digits.append(0x00)
        
        self.start()
        self.write_byte(0x40)  # Datenmodus
        self.stop()
        
        self.start()
        self.write_byte(0xC0)  # Start bei Adresse 0
        for d in digits:
            self.write_byte(d)
        self.stop()
        
        self.start()
        self.write_byte(0x88 + self.brightness)  # Helligkeit
        self.stop()

# Neopixel Implementierung (vereinfacht)
class NeoPixel:
    def __init__(self, pin, count):
        self.pin = Pin(pin, Pin.OUT)
        self.count = count
        self.buffer = bytearray(count * 3)
    
    def set_pixel(self, index, color):
        if index < self.count:
            self.buffer[index*3] = color[0]  # R
            self.buffer[index*3+1] = color[1]  # G
            self.buffer[index*3+2] = color[2]  # B
    
    def show(self):
        # Hier würde die echte Neopixel-Implementierung kommen
        # Für dieses Beispiel: nur simulieren
        pass

def update_leds(neopixel, hour_components, minute_components):
    """Aktualisiert die LED-Anzeige"""
    # Farbdefinitionen
    COLOR_OFF = (0, 0, 0)
    COLOR_RED = (255, 0, 0)
    COLOR_GREEN = (0, 255, 0)
    COLOR_YELLOW = (255, 255, 0)
    
    # Alle LEDs ausschalten
    for i in range(LED_COUNT):
        neopixel.set_pixel(i, COLOR_OFF)
    
    # Stunden-Komponenten (rot)
    for value in hour_components:
        if value in VALUE_TO_POS:
            pos = VALUE_TO_POS[value]
            led_idx = POS_TO_LED[pos]
            current = neopixel.buffer[led_idx*3:led_idx*3+3]
            # Rot hinzufügen
            neopixel.set_pixel(led_idx, (255, current[1], current[2]))
    
    # Minuten-Komponenten (grün)
    for value in minute_components:
        if value in VALUE_TO_POS:
            pos = VALUE_TO_POS[value]
            led_idx = POS_TO_LED[pos]
            current = neopixel.buffer[led_idx*3:led_idx*3+3]
            # Grün hinzufügen
            neopixel.set_pixel(led_idx, (current[0], 255, current[2]))
    
    neopixel.show()

def main():
    print("Ping-Pong-Ball Uhr - ESP32 Version")
    print("Autor: Paul Simmen mit Vibe (GLM-5-2)")
    
    # WLAN verbinden (Anpassen!)
    connect_wifi("DEIN_SSID", "DEIN_PASSWORT")
    
    # NTP Zeit synchronisieren
    get_ntp_time()
    
    # TM1637 initialisieren
    display = TM1637(TM1637_CLK, TM1637_DIO, brightness=5)
    
    # Neopixel initialisieren
    neopixel = NeoPixel(LED_PIN, LED_COUNT)
    
    while True:
        # Zeit holen
        h, m, s = get_ntp_time()
        
        # Zeit zerlegen
        hour_components = decompose_hours(h)
        minute_components = decompose_minutes(m)
        
        # LEDs aktualisieren
        update_leds(neopixel, hour_components, minute_components)
        
        # TM1637 anzeigen
        display.show(f"{h:02d}{m:02d}")
        
        # Warten
        time.sleep(1)

if __name__ == "__main__":
    main()
```

**Hinweise für ESP32:**

1. Installiere MicroPython auf dem ESP32S3
2. Kopiere den Code auf das Gerät
3. Passe WLAN-SSID und Passwort an
4. Für echte Neopixel-Unterstützung: `neopixel` Modul verwenden

---

## ⚡ Stromversorgung

### Anforderungen

- **Spannung:** 5V DC
- **Strom:** 3A (für LEDs + Mikrocontroller + Anzeige)

### Berechnung

- Neopixel LEDs: \~60mA pro LED bei voller Helligkeit → 9 × 60mA = 540mA
- TM1637: \~20mA
- Raspberry Pi Zero WH: \~200-500mA
- ESP32S3: \~100-200mA
- **Gesamt:** \~900mA - 1,2A (3A Netzteil ist ausreichend)

### Verdrahtungstipps

- Verwende ausreichend dicke Kabel (mind. 20AWG)
- Füge einen 1000µF Kondensator am Netzteilausgang hinzu
- Für längere LED-Streifen: separate Stromversorgung

---

## 📐 Mechanische Konstruktion

### Gehäuse-Design

1. **Ball-Halterung:**
  - 3x3 Raster mit 40mm Abstand (Ping-Pong-Ball-Durchmesser)
  - Jeder Ball wird von einer LED von unten beleuchtet
  - Material: 3D-gedruckt oder aus Acryl
2. **Abmessungen:**
  - Gesamtgröße: \~120mm × 120mm × 50mm (Höhe)
  - Ball-Positionen: 20mm von der Unterkante (für LED-Platzierung)
3. **Empfohlene 3D-Druck-Vorlage:**
  ```
   +---------------------+
   |   O   O   O       |  (O = Ping-Pong-Ball)
   |   O   O   O       |
   |   O   O   O       |
   +---------------------+
  ```

### Materialempfehlungen

- **Gehäuse:** PLA oder PETG (3D-Druck)
- **Diffusor:** Transluzentes Acryl über den Bällen
- **Befestigung:** Schrauben oder Kleber

---

## 🔧 Aufbauanleitung

### Schritt 1: Hardware vorbereiten

1. Ping-Pong-Bälle vorbereiten (evtl. kleine Löcher für Lichtdurchlass)
2. LEDs in der Halterung positionieren
3. Mikrocontroller und TM1637 auf dem Breadboard platzieren

### Schritt 2: Verdrahtung

1. TM1637 mit Mikrocontroller verbinden (CLK, DIO, VCC, GND)
2. Neopixel-LEDs in Reihe schalten (Datenleitung)
3. Stromversorgung anschließen

### Schritt 3: Software installieren

- **Raspberry Pi:** Python-Skript kopieren und Abhängigkeiten installieren
- **ESP32:** MicroPython-Code hochladen

### Schritt 4: Testen

1. Einzeltest jeder LED
2. TM1637-Anzeige testen
3. Zeitzerlegung testen
4. Komplettes System testen

---

## 📊 Zeitzerlegungs-Beispiele


| Zeit  | Stunden-Zerlegung | Minuten-Zerlegung | Aktive Bälle                        |
| ----- | ----------------- | ----------------- | ----------------------------------- |
| 00:00 | -                 | -                 | Keine                               |
| 01:00 | 1                 | -                 | (3,3) rot                           |
| 02:00 | 2                 | -                 | (2,3) rot                           |
| 03:00 | 3                 | -                 | (1,3) rot                           |
| 04:00 | 4                 | -                 | (3,2) rot                           |
| 06:00 | 4 + 2             | -                 | (3,2), (2,3) rot                    |
| 06:11 | 4 + 2             | 8 + 3             | (3,2), (2,3) rot; (2,2), (1,3) grün |
| 06:20 | 4 + 2             | 16 + 4            | (3,2), (2,3) rot; (3,1), (3,2) grün |
| 19:50 | 16 + 3            | 48 + 2            | (3,1), (1,3) rot; (1,1), (2,3) grün |
| 21:21 | 16 + 4 + 1        | 16 + 4 + 1        | (3,1), (3,2), (3,3) gelb            |


---

## 🎨 Farbvarianten (optional)


| Kombination       | Farbe  | Bedeutung                      |
| ----------------- | ------ | ------------------------------ |
| Stunden           | 🔴 Rot  | Stunden                        |
| Minuten           | 🟢 Grün | Minuten                        |
| Stunden + Minuten | 🟡 Gelb | Beide                          |
| Sekunden          | 🔵 Blau | Sekunden (falls implementiert) |
| Status            | ⚪ Weiß | WLAN/Error-Status              |


---

## 🔍 Fehlerbehebung


| Problem                       | Lösung                                       |
| ----------------------------- | -------------------------------------------- |
| LEDs leuchten nicht           | Stromversorgung prüfen, Datenleitung prüfen  |
| TM1637 zeigt nichts an        | CLK/DIO-Verbindung prüfen, Spannung prüfen   |
| Falsche Zeit                  | Internetverbindung prüfen, NTP-Server prüfen |
| Falsche Farben                | LED-Typ prüfen (RGB vs. GRB), Code anpassen  |
| Mikrocontroller startet nicht | USB-Verbindung prüfen, Bootloader prüfen     |


---

## 📝 Changelog

- **14.09.2026:** Erstversion - Projektbeschreibung, Code für RasPi und ESP32

---

## 💡 Ideen für Erweiterungen

1. **Sekunden-Anzeige:** zusätzlicher Ball oder Farbwechsel
2. **Datum-Anzeige:** auf TM1637 umschalten
3. **Alarm-Funktion:** spezielle LED-Muster
4. **Temperatur-Anzeige:** zusätzliche Sensoren
5. **Web-Interface:** Steuerung über Browser
6. **Sprachsteuerung:** "Uhr, zeige die Zeit"
7. **Mehrere Zeitzonen:** Umschaltung per Knopfdruck
8. **Animationen:** beim Stundenwechsel

---

## 📚 Quellen &amp; Referenzen

- [WS2812B Neopixel Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
- [TM1637 Datasheet](https://www.mikroe.com/glcd/4-digit-led-display)
- [Raspberry Pi GPIO Pinout](https://pinout.xyz/)
- [ESP32S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32s3_datasheet_en.pdf)
- [NTP Zeit-Synchronisation](https://de.wikipedia.org/wiki/Network_Time_Protocol)

---

## 📞 Support

Bei Fragen oder Problemen:

- Projekt-Repository: [GitHub - ping-pong-ball-clock](https://github.com/Paul-3400/ping-pong-ball-clock)
- Autor: Paul Simmen ([paul.simmen@bluewin.ch](mailto:paul.simmen@bluewin.ch))

---

**Viel Spaß beim Bauen!** 🎉