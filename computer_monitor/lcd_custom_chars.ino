const int LCD_CUSTOM_CHARS_COUNT = 1;

byte LCD_CUSTOM_CHARS[LCD_CUSTOM_CHARS_COUNT][8] = {
  {
    // filled square
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B00000
  }
};

void createCustomLCDChars() {
  for (byte i = 0; i < LCD_CUSTOM_CHARS_COUNT; i++) {
    lcd.createChar(i, LCD_CUSTOM_CHARS[i]);
  }
}
