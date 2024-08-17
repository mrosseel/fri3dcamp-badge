#include <Arduino.h>
#include <JC_Button.h>    // https://github.com/JChristensen/JC_Button
#include <Adafruit_GFX.h> // Core graphics library
#include <SPI.h>
#include "Adafruit_ST7789_Fri3d2024.h" // Hardware-specific library for ST7789 driver

// Joystick pins
#define PIN_JOY_X 1
#define PIN_JOY_Y 3
#define BUTTON_A 39 // Example pin for button A

// Initialize the display
SPIClass *spi = new SPIClass(HSPI);
Adafruit_GFX_Fri3dBadge2024_TFT tft(spi, TFT_CS, TFT_DC, TFT_RST);

// Initialize buttons
// Button(pin, debounceTime, pullUpEnable, invert)
Button button_A(39, 25, true, true);
Button button_B(40, 25, true, true);
Button button_X(38, 25, true, true);
Button button_Y(41, 25, true, true);
Button button_MENU(45, 25, true, true);
Button button_START(0, 25, false, true); // GPIO0 has HW fixed pullup

// Define grid dimensions for the letters
const int cols = 5;
const int rows = 6;
const char letters[rows][cols] = {
    {'A', 'B', 'C', 'D', 'E'},
    {'F', 'G', 'H', 'I', 'J'},
    {'K', 'L', 'M', 'N', 'O'},
    {'P', 'Q', 'R', 'S', 'T'},
    {'U', 'V', 'W', 'X', 'Y'},
    {'Z', ' ', '.', ',', '-'}};

String state = "setup";
int cursorX = 0;
int cursorY = 0;
int keyBoardYOffset = 60;

// beacuse the display is rotated
int screen_width = TFT_HEIGHT;
int screen_height = TFT_WIDTH;
String name = "";
int cellWidth = (screen_height / 2) / cols;
int cellHeight = ((screen_width / 2)) / rows; // Reserve 40 pixels for the name display
int keyboardWidth = cols * cellWidth;
// joystick state
boolean joystick_engaged_x = false;
boolean joystick_engaged_y = false;

void displayCenteredText(String text, int centerX, int centerY, int textSize, boolean maxSize = false)
{
    if (maxSize)
    {
        tft.setTextWrap(false);

        // Find the largest font size that fits the screen width
        int16_t x1, y1;
        uint16_t w, h;
        uint8_t font_size = 1;
        int16_t max_width = tft.width() - 20; // Leave a 10-pixel margin on each side

        do
        {
            tft.setTextSize(font_size);
            tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
            font_size++;
        } while (w < max_width && h < tft.height() - 20); // Also check height to avoid overflow
        // Step back one size to ensure it fits
        font_size -= 2;
        tft.setTextSize(font_size);
    }
    tft.setTextSize(textSize);

    // Calculate width of the text
    int16_t x1, y1;
    uint16_t w, h;
    // check if text is empty, put space instead
    if (text == "")
    {
        text = " ";
    }
    tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h); // Calculate bounds, (0,0) is temporary

    // Calculate the start point (x, y)
    int startX = centerX - w / 2;
    int startY = centerY - h / 2;
    Serial.print(" w: ");
    Serial.print(w);
    Serial.print(" h: ");
    Serial.print(h);
    Serial.print(" x: ");
    Serial.print(startX);
    Serial.print(" y: ");
    Serial.print(startY);
    Serial.print(" centerX: ");
    Serial.print(centerX);
    Serial.print(" centerY: ");
    Serial.println(centerY);

    // Set the cursor to the calculated position
    tft.fillRect(0, startY, screen_width, h, TFT_BLACK); // Clear the previous text
    tft.setCursor(startX, startY);
    tft.print(text);
}

void displayText(String text)
{
    // tft.setCursor(10, screen_width - 30);
    // tft.setCursor(screen_height/2, 0);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    // tft.println(text);
    Serial.println(text);
    Serial.print("screen width: ");
    Serial.println(screen_height);

    // tft.setCursor(240, 0);
    // tft.print(text);

    // height  reres
    // displayCenteredText(text, screen_width / 2, screen_height / 4, 3); // Display text centered at (x, y) with size 2
    displayCenteredText(text, screen_width / 2, 20, 3); // Display text centered at (x, y) with size 2
}

void nameTag(String text)
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    displayCenteredText(text, screen_width / 2, screen_height / 2, 4); // Display text centered at (x, y) with size 2
}

void drawLetter(int x, int y, char letter, int color = TFT_WHITE)
{
    int posX = x * cellWidth + (screen_width / 2 - keyboardWidth / 2);
    int posY = y * cellHeight;

    // int posX = x * cellWidth + ((screen_height/2) - (cols * cellWidth/2));
    // int posY = y * cellHeight + ((screen_width/2)- (rows * cellHeight/2));
    tft.setCursor(posX + cellWidth / 4, keyBoardYOffset + posY + cellHeight / 4);
    Serial.println(posX + cellWidth / 4);
    // tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextColor(color);
    tft.setTextSize(2);
    tft.print(letter);
}

void highlightCursor(int x, int y)
{
    int posX = x * cellWidth + (screen_width / 2 - keyboardWidth / 2);
    int posY = (y * cellHeight) + keyBoardYOffset;
    // int posX = x * cellWidth + ((screen_height / 2) - (cols * cellWidth / 2));
    // int posY = y * cellHeight + ((screen_width / 2) - (rows * cellHeight / 2));

    // tft.drawRect(posX, posY, cellWidth, cellHeight, TFT_RED);
    tft.fillRect(posX, posY, cellWidth, cellHeight, TFT_WHITE);
    drawLetter(x, y, letters[y][x], TFT_BLACK);
}
void removeHighlight(int x, int y)
{
    int posX = x * cellWidth + (screen_width / 2 - keyboardWidth / 2);
    int posY = (y * cellHeight) + keyBoardYOffset;
    // int posX = x * cellWidth + ((screen_height / 2) - (cols * cellWidth / 2));
    // int posY = y * cellHeight + ((screen_width / 2) - (rows * cellHeight / 2));
    // tft.drawRect(posX, posY, cellWidth, cellHeight, TFT_BLACK);
    tft.fillRect(posX, posY, cellWidth, cellHeight, TFT_BLACK);
    drawLetter(x, y, letters[y][x]);
}

void handleJoystick()
{
    int newx = map(analogRead(PIN_JOY_X), 0, 4096, -1, 2);
    int newy = map(analogRead(PIN_JOY_Y), 0, 4096, 1, -2);
    // Serial.print("X: ");
    // Serial.print(newx);
    // Serial.print(" Y: ");
    // Serial.println(newy);
    if (joystick_engaged_x && newx == 0)
    {
        joystick_engaged_x = false;
    }
    if (joystick_engaged_y && newy == 0)
    {
        joystick_engaged_y = false;
    }

    if (newx != 0 || newy != 0)
    {
        removeHighlight(cursorX, cursorY);
        if (!joystick_engaged_x)
        {
            cursorX = constrain(cursorX + newx, 0, cols - 1);
            joystick_engaged_x = true;
        }
        if (!joystick_engaged_y)
        {
            cursorY = constrain(cursorY + newy, 0, rows - 1);
            joystick_engaged_y = true;
        }
        // cursorX = cursorX + newx;
        // cursorY = cursorY + newy;
        // cursorX = constrain(cursorX, 0, cols - 1);
        // cursorY = constrain(cursorY, 0, rows - 1);
        highlightCursor(cursorX, cursorY);
    }
}
void drawGrid()
{
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            drawLetter(x, y, letters[y][x]);
        }
    }
}

void handleButtons(const char *buttonname)
{
    if (strcmp(buttonname, "A") == 0)
    {
        name += letters[cursorY][cursorX];
        Serial.println(name);
        displayText(name);
    }
    if (strcmp(buttonname, "B") == 0)
    {
        name = name.substring(0, name.length() - 1);
        Serial.println(name);
        displayText(name);
    }
    if (strcmp(buttonname, "START") == 0)
    {
        state = "nametag";
        nameTag(name);
    }
    if (strcmp(buttonname, "MENU") == 0)
    {
        state = "setup";
        // displayText("Enter your name");
        // name = "";
        // cursorX = 0;
        // cursorY = 0;
        tft.fillScreen(TFT_BLACK);
        displayText(name);
        drawGrid();
        highlightCursor(cursorX, cursorY);
    }
}

void checkButton(Button *b, const char *buttonname)
{

    b->read();

    if (b->wasPressed())
    {
        Serial.print(buttonname);
        Serial.println(" pressed");
        handleButtons(buttonname);
    }
    if (b->wasReleased())
    {
        Serial.print(buttonname);
        Serial.println(" released");
    }
}

void setup()
{
    Serial.begin(115200);
    button_A.begin();
    button_B.begin();
    button_X.begin();
    button_Y.begin();
    button_MENU.begin();
    button_START.begin();
    spi->begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
    tft.init(TFT_WIDTH, TFT_HEIGHT);
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    drawGrid();
    highlightCursor(cursorX, cursorY);
}

void loop()
{
    // handleButtonA();
    if (state == "setup")
    {
        handleJoystick();
        checkButton(&button_A, "A");
        checkButton(&button_B, "B");
        checkButton(&button_X, "X");
        checkButton(&button_Y, "Y");
        checkButton(&button_MENU, "MENU");
        checkButton(&button_START, "START");
    }
    if (state == "nametag")
    {
        checkButton(&button_MENU, "MENU");
    }
    delay(100); // Adjust delay for responsiveness
}

