#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(32, 16, 2);

struct vec{
  int x, y;
};

// based on code by Majenko @ https://arduino.stackexchange.com/a/48244
class Button {
    private:
        bool _state;
        uint8_t _pin;

    public:
        Button(uint8_t pin) : _pin(pin) {
            pinMode(_pin, INPUT_PULLUP);
            _state = digitalRead(_pin);
        }

        bool isReleased() {
            bool v = digitalRead(_pin);
            if (v != _state) {
                _state = v;
                if (_state) {
                    return true;
                }
            }
            return false;
        }
};


Button UP(6);
Button DOWN(4);
Button LEFT(7);
Button RIGHT(5);

vec axis = {0, 0};
vec grid = {5, 8};
vec dir = {0, 0};
vec DIR[] = {{0, 1},// UP
			{0,-1},// DOWN
			{-1, 0},//LEFT
			{1, 0}};//RIGHT

int row = 4;
byte lastPos = B00100;

byte snake[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

void updateSnake();
void Input();

void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
}

void loop()
{
  Input();
  updateSnake();
}

void Input()
{
  
  if (UP.isReleased() && dir.y == 0)
  {
    dir = {0, 1};
    snake[row] = lastPos;
  }
  else if (DOWN.isReleased() && dir.y == 0)
  {
    dir = {0, -1};
    snake[row] = lastPos;
  }
  else if (LEFT.isReleased() && dir.x == 0)
  {
    dir = {-1, 0};
    snake[row] = lastPos;
  }
  else if (RIGHT.isReleased() && dir.x == 0)
  {
    dir = {1, 0};
    snake[row] = lastPos;
  }
}

void updateSnake()
{
  // Serial.println(row);

  if (dir.y == DIR[1].y ) // DOWN
  {
     if (row > grid.y - 1)
    {
      axis.y++;
      if (axis.y > 1)
      {
        axis.y = 0;
      }
      snake[row-1] = B00000;
      row = 0;
      lcd.clear();
      delay(100);
      return;
    }

    snake[row] = lastPos;

    lcd.createChar(0, snake);
    lcd.setCursor(axis.x, axis.y);
    lcd.write((byte)0);
    
	snake[row] = B00000;
    row++;
  }
  
  if (dir.y == DIR[0].y) // UP
  {
    if (row < 0)
    {
      axis.y--;
      if (axis.y < 0)
      {
        axis.y = 1;
      }
      
      snake[row+1] = B00000;
      row = grid.y - 1;
	  lcd.write(" ");
      return;
    }
    
    snake[row] = lastPos;

    lcd.createChar(0, snake);
    lcd.setCursor(axis.x, axis.y);
    lcd.write((byte)0);
    
    snake[row] = B00000;
    row--;
  }
  
  if (dir.x == DIR[2].x) // LEFT
  {
    snake[row] = snake[row] << 1;
    lastPos = snake[row];

    lcd.createChar(0, snake);
    lcd.setCursor(axis.x, axis.y);
    lcd.write((byte)0);
    
    if (snake[row] == B00000)
    {
      axis.x--;
      if (axis.x < 0)
      {
        axis.x = 15;
      }
      
      snake[row] = B00001;
      lastPos = snake[row];
      lcd.write(" ");
      return;
    }
  }
  
  if (dir.x == DIR[3].x) // RIGHT
  {
    snake[row] = snake[row] >> 1;
    lastPos = snake[row];

    lcd.createChar(0, snake);
    lcd.setCursor(axis.x, axis.y);
    lcd.write((byte)0);
    
    if (snake[row] == B00000)
    {
      axis.x++;
      if (axis.x > 15)
      {
        axis.x = 0;
      }
      
      snake[row] = B10000;
      lastPos = snake[row];
      lcd.write(" ");
      return;
    }
  }
  
}
