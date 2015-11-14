#define usePGM
#include "supercd.h"
#include "font.h"
#include "fonts.h"
#define MAX_MSG_SIZE 20
#define RED 1
#define GREEN 2
#define BLUE 4
void setup() {
  // put your setup code here, to run once:
  DDRL = 0xFF; //black wires
  DDRC = 0xFF; //red wires
  Serial.begin(9600);
  pinMode(40, INPUT);
}
unsigned long sdel = 5;
unsigned long recycles = 5;
unsigned long serialTimeout = 2000;
unsigned long timer = 0;
#define select(x,y) ((x>>4*(y) & 0x0F))

uint16_t flip(uint16_t in, bool doFlip = true)
{
  uint16_t result = 0;
  if (doFlip)
  {
    for (byte i = 0; i < 16; i++)
    {
      result |= (0x01 & in >> 15 - i ) << i;
    }
    return result;
  }
  else
    return in;
}



class MagicWand
{

  private:
    char* activemessage;
    uint16_t* buf;
    unsigned int sz = 0;

  public:
    typedef uint16_t rgb[3];
    rgb* rgbscreen = 0;

    void setSize(unsigned int sz)
    {
      if (rgbscreen)
        delete[] rgbscreen;
      rgbscreen = new rgb[sz];
      this->sz = sz;
      for (unsigned int i = 0; i < sz; i++)
      {
        rgbscreen[i][0] = 0;
        rgbscreen[i][1] = 0;
        rgbscreen[i][2] = 0;
      }
    }
    MagicWand(unsigned int sz)
    {
      setSize(sz);
    }

    ~MagicWand()
    {
      delete[] rgbscreen;
    }

    void render(uint16_t &r, uint16_t &g, uint16_t &b)
    {
      for (byte i(0); i < 4; i++)
      {
        PORTC = (g ? select(g, i) : 0) << 4 | (b ? select(b, i) : 0); //green, blue
        PORTL = (r ? select(r, i) : 0) << 4 | (~(1 << i) & 0x0F); //red, GND
        //PORTL = (r ? select(r, i) : 0) << 4 | (~(1 << i) & 0x0F); //red, GND
        delayMicroseconds(sdel);
        PORTC = 0;
        PORTL = 0x0F;

      }
    }
    void render(bool dir = true)
    {
      for (unsigned short i (0); i < sz; i++)
        for (unsigned short c(0); c < recycles; c++)
          render(rgbscreen[dir ? i : sz - 1 - i][0], rgbscreen[dir ? i : sz - 1 - i][1], rgbscreen[dir ? i : sz - 1 - i][2]);
    }
    void putChar(unsigned short i, char c, bool usd, byte col, FontWrapper *f = 0)
    {
      if (f)
      {
        if (f->height <= 8)
        {
          for (byte x = 0; x < 8; x++)
          {
            rgbscreen[i + x][0] = col & 0x01 ? flip( ((Font<uint8_t> *)f)->column(c, x), usd) : 0;
            rgbscreen[i + x][1] = col & 0x02 ? flip( ((Font<uint8_t> *)f)->column(c, x), usd) : 0;
            rgbscreen[i + x][2] = col & 0x04 ? flip( ((Font<uint8_t> *)f)->column(c, x), usd) : 0;
            if (col == 9)
              rainbowify(usd, i, i + f->width);
          }
        }
        else if(f->height <= 16)
        {
          for (byte x = 0; x < 8; x++)
          {
            rgbscreen[i + x][0] = col & 0x01 ? flip( ((Font<uint16_t> *)f)->column(c, x), usd) : 0;
            rgbscreen[i + x][1] = col & 0x02 ? flip( ((Font<uint16_t> *)f)->column(c, x), usd) : 0;
            rgbscreen[i + x][2] = col & 0x04 ? flip( ((Font<uint16_t> *)f)->column(c, x), usd) : 0;
            if (col == 9)
              rainbowify(usd, i, i + f->width);
          }
        }
        else
        {
          for (byte x = 0; x < f->width; x++)
          {
            rgbscreen[i + x][0] = col & 0x01 ? flip( ((Font<uint32_t> *)f)->column(c, x) >> 4, usd) : 0;
            rgbscreen[i + x][1] = col & 0x02 ? flip( ((Font<uint32_t> *)f)->column(c, x) >> 4, usd) : 0;
            rgbscreen[i + x][2] = col & 0x04 ? flip( ((Font<uint32_t> *)f)->column(c, x) >> 4, usd) : 0;
            if (col == 9)
              rainbowify(usd, i, i + f->width);
          }
        }
      }
      else
        for (byte x = 0; x < 8; x++)
        {
          rgbscreen[i + x][0] = col & 0x01 ? flip(ch[c][x], usd) : 0;
          rgbscreen[i + x][1] = col & 0x02 ? flip(ch[c][x], usd) : 0;
          rgbscreen[i + x][2] = col & 0x04 ? flip(ch[c][x], usd) : 0;
          if (col == 9)
            rainbowify(usd, i, i + 8);
        }
    }

    void setMessage(char* message, int8_t color = -1, FontWrapper *f=0)
    {
      bool usd = false;
      if (message)
      {
        for (int i = 0; message[i] && i < sz / 8; i++)
        {
          byte col = (color < 0 ? 1 << (random() % 3) : color);
          putChar(i * f->width, message[i], usd, col, f);
        }
      }
    }


    unsigned int getSize()
    {
      return sz;
    }
    void rainbowify(bool usd = 0, unsigned short start = 0, unsigned short nd = 0)
    {
      if (nd == 0)
        nd = sz;
      for (; start < nd; start++)
      {
        rgbscreen[start][0] |= rgbscreen[start][1] | rgbscreen[start][2];
        // B__________T
        //0123456789ABCDEF
        rgbscreen[start][2] = rgbscreen[start][0] & flip(0b1111110000000000, usd);
        rgbscreen[start][1] = rgbscreen[start][0] & flip(0b0000011111100000, usd);
        rgbscreen[start][0] &=                      flip(0b0000000000111111, usd);
      }
    }


};

bool serialToColMap(char* buf, byte* colmap, int sz, int* acFnt=0)
{
  bool didSomething = Serial.available();
  byte col = RED; //active color
  byte fnt = 8; //active font (index)
  int i;
  for (i = 0; i < sz - 1 && Serial.available(); i++)
  {
    unsigned long timer = micros();
    if (Serial.peek() == '&')
    {
      Serial.read();
      char dir;
      timer = micros();
      while (!Serial.available() && micros() < timer + serialTimeout);
      switch (dir = Serial.read())
      {
        case 'R': col = RED; break;
        case 'G': col = GREEN; break;
        case 'B': col = BLUE; break;
        case '&': colmap[i] = col | (fnt << 3); buf[i] = '&'; i++; break;
        case '#': col = 9; break;
        default:
          if(dir >= '0' && dir <= '9')
            col = dir - '0';
          else if(dir >= 'a' && dir <= 'k' && acFnt)
            *acFnt = dir - 'a';
            
          break;
      }
    }
    timer = micros();
    while (!Serial.available() && micros() < timer + serialTimeout);
    colmap[i] = col;
    buf[i] = Serial.read();
    timer = micros();
    while (!Serial.available() && micros() < timer + serialTimeout);
  }
  buf[i] = 0;
  return didSomething;
}

void loop() {
  // put your main code here, to run repeatedly:
  /*uint16_t rgb[3] = {
    //0123456789ABCDEF
    0b1000000000000000,
    0b0000000000000000,
    0b0000000000000000
    //R
    //RG
    //G
    //GB
    //B

  };
  */
  
  Font <const uint16_t> terminus16(&font_terminus16, 16, 8, 256);
  Font <const uint8_t > cyr8 (&font_cyrA8, 8, 8, 256);
  Font <const uint8_t > cp8658x8 (&font_cp8658x8, 8, 8, 256);
  Font <const uint16_t> default8x9 (&font_default8x9, 9, 8, 256);
  Font <const uint8_t > drdos8x6 (&font_drdos8x6, 6, 8, 448);
  Font <const uint8_t > drdos8x8 (&font_drdos8x8, 8, 8, 448);
  Font <const uint16_t> gr737b9x16medieval (&font_gr737b9x16medieval, 16, 8, 256);
  Font <const uint16_t> gr9288x16thin (&font_gr9288x16thin, 16, 8, 256);
  Font <const uint32_t> iso0112x22 (&font_iso0112x22, 22, 12+1, 256);
  Font <const uint16_t> lat416 (&font_lat416, 16, 8, 256);
  Font <const uint32_t> latgrkcyr12 (&font_latgrkcyr12, 22, 12, 512);
  Font <const uint16_t> other (&font_other, 16, 8, 256);
  FontWrapper* fontArray[] = { 
    &cyr8, 
    &cp8658x8,  
    &drdos8x6, 
    &drdos8x8, 
    &default8x9, 
    &gr737b9x16medieval, 
    &gr9288x16thin, 
    &lat416, 
    &terminus16, 
    &iso0112x22,
    &other };
    //                       a    b          c         d         e       f       g      h        i         j      k
    typedef enum fontCode { CYR, CP865, DRDOS8x6, DRDOS8x8, DEFAULT8x9, GR737, GR928, LAT416, TERMINUS16, ISO1, OTHER};
  int activeFont=8;
  FontWrapper* defaultFont = fontArray[TERMINUS16];
  
  
  char myMessage[81] =  "SAMPLE TEXT  ";
  byte colmap[81];

  MagicWand mw(strlen( myMessage) * defaultFont->width);
  //01234567890123456789
  mw.setMessage(myMessage, RED, defaultFont );
  //mw.rgbscreen[0][0] = 0xFFFF;
  //mw.rainbowify();
  for (int i = 0; i < mw.getSize(); i++)
  {
    //mw.rgbscreen[i][2]|=(i%8?0:~0);
    //mw.rgbscreen[i][0]=0;
    //mw.rgbscreen[i][1]=0xF0F0;
    //mw.rgbscreen[i][2]=0;

  }
  while (true)
  {
    //mw.render(&rgb[0],&rgb[1],&rgb[2]);
    //while(!digitalRead(40));
    while (digitalRead(40));
    //sdel=analogRead(A4);
    delay(10);


    mw.render(true); //render left to right
    //mw.render(false); //render right to left
    if (serialToColMap(myMessage, colmap, 81, &activeFont))
    {
      int i;
      mw.setSize(strlen(myMessage) * fontArray[activeFont]->width);
      for (i = 0; i < strlen(myMessage); i++)
      {
        //CHANGE THIS TO 1 TO FLIP THE DISPLAY ------------------------v
        mw.putChar(i * (fontArray[activeFont] -> width), myMessage[i], 0, colmap[i], fontArray[activeFont]);
      }
    }


  }


}
