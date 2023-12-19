#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

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
bool gameBegin = false;

vec mat = vec(5, 8);
vec sectors = vec(16, 2);
vec mapSize = vec(mat.x * sectors.x, mat.y * sectors.y);
vec applePos = vec(0, 0);

struct node *s_head = NULL;
struct node *s_tail = NULL;
int n = 0; 

enum Direction {
  LEFT = 1,
  DOWN = LEFT << 1,
  RIGHT = LEFT << 2,
  UP = LEFT << 3,
};

Direction currDir;

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
  
  // Draws the apple
  vec aSec = vec(applePos.x / mat.x, applePos.y / mat.y);
  SECTORS[aSec.x][aSec.y][applePos.y - (aSec.y * mat.y)] |= (B10000 >> (applePos.x - (aSec.x * mat.x)));
 
  for (node* temp = s_head; temp != NULL; temp = temp->next)
  {
    vec p = vec(temp->pos.x, temp->pos.y);
    vec curr_sec = vec(p.x/mat.x, p.y/mat.y);
    SECTORS[curr_sec.x][curr_sec.y][p.y - (curr_sec.y * mat.y)] |= B10000 >> (p.x - (curr_sec.x * mat.x));
    //byte x = B11111 << 10;
    //for (int i = 0; i < 5; i++) Serial.print(bitRead(x, i));
    //Serial.print(curr_sec.x); Serial.print(" "); Serial.println(curr_sec.y);
  }
  
  byte currChar = 0;
  
  for (int i = 0; i < sectors.y; i++)
  {
    for (int j = 0; j < sectors.x; j++)
    {
      bool pixel = false;
      for (int h = 0; h < mat.y; h++)
      {
        if (SECTORS[j][i][h])
        {
          lcd.createChar(currChar, SECTORS[j][i]);
          
          lcd.setCursor(j, i);
          lcd.write(byte(currChar));
          currChar++;
          pixel = true;
          break;
        }
        
      }
      
      if (!pixel)
      {
        lcd.setCursor(j, i);
        lcd.write(' ');
      }

    }
  }
      
}

void repositionApple()
{
  applePos.x = random(0, mapSize.x);
  applePos.y = random(0, mapSize.y);
}

struct node *addBody()
{
  struct node *newPart = new node();
  struct node *last = s_tail->prev;
  newPart->next = s_tail;
  newPart->prev = last;
  last->next = newPart;
  s_tail->prev = newPart;
  
  return newPart;
}

void moveSnake()
{
  vec prevPos = vec(s_head->pos.x, s_head->pos.y);
  
  switch (currDir)
  {
    case LEFT:
    
     if(s_head->pos.x == 0) {
        s_head->pos.x = mapSize.x - 1;
      }
      else
        s_head->pos.x--;
    break;
    case DOWN:
     if(s_head->pos.y == mapSize.y - 1) {
       s_head->pos.y = 0;
      }
     else
       s_head->pos.y++;
    break;
    case RIGHT:
     if(s_head->pos.x == mapSize.x - 1) {
       s_head->pos.x = 0;
      }
      s_head->pos.x++;
    break;
    case UP:
     if(s_head->pos.y == 0) {
       s_head->pos.y = mapSize.y - 1;
      }
     else
      s_head->pos.y--;
    break;
  }
  
  for (node* temp = s_head->next; temp != NULL; temp = temp->next)
  {
    vec tempPos = vec(temp->pos.x, temp->pos.y);
    temp->pos = vec(prevPos.x, prevPos.y);
    prevPos = vec(tempPos.x, tempPos.y);
  }
}

void controlSnake()
{
  if (A.isReleased())
  {
    if (currDir == UP)
    {
      currDir = LEFT;
    }
    else {
      currDir = static_cast<Direction>(currDir << 1);
    }
  }
  else if (B.isReleased())
  {
    if (currDir == LEFT)
    {
      currDir = UP;
    }
    else {
      currDir = static_cast<Direction>(currDir >> 1);
    }
  }
}

bool gameOver()
{
  for (node* temp = s_head->next; temp != NULL; temp = temp->next)
  {
    vec headPos = vec(s_head->pos.x, s_head->pos.y);
    vec nodePos = vec(temp->pos.x, temp->pos.y);
    if (headPos.x == nodePos.x && headPos.y == nodePos.y)
    {
      return true;
    }
  } 

  return false;
}

void resetGame()
{
  for (node* temp = s_tail; temp != NULL; temp = temp->prev)
  {
    delete temp;
  }
  
  currDir = UP;
  
  s_head = new node();
  s_tail = new node();

  s_head->next = s_tail;
  s_head->prev = NULL;
  s_head->pos = vec(mapSize.x/2, mapSize.y - 4);
  
  s_tail->next = NULL;
  s_tail->prev = s_head;
  
  struct node *body1 = addBody();
  body1->pos = vec(s_head->pos.x, s_head->pos.y - 1);

}

void setup()
{  
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);;
  randomSeed(analogRead(0));
 
  lcd.setCursor(2,0);
  lcd.print("SNAKE GAME");
  lcd.setCursor(2,1);
  lcd.print("by Group 5");
  delay(2000);

  resetGame();
  repositionApple();
  initGameMap();
}

void loop()
{
    controlSnake();

    unsigned long time = millis();
    unsigned long elapsed = time - last_update;
    last_update = time;
    time_since_last_draw += elapsed;
    if(time_since_last_draw >= MIN_DRAW_WAIT) {
      if(gameOver() && n > 0)
      {
        resetGame();
        repositionApple();
        n = -1;
      }
      moveSnake();
      if(s_head->pos.x == applePos.x && s_head->pos.y == applePos.y) {
        repositionApple();
        addBody();
      }
      initGameMap();
      time_since_last_draw = 0;
    }

  if (n < 1) // to avoid triggering gameover again and again.
  {
    n++;
  }
  
}
