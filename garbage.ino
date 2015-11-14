    /*if (Serial.available())
    {

      char b;
      int i;
      int offset = 0;
      char chin;
      int8_t col = -1;
      if (Serial.peek() == '&')
      {
        Serial.read();
        timer = micros();
        while (!Serial.available() && micros() < timer + serialTimeout);
        if (Serial.available())
        {
          switch (b = Serial.read())
          {
            case 'R':
              col = RED;
              break;
            case 'G':
              col = GREEN;
              break;
            case 'B':
              col = BLUE;
              break;
            case '#':
              col = 9;
              break;
            default:
              if (b < '8')
              {
                col = b - '0';
              }

          }
        }
      }

      for (i = 0; i < 80 && Serial.available(); i++)
      {
        timer = micros();
        switch (chin = Serial.read())
        {
          default:
            myMessage[i - offset] = chin;
            break;
        }
        while (!Serial.available() && micros() < timer + serialTimeout);

      }
      myMessage[i] = 0;
      Serial.flush();
      mw.setSize(strlen(myMessage) * 8);
      mw.setMessage(myMessage, col);
      if (col == 9)
        mw.rainbowify();
    }*/
