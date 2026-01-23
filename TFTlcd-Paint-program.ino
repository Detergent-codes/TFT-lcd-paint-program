/*
 Advnced paint program for a 2.4 inch tft panel shield specifically
 designed for the Arduino UNO R4.

 And i know this code can be a lot more optimized for this use case,
 but im new to arduino and i tried my best.

 Code by Aalok
*/

#include <Arduino_GFX_Library.h>
#include <TouchScreen.h>

//Configure Settings
 #define brushSize   3               //Thickness of the drawn line
 #define canvasColour   BLACK        //Colour of the drawing canvas
 #define contrastColour   WHITE      //Colour of text, lines, etc.Must be different than background colour
 #define seperatorProtectMargin   33 //Makes sure that the seperator line does not get painted over, Keep 33 for brush sizes 3 or under 3
 #define clearTimerLimit   2000        //Time in milliseconds before the "Sure?" button resets
 #define sureMessageColour   RED     //Colour of the confirmation message after clicking the clear canvas button

// Color Definitions
 #define BLACK   0x0000
 #define WHITE   0xFFFF
 #define RED     0xF800
 #define ORANGE  0xFDA0
 #define YELLOW  0xFFE0
 #define GREEN   0x07E0
 #define BLUE    0x001F
// Display Setup
 Arduino_DataBus *bus = new Arduino_UNOPAR8();  
 Arduino_GFX *gfx = new Arduino_ILI9486(bus, DF_GFX_RST);  //same as ILI9341 but more vibrant colours

// Touch Pins (Using your verified pins: 6, A2, A1, 7)
 const int XP=6, XM=A2, YP=A1, YM=7; 
 TouchScreen ts = TouchScreen(XP, YP, XM, YM, 280);

//Default colour setting
 int currentColor = RED;
 int boxW = 32; // Palette box width

//BUTTON LOGIC VARIABLES
 bool confirmClear = false;      
 uint32_t clearTimer = 0;        

void drawClearButton(uint16_t color) {
  gfx->fillRect(261, 1, 58, 28, canvasColour); // Clear button interior
  gfx->drawRect(260, 0, 60, 30, color);        // Draw border
  gfx->setTextColor(color);
  gfx->setTextSize(1);
  if (confirmClear) {
    gfx->setCursor(269, 7);  gfx->print("Are You");
    gfx->setCursor(275, 16); gfx->print("Sure?");
  } else {
    gfx->setCursor(275, 7);  gfx->print("Clear");
    gfx->setCursor(272, 16); gfx->print("Canvas");
  }
}

void setup() {
  pinMode(A4, OUTPUT);
  digitalWrite(A4, LOW); 
  delay(1);
  digitalWrite(A4, HIGH);
  delay(1);

  gfx->begin();
  gfx->setRotation(1); // Landscape
  gfx->fillScreen(canvasColour);
  gfx->drawRect(0 ,0 , 320, 240 , WHITE);

  gfx->setTextSize(1);
  gfx->setTextColor(contrastColour);
  gfx->setCursor(120, 225);
  gfx->print("Code by Aalok");

  gfx->setTextSize(3);
  gfx->setTextColor(RED);
  gfx->setCursor(70, 86);
  gfx->print("Welcome To");

  gfx->setTextSize(5);
  gfx->setTextColor(contrastColour);
  gfx->setCursor(71, 114);
  gfx->print("CANVAS");

  delay(3000);
  gfx->fillScreen(canvasColour);

  //7-COLOR PALETTE
  gfx->fillRect(0 * boxW, 0, boxW, 30, RED);
  gfx->fillRect(1 * boxW, 0, boxW, 30, ORANGE);
  gfx->fillRect(2 * boxW, 0, boxW, 30, YELLOW);
  gfx->fillRect(3 * boxW, 0, boxW, 30, GREEN);
  gfx->fillRect(4 * boxW, 0, boxW, 30, BLUE);
  
  // Black Box (With white outline so you can see it)
  gfx->drawRect(5 * boxW, 0, boxW, 30, WHITE);
  gfx->fillRect(5 * boxW + 1, 1, boxW - 2, 28, BLACK);

  // White Box (With black outline so you can see it)
  gfx->drawRect(6 * boxW, 0, boxW, 30, BLACK);
  gfx->fillRect(6 * boxW + 1, 1, boxW - 2, 28, WHITE);

  // Clear Canvas Button
  drawClearButton(contrastColour);

  // Palette Separator Line
  gfx->drawFastHLine(0, 30, 320, contrastColour);
}

void loop() {
  // --- 3-SECOND TIMEOUT CHECK ---
  // We move this to the very top so it checks even if no touch is detected
  if (confirmClear && (millis() - clearTimer >= clearTimerLimit)) {
    confirmClear = false;
    drawClearButton(contrastColour);
  }

  TSPoint p = ts.getPoint();

  // MANDATORY R4 FIX: Restore display pins
  pinMode(6, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(7, OUTPUT);

  if (p.z > 50 && p.z < 1000) {
    // --- YOUR CUSTOM CALIBRATION MAPPING ---
    int x = map(p.x, 928, 215, 0, 320); //X axis
    int y = map(p.y, 202, 880, 0, 240); //Y axis

    // Palette Logic
    if (y < 31) {
      if (x < boxW)           currentColor = RED;
      else if (x < boxW * 2)  currentColor = ORANGE;
      else if (x < boxW * 3)  currentColor = YELLOW;
      else if (x < boxW * 4)  currentColor = GREEN;
      else if (x < boxW * 5)  currentColor = BLUE;
      else if (x < boxW * 6)  currentColor = BLACK;
      else if (x < boxW * 7)  currentColor = WHITE;
      else if (x >= 260) {
        if (!confirmClear) {
          confirmClear = true;
          clearTimer = millis(); 
          drawClearButton(sureMessageColour); 
          delay(200); // Debounce to prevent instant double-trigger
        } else {
          gfx->fillRect(0, 31, 320, 209, canvasColour); 
          confirmClear = false;
          drawClearButton(contrastColour);
        }
      }
    }

    else if (y > seperatorProtectMargin) {
      // Cancel the "Sure?" prompt if the user decides to draw instead
      if (confirmClear) {
        confirmClear = false;
        drawClearButton(contrastColour);
      }

        // First touch point: draw a small filled square
        gfx->fillRect(x - brushSize, y - brushSize, brushSize, brushSize, currentColor);
    }
  }
}
