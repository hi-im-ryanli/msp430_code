/**
 * ascii_table_extended.cpp - spew ASCII table print insertion operators
 *
 * Author: rick@kimballsoftware.com
 * Date: 2-28-2013
 *
 */

#include <fabooh.h>
#include <serial.h>

#define NO_DATA_INIT
#define NO_BSS_INIT
#include "main.h"

template <uint32_t BAUD, uint32_t MCLK_HZ, typename TXPIN, typename RXPIN>
struct serial_t:
#if defined(__MSP430_HAS_USCI__)
  serial_base_usci_t<BAUD, MCLK_HZ, TXPIN, RXPIN>,
#else
  serial_base_sw_t<BAUD, MCLK_HZ, TXPIN, RXPIN>,
#endif
  print_t<serial_t<BAUD, MCLK_HZ, TXPIN, RXPIN>, uint16_t, uint32_t>
{
};

//------- File Globals ------
namespace {
  const uint32_t BAUD_RATE=9600;
  typedef serial_t<BAUD_RATE, CPU::frequency, TX_PIN, NO_PIN> serial;
  serial Serial;
  unsigned thisByte; // first visible ASCIIcharacter ' ' is number 32:
}

inline void setup(void)
{
  thisByte=' '; // first visible ASCIIcharacter ' ' is number 32:

  Serial.begin(BAUD_RATE);

  // prints title with ending line break
  Serial.print("ASCII Table ~ Character Map\n");
}

void loop()
{

  // either of these methods should end up with the same code size
#if 1
  // use insertion operator style

  Serial  << _RAW(thisByte)
          << ", dec: " << thisByte
          << ", oct: 0" << _OCT(thisByte)
          << ", hex: 0x" << _HEX(thisByte)
          << ", bin: 0b" << _BIN(thisByte)
          << endl;
#else
  // use method call style

  Serial.print(thisByte, RAW);
  Serial.print(", dec: "); Serial.print(thisByte);
  Serial.print(", oct: 0"); Serial.print(thisByte,OCT);
  Serial.print(", hex: 0x"); Serial.print(thisByte,HEX);
  Serial.print(", bin: 0b"); Serial.print(thisByte,BIN);
  Serial.println();
#endif

  // if printed last visible character '~' or 126, stop:
  if( thisByte == '~') {
    // This loop loops forever and does nothing
    //while(1);
    LPM4;
  }

  // go on to the next character
  thisByte++;
}
