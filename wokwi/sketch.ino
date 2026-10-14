/*
  Ping-Pong-Ball-Uhr - WokWi-Testsketch (Seeed XIAO ESP32-S3)
  Autor: Paul Simmen mit Vibe (GLM-5-latest-short), 14.10.2026

  Testet die komplette Anzeigelogik der Projektbeschreibung:
  - 9x WS2812B (NeoPixel) in 3x3-Matrix, Werte wie in der Beschreibung
  - TM1637-7-Segment-Anzeige (WokWi: 4 Ziffern mit Doppelpunkt,
    echte Hardware: 6 Ziffern - Logik bleibt identisch)
  - NTP-Zeitsynchronisation, Rueckfall auf Handbetrieb, wenn keine
    Zeit ankommt (der WokWi-Simulator beantwortet NTP-Anfragen im
    virtuellen Netz "Wokwi-GUEST" meist mit der echten Zeit)
  - Taster D0 = Stunde+, D1 = Minute+, D2 = Demo-Zeiten

  Farbkodierung analog Projektbeschreibung:
    Rot   = Stundenkomponente
    Gruen = Minutenkomponente
    Gelb  = Stunde + Minute am gleichen Ball

  Zeitzerlegung - Hinweis:
    Die Wertemengen der Beschreibung (h: {16,4,3,2,1}, m: {48,16,8,4,3,2,1})
    haben Luecken: Stunden 11-15 und Minuten 35-47 sind nicht darstellbar.
    Dieser Sketch nutzt die vollstaendigen Mengen
      h: {16,12,8,4,3,2,1}   m: {48,32,16,8,4,3,2,1}
    Alle Beispiele der Beschreibung ergeben exakt die gleichen Baelle,
    zusaetzlich sind alle 24 Stunden und 60 Minuten darstellbar.
*/

#include <Adafruit_NeoPixel.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <time.h>

// ---------- Pins (XIAO ESP32-S3, GPIO-Nummern) ----------
#define PIN_LED_DATA 4   // D3 - NeoPixel-Daten (GPIO18 der Beschreibung ist am XIAO nicht herausgefuehrt)
#define PIN_TM_CLK   5   // D4 - wie Projektbeschreibung (GPIO5)
#define PIN_TM_DIO   6   // D5 - wie Projektbeschreibung (GPIO6)
#define PIN_BTN_H    1   // D0 - Stunde+
#define PIN_BTN_M    2   // D1 - Minute+
#define PIN_BTN_DEMO 3   // D2 - Demo-Zeiten (GPIO3 = Strapping-Pin: am echten XIAO beim Reset nicht gedrueckt halten)

#define LED_COUNT 9
#define LED_BRIGHTNESS 60   // 0..255 - fuer die echten Baelle spaeter anpassen

// Zeitzone Schweiz (CET/CEST, Sommerzeitumschaltung wie in Europa)
const char* TZ_STR = "CET-1CEST,M3.5.0,M10.5.0/3";

// ---------- Matrix-Werte (grosse Werte zuerst, greedy) ----------
const uint8_t HOUR_VALUES[]   = {16, 12, 8, 4, 3, 2, 1};
const uint8_t MINUTE_VALUES[] = {48, 32, 16, 8, 4, 3, 2, 1};

// Ball-Wert -> LED-Index der Kette (np1..np9, zeilenweise wie in der Matrix)
// Zeile 1: 48, 12, 3 | Zeile 2: 32, 8, 2 | Zeile 3: 16, 4, 1
int8_t valueToLed(uint8_t v) {
  switch (v) {
    case 48: return 0;
    case 12: return 1;
    case 3:  return 2;
    case 32: return 3;
    case 8:  return 4;
    case 2:  return 5;
    case 16: return 6;
    case 4:  return 7;
    case 1:  return 8;
  }
  return -1;
}

// ---------- Demo-Zeiten (Beispiele der Beschreibung + Luecken-Faelle) ----------
const uint8_t DEMO_TIMES[][2] = {
  { 6,  0}, { 6, 11}, { 6, 20}, {21, 21}, {19, 50},  // Projektbeispiele
  {12, 34}, {11, 47}, {15, 47},                        // nur mit 12/32 darstellbar
  { 0,  0}
};
const uint8_t DEMO_COUNT = sizeof(DEMO_TIMES) / sizeof(DEMO_TIMES[0]);

// ---------- Zustand ----------
enum Mode { NTP_SUCHE, NTP_AKTIV, MANUELL };
Mode modus = NTP_SUCHE;
uint8_t hh = 6, mm = 11, ss = 0;   // Startwert im Handbetrieb (Beispiel 06:11)
uint8_t demoIdx = 0;
unsigned long letzteSekundeMs = 0;
unsigned long ntpStartMs = 0;

Adafruit_NeoPixel pixels(LED_COUNT, PIN_LED_DATA, NEO_GRB + NEO_KHZ800);
TM1637Display display(PIN_TM_CLK, PIN_TM_DIO);

// Bitmasken pro Ball: Bit 0 = Stunde (rot), Bit 1 = Minute (gruen)
uint8_t ballState[LED_COUNT];

// ---------- Zeitzerlegung ----------
void zerlegeZeit(uint8_t h, uint8_t m) {
  for (uint8_t i = 0; i < LED_COUNT; i++) ballState[i] = 0;

  uint8_t rest = h;
  for (uint8_t i = 0; i < sizeof(HOUR_VALUES) && rest > 0; i++) {
    if (rest >= HOUR_VALUES[i]) {
      ballState[valueToLed(HOUR_VALUES[i])] |= 1;
      rest -= HOUR_VALUES[i];
    }
  }
  rest = m;
  for (uint8_t i = 0; i < sizeof(MINUTE_VALUES) && rest > 0; i++) {
    if (rest >= MINUTE_VALUES[i]) {
      ballState[valueToLed(MINUTE_VALUES[i])] |= 2;
      rest -= MINUTE_VALUES[i];
    }
  }
  if (rest > 0) Serial.printf("WARNUNG: Rest %u nicht darstellbar!\n", rest);
}

// ---------- Ausgaben ----------
void zeigeBaelle() {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    uint8_t r = (ballState[i] & 1) ? LED_BRIGHTNESS : 0;
    uint8_t g = (ballState[i] & 2) ? LED_BRIGHTNESS : 0;
    pixels.setPixelColor(i, pixels.Color(r, g, 0));   // rot/gruen/gelb
  }
  pixels.show();
}

void zeigeSegment() {
  // WokWi-Sim: 4 Ziffern HH:MM, Doppelpunkt ueber Bit 7 der 2. Ziffer.
  // Echte 6-stellige Anzeige: hier um SS ergaenzen (z.B. RobTillaart TM1637_RT).
  display.showNumberDecEx(hh * 100 + mm, 0b01000000, true);
}

void druckeStatus() {
  const char* wert[9] = {"48", "12", "3", "32", "8", "2", "16", "4", "1"};
  char rot[24] = "", gruen[24] = "", gelb[24] = "";
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    char buf[6];
    if (ballState[i] == 3)      { snprintf(buf, sizeof(buf), "%s ", wert[i]); strcat(gelb, buf); }
    else if (ballState[i] == 1) { snprintf(buf, sizeof(buf), "%s ", wert[i]); strcat(rot, buf); }
    else if (ballState[i] == 2) { snprintf(buf, sizeof(buf), "%s ", wert[i]); strcat(gruen, buf); }
  }
  const char* m = (modus == NTP_AKTIV) ? "NTP" : (modus == NTP_SUCHE) ? "NTP-Suche" : "Handbetrieb";
  Serial.printf("%02u:%02u:%02u [%s] rot: %s| gruen: %s| gelb: %s\n",
                hh, mm, ss, m, rot, gruen, gelb);
}

// ---------- Taster (Entprellen, wartet auf Loslassen) ----------
bool tasterGedrueckt(uint8_t pin) {
  if (digitalRead(pin) != LOW) return false;
  delay(30);
  if (digitalRead(pin) != LOW) return false;
  while (digitalRead(pin) == LOW) delay(10);
  return true;
}

// ---------- Setup / Loop ----------
void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN_H, INPUT_PULLUP);
  pinMode(PIN_BTN_M, INPUT_PULLUP);
  pinMode(PIN_BTN_DEMO, INPUT_PULLUP);

  pixels.begin();
  pixels.show();                 // alle Baelle aus
  display.setBrightness(5);
  display.clear();

  Serial.println();
  Serial.println("Ping-Pong-Ball-Uhr - WokWi-Test (XIAO ESP32-S3)");
  Serial.println("Suche NTP-Zeit (max. 10 s)...");

  WiFi.begin("Wokwi-GUEST", "");  // virtueller Zugang im Simulator
  configTzTime(TZ_STR, "pool.ntp.org", "time.nist.gov");
  ntpStartMs = millis();
  letzteSekundeMs = millis();
}

void loop() {
  unsigned long jetzt = millis();
  bool sekundeNeu = false;

  // --- Zeitquelle ---
  if (modus == NTP_SUCHE) {
    time_t now = time(nullptr);
    if (now > 1700000000) {                      // gueltige NTP-Zeit erhalten
      struct tm t;
      localtime_r(&now, &t);
      hh = t.tm_hour; mm = t.tm_min; ss = t.tm_sec;
      modus = NTP_AKTIV;
      Serial.println("NTP-Zeit empfangen!");
      sekundeNeu = true;
    } else if (jetzt - ntpStartMs > 10000) {     // Rueckfall Handbetrieb
      modus = MANUELL;
      Serial.println("Keine NTP-Zeit erhalten -> Handbetrieb.");
      Serial.println("Taster: D0 Stunde+, D1 Minute+, D2 Demo-Zeiten.");
      sekundeNeu = true;
    }
  }
  else if (modus == NTP_AKTIV && jetzt - letzteSekundeMs >= 1000) {
    letzteSekundeMs = jetzt;
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    hh = t.tm_hour; mm = t.tm_min; ss = t.tm_sec;
    sekundeNeu = true;
  }
  else if (modus == MANUELL && jetzt - letzteSekundeMs >= 1000) {
    letzteSekundeMs = jetzt;
    if (++ss >= 60) { ss = 0; if (++mm >= 60) { mm = 0; if (++hh >= 24) hh = 0; } }
    sekundeNeu = true;
  }

  // --- Taster ---
  if (tasterGedrueckt(PIN_BTN_H)) {
    if (modus != MANUELL) { modus = MANUELL; letzteSekundeMs = millis(); }
    hh = (hh + 1) % 24; ss = 0;
    sekundeNeu = true;
  }
  if (tasterGedrueckt(PIN_BTN_M)) {
    if (modus != MANUELL) { modus = MANUELL; letzteSekundeMs = millis(); }
    mm = (mm + 1) % 60; ss = 0;
    sekundeNeu = true;
  }
  if (tasterGedrueckt(PIN_BTN_DEMO)) {
    if (modus != MANUELL) { modus = MANUELL; letzteSekundeMs = millis(); }
    demoIdx = (demoIdx + 1) % DEMO_COUNT;
    hh = DEMO_TIMES[demoIdx][0]; mm = DEMO_TIMES[demoIdx][1]; ss = 0;
    Serial.printf("Demo-Zeit %u\n", demoIdx + 1);
    sekundeNeu = true;
  }

  // --- Anzeige einmal pro Sekunde (oder nach Tastendruck) aktualisieren ---
  if (sekundeNeu) {
    zerlegeZeit(hh, mm);
    zeigeBaelle();
    zeigeSegment();
    druckeStatus();
  }
}
