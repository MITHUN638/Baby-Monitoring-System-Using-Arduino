#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHT_PIN       2
#define SOUND_PIN     3
#define BUZZER_PIN    8
#define LED_PIN       9

#define DHT_TYPE      DHT11

#define TEMP_HIGH     22.0
#define TEMP_LOW      18.0
#define HUM_HIGH      60
#define HUM_LOW       40

#define CRY_CONFIRM   3

#define READ_INTERVAL 2000
#define BUZZER_FREQ   1000
#define BUZZER_DUR    300

DHT dht(DHT_PIN, DHT_TYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

int           cryCount   = 0;
bool          alertActive = false;
unsigned long lastReadMs  = 0;

byte degreeChar[8] = {
  0b00110, 0b01001, 0b01001, 0b00110,
  0b00000, 0b00000, 0b00000, 0b00000
};

void displayStatus(float temp, float hum, bool crying);
void triggerAlert(bool active);
String buildTempHumString(float temp, float hum);

void setup() {
  Serial.begin(9600);
  Serial.println(F("=== Arduino Baby Monitor ==="));
  Serial.println(F("Initialising sensors..."));


  pinMode(SOUND_PIN,  INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN,    OUTPUT);


  dht.begin();


  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degreeChar);
  lcd.setCursor(0, 0);
  lcd.print(F("Baby Monitor v1 "));
  lcd.setCursor(0, 1);
  lcd.print(F("Starting up...  "));
  delay(2000);
  lcd.clear();

  Serial.println(F("Ready. Monitoring started."));
}

void loop() {
  unsigned long now = millis();


  if (now - lastReadMs >= READ_INTERVAL) {
    lastReadMs = now;


    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();


    if (isnan(temp) || isnan(hum)) {
      Serial.println(F("ERROR: DHT11 read failed. Check wiring."));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Sensor error!   "));
      lcd.setCursor(0, 1);
      lcd.print(F("Check DHT wiring"));
      return;
    }


    int soundLevel = digitalRead(SOUND_PIN);
    if (soundLevel == HIGH) {
      cryCount = min(cryCount + 1, 10);
    } else {
      cryCount = max(cryCount - 1, 0);
    }
    alertActive = (cryCount >= CRY_CONFIRM);


    displayStatus(temp, hum, alertActive);
    triggerAlert(alertActive);


    Serial.print(F("Temp: "));   Serial.print(temp, 1); Serial.print(F(" C  |  "));
    Serial.print(F("Hum: "));    Serial.print(hum,  0); Serial.print(F(" %  |  "));
    Serial.print(F("Sound: "));  Serial.println(alertActive ? F("CRYING") : F("quiet"));
  }
}

void displayStatus(float temp, float hum, bool crying) {

  lcd.setCursor(0, 0);
  lcd.print(F("T:"));
  lcd.print(temp, 1);
  lcd.write(0);
  lcd.print(F("C H:"));
  lcd.print((int)round(hum));
  lcd.print(F("%   "));


  lcd.setCursor(0, 1);
  if (crying) {
    lcd.print(F("!! BABY CRYING !!"));
  } else if (temp > TEMP_HIGH) {
    lcd.print(F("ROOM TOO HOT!   "));
  } else if (temp < TEMP_LOW) {
    lcd.print(F("ROOM TOO COLD!  "));
  } else if (hum > HUM_HIGH) {
    lcd.print(F("TOO HUMID       "));
  } else if (hum < HUM_LOW) {
    lcd.print(F("TOO DRY         "));
  } else {
    lcd.print(F("Room OK  :)     "));
  }
}

void triggerAlert(bool active) {
  if (active) {
    tone(BUZZER_PIN, BUZZER_FREQ, BUZZER_DUR);
    digitalWrite(LED_PIN, HIGH);
  } else {
    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, LOW);
  }
}
