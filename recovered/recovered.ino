#include <Arduboy2.h>

Arduboy2 arduboy;

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(15);
}

void loop() {
  if (!(arduboy.nextFrame()))
    return;
  arduboy.clear();
  arduboy.setCursor(0, 0);
  arduboy.println(F("  ARDUBOY RESTORED"));
  arduboy.println(F("---------------------"));
  arduboy.println(F("Your Arduboy has been"));
  arduboy.println(F("successfully restored"));
  arduboy.println(F("---------------------"));
  arduboy.println(F("You may now upload a"));
  arduboy.println(F("new game!"));
  arduboy.println(F("---------------------"));
  arduboy.display();
}
