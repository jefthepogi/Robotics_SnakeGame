#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(32, 16, 2);

struct vec {
  int x, y;
  vec(int x = 0, int y = 0) : x(x), y(y) {}
};

struct node {
  node *prev;
  node *next;
  vec pos;
};

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

Button A(13);
Button B(12);

short time_since_last_draw = 0;
const short MIN_DRAW_WAIT = 60;
unsigned long last_update = 0;

vec mat = vec(5, 8);
vec sectors = vec(16, 2);
vec mapSize = vec(mat.x * sectors.x, mat.y * sectors.y);

struct node *s_head = NULL;

enum Direction {
  LEFT = 1,
  DOWN = LEFT << 1,
  RIGHT = LEFT << 2,
  UP = LEFT << 3,
};

Direction currDir = UP;

void initChar(byte* character)
{
  for (int i = 0; i < 8; i++)
  {
    character[i] = B00000;
  }
}

void initGameMap()
{
  byte SECTORS[sectors.x][sectors.y][mat.y];
  
  for (int i = 0; i < sectors.y; i++)
  {
    for (int j = 0; j < sectors.x; j++)
    {
      initChar(SECTORS[j][i]);
    }
  }
 
  for (node* temp = s_head; temp != NULL; temp = temp->next)
  {
    vec p = vec(temp->pos.x, temp->pos.y);
    vec curr_sec = vec(p.x/mat.x, p.y/mat.y);
    SECTORS[curr_sec.x][curr_sec.y][p.y - (curr_sec.y * mat.y)] = B10000 >> (p.x - (curr_sec.x * mat.x));
    //byte x = B11111 << 10;
    //for (int i = 0; i < 5; i++) Serial.print(bitRead(x, i));
    //Serial.print(curr_sec.x); Serial.print(" "); Serial.println(curr_sec.y);
  }
  
  byte currChar = 0;
  
  for (int i = 0; i < sectors.y; i++)
  {
    for (int j = 0; j < sectors.x; j++)
    {
      for (int h = 0; h < mat.y; h++)
      {
        if (SECTORS[j][i][h])
        {
          lcd.createChar(currChar, SECTORS[j][i]);
          lcd.setCursor(j, i);
          lcd.write(byte(currChar));
          currChar++;
          //Serial.println("fck");
          break;
        }

      }
    }
  }
      
}

void moveSnake()
{
  switch (currDir)
  {
    case LEFT:
      s_head->pos.x--;
    break;
    case DOWN:
      s_head->pos.y++;
    break;
    case RIGHT:
      s_head->pos.x++;
    break;
    case UP:
      s_head->pos.y--;
    break;
  }
}

void control()
{
  if (A.isReleased())
  {
    if (currDir == LEFT)
    {
      currDir = UP;
    }
    else {
      currDir = static_cast<Direction>(currDir << 1);
      //Serial.println("fkoff");
    }
  }
  else if (B.isReleased())
  {
    if (currDir == UP)
    {
      currDir = LEFT;
    }
    else {
      currDir = static_cast<Direction>(currDir << 1);
    }
  }
}

void setup()
{  
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  //Serial.print();
  //Serial.println();
  s_head = new node();
  s_head->prev = NULL;
  s_head->prev = NULL;
  s_head->pos = vec(mapSize.x - 24, mapSize.y - 20);

}

void loop()
{
  control();
  moveSnake();
  initGameMap();
}
