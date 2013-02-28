/*
 * swserial430.h - cycle counting based RS232 Serial template for msp430
 *
 * Created: Nov-12-2012
 *  Author: rick@kimballsoftare.com
 *    Date: 02-28-2013
 * Version: 1.0.0
 *
 * Acknowledgments:
 *  Inspiration for cycle counting RX/TX routines from Kevin Timmerman.
 *  see: http://forum.43oh.com/topic/1284-software-async-serial-txrx-without-timer/
 *  Serial API inspired by Arduino Serial.
 *
 * =========================================================================
 *  Copyright © 2013 Rick Kimball
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SWSERIAL_430_H_
#define SWSERIAL_430_H_

/*
 * serial_base_sw_t - blocking bit bang cycle counting UART
 *
 */
template <uint32_t BAUD, uint32_t MCLK_HZ, typename TXPIN, typename RXPIN>
struct serial_base_sw_t {

// private
  template <uint16_t DUR, uint16_t DUR_HALF>
  struct BITRATE {
      enum {
          dur=DUR,
          dur_half=DUR_HALF
      };
  };

  enum { LOOP1_OVERHEAD=16, LOOP2_OVERHEAD=32 }; /* see asm code */
  static const BITRATE<((MCLK_HZ/BAUD)-LOOP1_OVERHEAD) << 1,(MCLK_HZ/BAUD)-LOOP2_OVERHEAD> bit_rate;

  int _read();                    /* asm read routine */
  void _write(const unsigned c);  /* asm write routine */

// public

  /**
   * begin() - initialize TX/RX pins
   */
  void begin(uint32_t baud=BAUD) {
      TXPIN::high();          // TX pin idles with a high value
      TXPIN::setmode_output();
      RXPIN::setmode_input();

      (void)baud; /* use template to set baud rate */
  }

  /*
   * end() - set TX pin back to default
   */
  void end() {
      // no need to flush, we do all the work
      TXPIN::set_low();
      TXPIN::pinMode(GPIO::INPUT);
  }

  /*
   * read() - blocking read
   */
  int read(void) {
      return _read();
  }

  /*
   * write_impl() - provide our implementation for print_t<WRITER> class
   *
   * blocking cycle counting write routine
   */
  int write_impl(uint8_t c) {
      _write(c);
      return 1;
  }

};

/*
 * serial_base_sw_t<>::_write() - write one byte to the TX pin
 */
template <uint32_t BAUD, uint32_t MCLK_HZ, typename TXPIN, typename RXPIN>
void serial_base_sw_t<BAUD, MCLK_HZ, TXPIN, RXPIN>::_write(unsigned c)
{
    __asm__ (
            "   bis     #0x0300, %[c]        ; Add Stop bit(s) to tx char\n"
            "   mov     %[bit_dur], r14      ; Bit duration\n"
            "   mov     %[tx_bit_mask], r12  ; Serial output bitmask\n"
            "   jmp     3f                   ; Send start bit...\n"
            "1:\n"
            "   mov     r14, r13             ; Get bit duration\n"
            "2:\n"
            "   nop                          ; 4 cycle loop\n"
            "   sub     #8, r13\n"
            "   jc      2b\n"
            "   subc    r13, r0              ; 0 to 3 cycle delay\n"
            "   nop                          ; 3\n"
            "   nop                          ; 2\n"
            "   nop                          ; 1\n"
            "   rra     %[c]                 ; Get bit to tx, test for zero\n"
            "   jc      4f                   ; If high...\n"
            "3:\n"
            "   bic.b   r12, %[PXOUT]        ; Send zero bit\n"
            "   jmp     1b                   ; Next bit...\n"
            "4:\n"
            "   bis.b   r12, %[PXOUT]        ; Send one bit\n"
            "   jnz     1b                   ; If tx data is not zero, then there are more bits to send...\n"

            : /* return value */

            : /* external variables */
            [c]           "r" (c),
            [bit_dur]     "i" (bit_rate.dur),
            [half_dur]    "i" (bit_rate.dur_half),
            [tx_bit_mask] "i" (TXPIN::pin_mask),   /* #BIT2  */
            [PXOUT]       "m" (TXPIN::POUT()) /* &P1OUT */

            : /* _write() clobbers these registers */
            "r14", "r13", "r12"
    );

    return;
}

/*
 * serial_base_sw_t<>::_read() - read one byte from RX pin
 */
template <uint32_t BAUD, uint32_t MCLK_HZ, typename TXPIN, typename RXPIN>
int serial_base_sw_t<BAUD, MCLK_HZ, TXPIN, RXPIN>::_read()
{
    register unsigned result;

    __asm__ (
        " mov     %[bit_dur], r14     ; Bit duration\n"
        " mov     %[rx_bit_mask], r13 ; Input bitmask\n"
        " mov     #0x01FF, %[rc]      ; 9 bits - 8 data + stop\n"

        "1:                           ; Wait for start bit\n"
        " mov.b   %[PXIN], r12        ; Get serial input\n"
        " and     r13, r12            ; Mask and test bit\n"
        " jc      1b                  ; Wait for low...\n"
        " mov     %[half_dur], r13    ; Wait for 1/2 bit time\n"
        "3:\n"
        " nop                         ; Bit delay\n"
        " sub     #8, r13             ; 1/2 bit adjust\n"
        " jc      3b\n"
        " subc    r13, r0             ; 0 to 3 cycle delay\n"
        " nop                         ; 3\n"
        " nop                         ; 2\n"
        " nop                         ; 1\n"
        " mov.b   %[PXIN], r12        ; Get serial input\n"
        " and     %[rx_bit_mask], r12 ;\n"
        " rrc     %[rc]               ; Shift in a bit\n"
        " mov     r14, r13            ; Setup bit timer\n"
        " jc      3b                  ; Next bit...\n"

        " rla     %[rc]               ; Move stop bit to carry\n"
        " swpb    %[rc]               ; Move rx byte to lower byte, start bit in msb\n"

        "4:\n"

        : /* return value */
            [rc] "=r" (result)
        : /* external variables */
            [bit_dur]     "i" (bit_rate.dur),
            [half_dur]    "i" (bit_rate.dur_half),
            [rx_bit_mask] "i" (RXPIN::pin_mask),   /* #BIT1, */
            [PXIN]        "m" (RXPIN::PIN())  /* &P1IN, */
        : /* _read() clobbers these registers */
            "r14", "r13", "r12"
    );

    return result;
}

// typical usage
template <uint32_t BAUD, uint32_t MCLK_HZ, typename TXPIN, typename RXPIN>
struct sw_serial_t:
    serial_base_sw_t<BAUD,MCLK_HZ,TXPIN,RXPIN>,
    print_t<sw_serial_t<BAUD, MCLK_HZ, TXPIN, RXPIN> >
{
};

#endif /* SWSERIAL_430_H_ */