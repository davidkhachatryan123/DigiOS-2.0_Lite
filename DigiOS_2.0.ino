/* ----------------------------------------------
  DigiOS 2.0 - mini-OS emulator for Digispark

  Available commands:
  p[0-2] [on|off], ls, help, clear,
  reboot, logout
  ----------------------------------------------- */

// password of up to seven characters
const char password[] = "root12";

//-----------------------------------------------

#include <DigiCDC.h>

char serialChar[1], stringInput[8];
bool stringComplete = false;


enum system_status
{
  LOGIN,
  PASSWORD,
  LOGEDIN
};

system_status _status = LOGIN;


static void reboot()
{
  SerialUSB.print(F("\r\nRebooting ... "));

  noInterrupts();
  CLKPR = 0b10000000;
  CLKPR = 0;
  void (*ptrToFunction)();
  ptrToFunction = 0x0000;
  (*ptrToFunction)();
}

static void logout()
{
  _status = LOGIN;

  SerialUSB.print(F("\r\nBye!"));
}

void clearScreen()
{
  for (uint8_t i = 0; i < 35; i++)
    SerialUSB.println();
}

static void horizontaLine()
{
  SerialUSB.println();

  for (uint8_t i = 0; i < 32; i++)
    SerialUSB.print(F("-"));

  SerialUSB.println();
}

static void gpioList()
{
  horizontaLine();
  SerialUSB.print(F("GPIO status list"));
  horizontaLine();

  for (uint8_t i = 0; i < 3; i++) {
    SerialUSB.print(F("\r\nPin "));
    SerialUSB.print(i, DEC);
    SerialUSB.print((PINB & (1 << i)) ? F(" HIGH") : F(" LOW"));
  }

  SerialUSB.println();
  horizontaLine();
}

static void help()
//-----------------------------------------------
{
  horizontaLine();
  SerialUSB.println(F("\r\nDigiOS version 2.0 User Commands"));
  horizontaLine();

  SerialUSB.println(F("\r\nlogin, p[0-2] [on|off], ls, clear, help, logout"));
}

static void serialReader()
{
  while (SerialUSB.available())
  {
    serialChar[0] = (char)SerialUSB.read();

    if ((' ' <= serialChar[0]) && (serialChar[0] <= '~'))
    {
      strcat(stringInput, serialChar);
    }
    else
    {
      if (stringInput[0] != 0)
      {
        stringComplete = true;

        return;
      }
    }
  }
}


// list of keywords and procedures assigned to them
static const struct
{
  const char phrase[8];
  void (*handler)(void);
} keys[] =
{
  // ---- comment on this block to get more memory for your own code ---
  { "ls", gpioList },
  { "reboot", reboot },
  { "clear", clearScreen },
  { "help", help },
  // -------------------------------------------------------------------
  { "logout", logout }
};


void setup()
{
  // Set pins 0-2 as OUTPUT:
  DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

  SerialUSB.begin();
}

void loop()
{
  serialReader();

  if (stringComplete)
  {
    switch (_status)
    {
      case LOGIN:
        clearScreen();
        SerialUSB.print(F("\r\nDigiOS 2.0 - Digispark mini-OS\r\n\r\nPassword: "));

        _status = PASSWORD;

        break;
      case PASSWORD:
        if (!strcmp(stringInput, password))
          _status = LOGEDIN;
        else {
          SerialUSB.println(F("\r\nLogin incorrect"));
          SerialUSB.delay(1500);

          _status = LOGIN;
        }

        return;
        break;
      case LOGEDIN:
        SerialUSB.print(F("\r\ncmd:> "));

        // keyword procedures
        for (uint8_t i = 0; i < sizeof keys / sizeof * keys; i++)
          if (!strcmp(stringInput, keys[i].phrase))
          {
            keys[i].handler();

            if (i != (sizeof keys / sizeof * keys) - 1)
            {
              SerialUSB.print(F("\r\ncmd:> "));
            }
          }

        if (stringInput[0] == 'p') {
          if ((stringInput[1] - 48) < 3 and stringInput[4] == 'n') {
            PORTB |= (1 << stringInput[1] - 48);
          } else if ((stringInput[1] - 48) < 3 and stringInput[4] == 'f') {
            PORTB &= ~(1 << stringInput[1] - 48);
          }
        }

        break;
    }

    stringInput[0] = 0;
    stringComplete = false;
  }
}
