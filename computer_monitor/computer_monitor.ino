#include <LiquidCrystal.h>

// Connections to the circuit: LCD screen
const int LCD_RS_PIN = 12;
const int LCD_ENABLE_PIN = 11;
const int LCD_DATA_PIN_4 = 4;
const int LCD_DATA_PIN_5 = 5;
const int LCD_DATA_PIN_6 = 6;
const int LCD_DATA_PIN_7 = 7;
const int LCD_ROWS = 2;
const int LCD_COLS = 16;

// Connect it to HDD LED-. This can be changed to any pin that works with digitalRead().
const int HDD_LED_PIN = 3;

// Indirectly connected to CPU fan sense (green) wire. Need interrupt-compatible pin.
const int CPU_FAN_SENSE_PIN = 2;

// Connect to CPU fan control (blue) wire.
const int CPU_FAN_CONTROL_PIN = A5;

// TMP36 temperature sensor. Needs analog input pin.
const int TEMPERATURE_SENSOR_PIN = A0;

LiquidCrystal lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_DATA_PIN_4, LCD_DATA_PIN_5, LCD_DATA_PIN_6, LCD_DATA_PIN_7);

unsigned long previousReportTime = millis();
bool needRender = false;

// HDD activity
byte hddIsActiveLastState = 0;
unsigned long hddIsActiveLastStateTime = millis();
unsigned long hddActiveTimeSinceLastReport = 0;
unsigned long hddDisplayedActiveTime = 0;
byte hddIsActive = 0;

// Temperature is computed by averaging successive measures
const int N_TEMP_MEASURES = 5;
byte temperatureMeasures[N_TEMP_MEASURES];
byte temperatureMeasuresIndex = 0;
int displayedTemperature = 0;

// Fan speed
unsigned int fanSpeed = 0;
volatile byte fanSpeedCounter = 0;

// Fan control
byte fanControlPWM = 0; // %

// debug
unsigned long loopSpeedCounter = 0;

extern void createCustomLCDChars();

void setup() {
  Serial.begin(115200);

  // Use a pull-up resistor to make clear variations between 5V and 0V,
  // instead of variations between somewhere around 3V and 0V.
  pinMode(HDD_LED_PIN, INPUT_PULLUP);
  
  pinMode(CPU_FAN_SENSE_PIN, INPUT);

  lcd.begin(LCD_COLS, LCD_ROWS);

  createCustomLCDChars();

  attachInterrupt(digitalPinToInterrupt(CPU_FAN_SENSE_PIN), fanSenseISR, RISING);
}

void fanSenseISR() {
  fanSpeedCounter++;
}

// Prints a number padded with spaces on the left
void lcdPrintWithPadding(int value, int numberOfChars) {
  int ref = 10;
  for (byte i = 1; i < numberOfChars; i++) {
    if (value < ref) {
      lcd.write(' ');
    }
    ref *= 10;
  }
  lcd.print(value);
}

void printPercentage(byte value) {
  lcdPrintWithPadding(value, 3);
  lcd.write('%');
}

void render() {
  // Show HDD activity %
  lcd.home();
  lcd.write("HDD");
  printPercentage(hddDisplayedActiveTime / 10);

  // display equivalent of HDD LED
  if (hddIsActive) {
    lcd.write((byte) 0); // custom chracter: filled square
  } else {
    lcd.write(' ');
  }

  // display temperature
  lcd.setCursor(LCD_COLS - 4, 0);
  lcdPrintWithPadding(displayedTemperature, 2);
  lcd.write(0xDF); // 0xDF is ° in HD44780
  lcd.print("C");

  // display fan control
  lcd.setCursor(0, 1);
  lcd.print("Fan");
  printPercentage(fanControlPWM);

  // display fan speed
  lcd.setCursor(7, 1);
  lcdPrintWithPadding(fanSpeed, 5);
  lcd.print(" RPM");
}

void loop() {
  unsigned long now = millis();

  // Measure HDD activity
  hddIsActive = digitalRead(HDD_LED_PIN) == LOW;
  if (hddIsActiveLastState != hddIsActive) {
    // We have changed from active to inactive or the opposite
    if (! hddIsActive) {
      // We have reached the end of an activity interval => increment activity time
      hddActiveTimeSinceLastReport += now - hddIsActiveLastStateTime;
    }
    hddIsActiveLastStateTime = now;
    hddIsActiveLastState = hddIsActive;
    needRender = true; // render need to display HDD LED state
  }

  // Report the measurements on the LCD screen
  if (now >= previousReportTime + 1000) {
    // Add one temperature measure to array
    temperatureMeasures[temperatureMeasuresIndex] = (analogRead(TEMPERATURE_SENSOR_PIN) / 1024.0 * 5.0 - 0.5) * 100;
    temperatureMeasuresIndex = (temperatureMeasuresIndex + 1) % N_TEMP_MEASURES;

    // Finish computing the current HDD activity measure
    if (hddIsActive) {
      // We are currently active, so add time since activity started to active time.
      hddActiveTimeSinceLastReport += now - hddIsActiveLastStateTime;
    }
    // Send HDD activity to LCD and reset counter
    hddDisplayedActiveTime = hddActiveTimeSinceLastReport;
    hddIsActiveLastStateTime = now;
    hddActiveTimeSinceLastReport = 0;

    // Send fan speed to LCD and reset counter
    fanSpeed = fanSpeedCounter * 30;
    fanSpeedCounter = 0;

    // Send temperature to LCD
    displayedTemperature = 0;
    for (byte i = 0; i < N_TEMP_MEASURES; i++) {
      displayedTemperature += temperatureMeasures[i];
    }
    displayedTemperature /= N_TEMP_MEASURES;

    // to debug performance
    Serial.print(loopSpeedCounter);
    Serial.println(" loops per second");
    loopSpeedCounter = 0;

    // Read fan control value (which is PWM)
    unsigned long positiveLength = pulseIn(CPU_FAN_CONTROL_PIN, HIGH, 200);
    unsigned long negativeLength = pulseIn(CPU_FAN_CONTROL_PIN, LOW, 200);
    if (positiveLength + negativeLength == 0) {
      // No pulse, so we must be at 0% or 100%
      fanControlPWM = digitalRead(CPU_FAN_CONTROL_PIN) == HIGH ? 100 : 0;
    } else {
      fanControlPWM = positiveLength * 100 / (positiveLength + negativeLength);
    }

    previousReportTime = now;
    needRender = true;
  }

  if (needRender) {
    render();
    needRender = false;
  }

  loopSpeedCounter++;
}
