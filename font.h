#include "Arduino.h"
//font class 
//(actual font definitions must be declared elsewhere, this is just a wrapper.)
class FontWrapper
{
  public:
    const int height;
    const int width;
    const int sz;
    FontWrapper( int h, int w, int s): height(h), width(w), sz(s)
    {}
};
  
template <typename T>
class Font: public FontWrapper
{
  private:
    const T* base;
    
  public:
    Font(const void* ptr, int chheight=sizeof(T)*8, int chwidth=8, int sz=256):
      FontWrapper(chheight, chwidth, sz), base(( (T*) ptr))
    {
      //etc
    }
    T* letter(char c)
    {
      return (T*) (base + (width * c));
    }
    T* operator[](char c)
    {
      return letter(c);
    }
    T column(char c, unsigned int x)
    {
      if(x >= width)
        return 0;
      if(sizeof(T) <= 2)
        return pgm_read_word_near(letter(c)+x);
        
      else if(sizeof(T) <= 4)
      {
        //because arduino sucks at reading PROGMEM data that is more than 2 bytes wide
        const void* p = letter(c);
        return ((T)pgm_read_word_near(p+4*x+2) << 16) | pgm_read_word_near(letter(c)+x);
      }
      else
        return pgm_read_word_near(letter(c)+x);
    }
    T sizemask()
    {
      return (T) ((1<<(height-1)-1)<<1)+1;
    }
};
