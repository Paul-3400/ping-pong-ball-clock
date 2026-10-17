# Ping-Pong-Ball Uhr - Projektbeschreibung

**Autor:** Paul Simmen mit Vibe (GLM-5-latest-short), 14.09.2026 - aktualisiert 17.10.2026

## 📌 Projektübersicht

Eine kreative Uhr, die die Zeit mit 9 Ping-Pong-Bällen (40mm Durchmesser) anzeigt. Jeder Ball kann in verschiedenen Farben leuchten (Rot, Grün, Blau) und repräsentiert einen Zeitwert. Die Basis der Anzeige ist die Zahl **4**, wobei die Werte in einer 3x3-Matrix angeordnet sind.

Zusätzlich zur Ping-Pong-Ball-Anzeige wird die Zeit auf einer **TM1637 6-Zahlen 7-Segment-Anzeige** (0,36 Zoll) angezeigt.

**Status (17.10.2026):** Die komplette Anzeigelogik ist als Prototyp in Wokwi getestet (Seeed XIAO ESP32-S3). Der getestete Code liegt im Ordner [`wokwi/`](wokwi/) - Details siehe Abschnitt «WokWi-Prototyp-Test» weiter unten. Die Speisung ist ausgelegt: **Netzteil 5V/3A mit Schottky-Diode vor dem 5V-Pin** (Abschnitt «Stromversorgung»).

### 🎯 Ziele

- Zeitdarstellung mit Ping-Pong-Bällen
- Internet-Zeitsynchronisation (NTP)
- Steuerung mit Raspberry Pi Zero WH oder Seeed XIAO ESP32-S3
- 5V/3A Stromversorgung
- Klare visuelle Anzeige mit Farben

---

## 📦 Materialliste

| Komponente        | Menge | Beschreibung                                   |
| ----------------- | ----- | ---------------------------------------------- |
| Ping-Pong-Bälle   | 9     | 40mm Durchmesser, Standard-Tischtennisbälle    |
| RGB-LEDs          | 9     | WS2812B Neopixel (individuell ansteuerbar)     |
| Mikrocontroller   | 1     | Raspberry Pi Zero WH **oder** XIAO ESP32-S3    |
| 7-Segment-Anzeige | 1     | TM1637 6-Zahlen, 0,36 Zoll                     |
| Taster            | 3     | Stunde+, Minute+, Demo/Uhrzeit stellen         |
| Netzteil          | 1     | 5V, 3A                                         |
| Schottky-Diode   | 1     | 1N5819 oder SS14 (USB-Rückstrom-Schutz)       |
| Elko             | 1     | 470-1000µF an der 5V-Schiene (LED-Stabilität) |
| Gehäuse           | 1     | Für mechanische Fixierung der Bälle            |
| Verdrahtung       | -     | Jumper-Kabel, Lötmaterial                      |
| Widerstände       | 1-2   | 220-470Ω für LED-Strombegrenzung (falls nötig) |
| Breadboard        | 1     | Für Prototypenaufbau                           |

---

## 🔢 Matrix-Anordnung & Zeitlogik

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

**Stunden (0-23):** Werte: {16, 12, 8, 4, 3, 2, 1} - Beispiel: 19h = 16 + 3, 15h = 12 + 3

**Minuten (0-59):** Werte: {48, 32, 16, 8, 4, 3, 2, 1} - Beispiel: 50m = 48 + 2, 47m = 32 + 8 + 4 + 3

> **Korrektur vom 14.10.2026:** Die ursprünglichen Wertemengen ({1,2,3,4,16} für Stunden, {1,2,3,4,8,16,48} für Minuten) hatten Lücken: **Stunden 11-15** und **Minuten 35-47** waren nicht darstellbar (z.B. gab es keine Kombination für 40 Minuten; 14h hätte faktisch als 10h geleuchtet). Mit den vollständigen Mengen (Stunden zusätzlich 12 und 8, Minuten zusätzlich 32) sind alle 24 Stunden und 60 Minuten darstellbar - und **alle ursprünglichen Beispiele zeigen exakt die gleichen Bälle wie vorher**. Rechnerisch verifiziert und im Wokwi-Prototyp getestet.

### Farbkodierung

| Farbe  | Bedeutung                        |
| ------ | -------------------------------- |
| 🔴 Rot  | Stundenkomponente                |
| 🟢 Grün | Minutenkomponente                |
| 🔵 Blau | (optional: Sekunden oder Status) |
| 🟡 Gelb | Stunden + Minuten                |

**Beispiele:**

- **6:00** → 4h + 2h → Bälle (3,2)=4 und (2,3)=2 leuchten **rot**
- **6:11** → 4h + 2h + 8m + 3m → (3,2)=4, (2,3)=2 (rot), (2,2)=8, (1,3)=3 (grün)
- **6:20** → 4h + 2h + 16m + 4m → (2,3)=2 (rot), (3,1)=16 (grün), (3,2)=4 (**gelb** - Stunde und Minute am gleichen Ball)
- **21:21** → 16h + 4h + 1h + 16m + 4m + 1m → Bälle (3,1)=16, (3,2)=4, (3,3)=1 leuchten **gelb**
- **11:47** → 8h + 3h + 32m + 8m + 4m + 3m → (2,2)=8, (1,3)=3 (**gelb**), (2,1)=32, (3,2)=4 (grün) - *diese Zeit ist erst mit der Korrektur darstellbar*

---

## 🔌 Schaltplan

### Pin-Belegung

| Komponente    | Raspberry Pi Zero WH | ESP32-S3 (DevKit) | XIAO ESP32-S3                |
| ------------- | -------------------- | ----------------- | ---------------------------- |
| TM1637 CLK    | GPIO 5 (Pin 29)      | GPIO 5            | GPIO 5 = **D4**              |
| TM1637 DIO    | GPIO 6 (Pin 31)      | GPIO 6            | GPIO 6 = **D5**              |
| Neopixel DATA | GPIO 18 (Pin 12)     | GPIO 18           | GPIO 4 = **D3** (GPIO18 ist am XIAO nicht herausgeführt) |
| Taster Std+   | -                    | GPIO 1            | **D0** (GPIO1)               |
| Taster Min+   | -                    | GPIO 2            | **D1** (GPIO2)               |
| Taster Demo   | -                    | GPIO 3            | **D2** (GPIO3, Strapping-Pin: am echten Board beim Reset nicht gedrückt halten) |
| 5V            | Pin 2 oder 4         | 5V Pin            | 5V                           |
| GND           | Pin 6, 9, 14, etc.   | GND Pin           | GND                          |

### Verbindungsschema

```
Netzteil 5V/3A (+) ──[Schottky-Diode 1N5819, Anode zum Netzteil]──► 5V-Pin
Netzteil (-) ──────────────────────────────────────────────► gemeinsame GND-Schiene

┌─────────────────────────────────────────────────────────────┐
│                    Mikrocontroller (RasPi/XIAO ESP32-S3)      │
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
                              │           ├── TM1637 CLK         │
                              │           ├── TM1637 DIO         │
                              │           └──────────────────────┘
                              │
                              ├── GPIO ──── Neopixel DATA
                              │
                              ├── GPIO ──── Taster Std+ (nach GND, intern Pullup)
                              ├── GPIO ──── Taster Min+ (nach GND)
                              └── GPIO ──── Taster Demo (nach GND)

Verdrahtung der Neopixel: Strom (5V/GND) und Daten als Daisy-Chain von Ball zu Ball (DOUT → DIN des nächsten Balls), wie im Wokwi-Diagramm.
```

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
    """Zerlegt Stunden in Komponenten (vollstaendige Menge, seit 14.10.2026)"""
    components = []
    for value in [16, 12, 8, 4, 3, 2, 1]:
        if h >= value:
            components.append(value)
            h -= value
    return components

def decompose_minutes(m):
    """Zerlegt Minuten in Komponenten (vollstaendige Menge, seit 14.10.2026)"""
    components = []
    for value in [48, 32, 16, 8, 4, 3, 2, 1]:
        if m >= value:
            components.append(value)
            m -= value
    return components

def get_led_states(h, m):
    """Ermittelt, welche LEDs in welcher Farbe leuchten sollen"""
    hour_components = decompose_hours(h)
    minute_components = decompose_minutes(m)

    states = {pos: 0 for pos in MATRIX.keys()}  # 0=aus, 1=rot, 2=gruen, 3=gelb

    for value in hour_components:
        pos = VALUE_TO_POSITION[value]
        states[pos] = states.get(pos, 0) | 1  # Rot setzen

    for value in minute_components:
        pos = VALUE_TO_POSITION[value]
        states[pos] = states.get(pos, 0) | 2  # Gruen setzen

    return states
```

**Getestete Implementierung (Arduino, XIAO ESP32-S3, inkl. NTP, Handbetrieb und Demo-Taster):** siehe [`wokwi/sketch.ino`](wokwi/sketch.ino). Die nachfolgenden Python-Varianten für Raspberry Pi und MicroPython sind entsprechend aktualisiert.

### Code für Raspberry Pi Zero WH (Python)

```python
# ping_pong_clock_rpi.py
# Autor: Paul Simmen mit Vibe (GLM-5-latest-short), 14.10.2026

import time
import datetime
import requests
from rpi_ws281x import PixelStrip, Color

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

def decompose_hours(h):
    """Zerlegt Stunden in Komponenten (vollstaendige Menge, seit 14.10.2026)"""
    components = []
    for value in [16, 12, 8, 4, 3, 2, 1]:
        if h >= value:
            components.append(value)
            h -= value
    return components

def decompose_minutes(m):
    """Zerlegt Minuten in Komponenten (vollstaendige Menge, seit 14.10.2026)"""
    components = []
    for value in [48, 32, 16, 8, 4, 3, 2, 1]:
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
        now = datetime.datetime.now()
        return now.hour, now.minute, now.second

def update_leds(strip, hour_components, minute_components):
    """Aktualisiert die LED-Anzeige (rot=Stunden, gruen=Minuten, gelb=beides)"""
    for i in range(LED_COUNT):
        strip.setPixelColor(i, Color(0, 0, 0))

    for value in hour_components:
        if value in VALUE_TO_POS:
            led_idx = POS_TO_LED[VALUE_TO_POS[value]]
            current = strip.getPixelColor(led_idx)
            strip.setPixelColor(led_idx, Color(255, (current >> 8) & 255, 0))

    for value in minute_components:
        if value in VALUE_TO_POS:
            led_idx = POS_TO_LED[VALUE_TO_POS[value]]
            current = strip.getPixelColor(led_idx)
            strip.setPixelColor(led_idx, Color((current >> 16) & 255, 255, 0))

    strip.show()

def main():
    print("Ping-Pong-Ball Uhr - Raspberry Pi Version")
    print("Autor: Paul Simmen mit Vibe (GLM-5-latest-short)")

    strip = PixelStrip(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS)
    strip.begin()

    try:
        while True:
            h, m, s = get_time_from_internet()
            update_leds(strip, decompose_hours(h), decompose_minutes(m))
            time.sleep(1)
    except KeyboardInterrupt:
        for i in range(LED_COUNT):
            strip.setPixelColor(i, Color(0, 0, 0))
        strip.show()
        print("\nProgramm beendet.")

if __name__ == "__main__":
    main()
```

**Abhängigkeiten für Raspberry Pi:**

```bash
sudo apt-get update
sudo apt-get install python3-pip
pip3 install rpi_ws281x requests
```

---

## 🔬 WokWi-Prototyp-Test (14.10.2026)

**Aufbau (Simulator):** Seeed XIAO ESP32-S3 (`board-xiao-esp32-s3`), 9x `wokwi-neopixel` in der 3x3-Matrix mit Wert-Beschriftung, TM1637-Modul, 3 Taster (Stunde+/Minute+/Demo).

**Ergebnisse:**

- ✅ **Farblogik verifiziert:** rot/grün/gelb gemäss Checkliste, alle Demo-Zeiten korrekt (inkl. der neuen Grenzfälle 11:47, 12:34, 15:47, die erst mit den vollständigen Wertemengen darstellbar sind)
- ✅ **NTP-Zeitsynchronisation funktioniert auch im Browser-Simulator:** Das virtuelle WLAN «Wokwi-GUEST» beantwortet NTP-Anfragen mit der echten Zeit; ein Handbetrieb über die Taster ist als Rückfall implementiert
- ✅ **Daisy-Chain-Verdrahtung:** Strom und Daten laufen von Ball zu Ball (wie später in der echten Uhr); im Diagramm sauber gerastert mit nicht überlappenden Leitungen
- ℹ️ **TM1637:** Wokwi bietet das Modul nur 4-stellig → die Simulation zeigt HH:MM; die echte 6-stellige Anzeige ergänzt später noch SS (Code-Stelle ist markiert)

**Dateien:** [`wokwi/sketch.ino`](wokwi/sketch.ino) · [`wokwi/diagram.json`](wokwi/diagram.json) · [`wokwi/libraries.txt`](wokwi/libraries.txt) (Adafruit NeoPixel, TM1637)

**Update 17.10.2026 (diagram.json):** Netzteil-Gruppe ergänzt - Schiebeschalter `swPsu` (steht für den Plus-Pol des Netzteils, Griff links = EIN), Schottky-Diode als Community-Chip `chip-diode` (Dependency `github:drf5n/Wokwi-Chip-Diode@1.0.0`, Pins ANODE/CATHODE) sowie Beschriftungstexte. Ausserdem GND-Kette vervollständigt (np5→np6) und Titel-Text-Encoding repariert. In der Simulation bleibt der Netzteil-Zweig stromlos - Wokwi versorgt das Board virtuell über USB.

---

## ⚡ Stromversorgung

### Anforderungen

- **Spannung:** 5V DC
- **Strom:** 3A (für LEDs + Mikrocontroller + Anzeige)

### Berechnung

- Neopixel LEDs: ~60mA pro LED bei voller Helligkeit → 9 × 60mA = 540mA
- TM1637: ~20mA
- Raspberry Pi Zero WH: ~200-500mA / XIAO ESP32-S3: ~100-200mA
- **Gesamt:** ~900mA - 1,2A (3A Netzteil ist ausreichend)

### Verdrahtungstipps

- Neopixel-Strom als Daisy-Chain von Ball zu Ball führen (wie getestet), Daten über DOUT → DIN
- 220-470Ω in der NeoPixel-Datenleitung und 1000µF Kondensator am Netzteilausgang
- Ausreichend dicke Kabel (mind. 20AWG)
- **Beim XIAO ESP32-S3: Schottky-Diode (1N5819/SS14) zwischen Netzteil-Plus und 5V-Pin** - siehe eigener Unterabschnitt unten

### Speisung am XIAO ESP32-S3: Schottky-Diode ist Pflicht (17.10.2026)

Der 5V-Pin des XIAO ESP32-S3 hängt intern direkt an der USB-Betriebsspannung (VBUS) - das Board hat **keine** eingebaute Schutzdiode gegen Rückspeisung. Liegt das Netzteil direkt am 5V-Pin und du steckst gleichzeitig USB an (z.B. zum Umprogrammieren), fliesst Strom vom Netzteil zurück in den USB-Port des Computers - das kann den Port beschädigen («Backfeeding»).

**Lösung:** Schottky-Diode in Serie zwischen Netzteil-Plus und 5V-Pin:

```
Netzteil 5V/3A (+) ──►|[ 1N5819 / SS14 ]|── 5V-Pin XIAO
                    Anode   Kathode (Ring)
```

- **Anode zum Netzteil, Kathode (Ring) zum 5V-Pin** - so darfst du jederzeit USB anstöpseln, während die Uhr am Netzteil läuft
- Spannungsabfall ca. 0,2-0,3 V - unkritisch für den Bordspannungsregler des XIAO
- Netzteil-GND zwingend mit der gemeinsamen GND-Schiene verbinden (USB-GND, XIAO-GND, LEDs, TM1637 - alles eine Masse)
- Im WokWi-Diagramm abgebildet als Schiebeschalter `swPsu` + `chip-diode`; die Diode ist dort ein Community-Chip, der über die `dependencies` in der diagram.json geladen wird

Quelle: [Seeed Studio Wiki - XIAO ESP32-S3 (Empfehlung: Diode zwischen externer Quelle und 5V-Pin)](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)

---

## 📐 Mechanische Konstruktion

### Gehäuse-Design

1. **Ball-Halterung:** 3x3 Raster mit 40mm Abstand (Ping-Pong-Ball-Durchmesser). Jeder Ball wird von einer LED von unten beleuchtet. Material: 3D-gedruckt oder aus Acryl.
2. **Abmessungen:** Gesamtgrösse ~120mm × 120mm × 50mm (Höhe); Ball-Positionen: 20mm von der Unterkante (für LED-Platzierung)
3. **Raster:**
```
 +---------------------+
 |   O   O   O         |  (O = Ping-Pong-Ball)
 |   O   O   O         |
 |   O   O   O         |
 +---------------------+
```

### Materialempfehlungen

- **Gehäuse:** PLA oder PETG (3D-Druck)
- **Diffusor:** Transluzentes Acryl über den Bällen
- **Befestigung:** Schrauben oder Kleber

---

## 🔧 Aufbauanleitung

1. **Hardware vorbereiten:** Bälle vorbereiten (evtl. kleine Löcher für Lichtdurchlass), LEDs in der Halterung positionieren, Mikrocontroller und TM1637 auf dem Breadboard platzieren
2. **Verdrahtung:** TM1637 (CLK/DIO/VCC/GND), Neopixels als Daisy-Chain (Strom von Ball zu Ball, DOUT → DIN), Stromversorgung
3. **Software:** Anzeigelogik gemäss oben; getestete Arduino-Version siehe `wokwi/sketch.ino`
4. **Testen:** Einzeltest jeder LED, TM1637-Anzeige, Zeitzerlegung, komplettes System

---

## 📊 Zeitzerlegungs-Beispiele

| Zeit  | Stunden-Zerlegung | Minuten-Zerlegung | Aktive Bälle                                        |
| ----- | ----------------- | ----------------- | --------------------------------------------------- |
| 00:00 | -                 | -                 | Keine                                               |
| 01:00 | 1                 | -                 | (3,3) rot                                           |
| 06:00 | 4 + 2             | -                 | (3,2), (2,3) rot                                    |
| 06:11 | 4 + 2             | 8 + 3             | (3,2), (2,3) rot; (2,2), (1,3) grün                 |
| 06:20 | 4 + 2             | 16 + 4            | (2,3) rot; (3,1) grün; (3,2) **gelb**               |
| 19:50 | 16 + 3            | 48 + 2            | (3,1), (1,3) rot; (1,1), (2,3) grün                 |
| 21:21 | 16 + 4 + 1        | 16 + 4 + 1        | (3,1), (3,2), (3,3) gelb                            |
| 11:47 | 8 + 3             | 32 + 8 + 4 + 3    | (2,2), (1,3) gelb; (2,1), (3,2) grün |
| 12:34 | 12                | 32 + 2            | (1,2) rot; (2,1), (2,3) grün                        |
| 15:47 | 12 + 3            | 32 + 8 + 4 + 3    | (1,3) gelb; (1,2) rot; (2,1), (2,2), (3,2) grün |

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

| Problem                       | Lösung                                        |
| ----------------------------- | --------------------------------------------- |
| LEDs leuchten nicht           | Stromversorgung prüfen, Datenleitung prüfen  |
| TM1637 zeigt nichts an        | CLK/DIO-Verbindung prüfen, Spannung prüfen    |
| Falsche Zeit                  | Internetverbindung prüfen, NTP-Server prüfen  |
| Falsche Farben                | LED-Typ prüfen (RGB vs. GRB), Code anpassen   |
| Mikrocontroller startet nicht | USB-Verbindung prüfen, Bootloader prüfen      |
| WokWi: «undefined reference to setup/loop» | sketch.ino wurde unvollständig eingefügt - Datei komplett übernehmen (endet mit `)...` bzw. `}`) |

---

## 📝 Changelog

- **14.09.2026:** Erstversion - Projektbeschreibung, Code für RasPi und ESP32
- **14.10.2026:** Wertemengen der Zeitzerlegung vervollständigt (Lücken behoben: Stunden 11-15, Minuten 35-47); Pin-Belegung für Seeed XIAO ESP32-S3 ergänzt (GPIO18 → D3/GPIO4, Taster D0/D1/D2); WokWi-Prototyp-Test erfolgreich, Ordner `wokwi/` hinzugefügt; Beispiel 06:20 korrigiert ((3,2) leuchtet gelb, nicht grün)
- **17.10.2026:** Speisung ergänzt und ausgelegt: Netzteil 5V/3A mit Schottky-Diode (1N5819/SS14) vor dem XIAO-5V-Pin (USB-Rückstrom-Schutz, siehe Abschnitt «Stromversorgung»); WokWi-diagram.json um Netzteil-Gruppe + Dioden-Chip (chip-diode-Dependency) erweitert und GND-Kette np5→np6 vervollständigt

---

## 💡 Ideen für Erweiterungen

1. **Sekunden-Anzeige:** 6-stellige TM1637 zeigt HHMMSS
2. **Datum-Anzeige:** auf TM1637 umschalten
3. **Alarm-Funktion:** spezielle LED-Muster
4. **Temperatur-Anzeige:** zusätzliche Sensoren
5. **Web-Interface:** Steuerung über Browser
6. **Mehrere Zeitzonen:** Umschaltung per Knopfdruck
7. **Animationen:** beim Stundenwechsel

---

## 📚 Quellen & Referenzen

- [WS2812B Neopixel Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
- [TM1637 Datasheet](https://www.mikroe.com/glcd/4-digit-led-display)
- [Raspberry Pi GPIO Pinout](https://pinout.xyz/)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32s3_datasheet_en.pdf)
- [NTP Zeit-Synchronisation](https://de.wikipedia.org/wiki/Network_Time_Protocol)
- [Wokwi: diagram.json-Format (Verbindungen, Draht-Führung)](https://docs.wokwi.com/diagram-format)
- [Wokwi: TM1637-Modul](https://docs.wokwi.com/parts/wokwi-tm1637-7segment)
- [Wokwi: NeoPixel-Modul](https://docs.wokwi.com/parts/wokwi-neopixel)
- [Seeed Studio Wiki - XIAO ESP32-S3 Getting Started (Speisung über den 5V-Pin)](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- [Wokwi-Chip-Diode - Community-Custom-Chip (MIT)](https://github.com/drf5n/Wokwi-Chip-Diode)

---

## 📞 Support

Bei Fragen oder Problemen:

- Projekt-Repository: [GitHub - ping-pong-ball-clock](https://github.com/Paul-3400/ping-pong-ball-clock)
- Autor: Paul Simmen ([paul.simmen@bluewin.ch](mailto:paul.simmen@bluewin.ch))

---

**Viel Spaß beim Bauen!** 🎉
