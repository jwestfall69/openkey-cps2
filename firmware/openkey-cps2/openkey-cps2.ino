#include <avr/sleep.h>

// PIN 9 (PIN_B1) <=> CN9 PIN 2
#define SET_DATA      PORTB.OUTSET = PIN0_bm
#define CLR_DATA      PORTB.OUTCLR = PIN0_bm

// PIN 8 (PIN_B0) <=> CN9 PIN 3
#define SET_SETUP    PORTB.OUTSET = PIN1_bm
#define CLR_SETUP    PORTB.OUTCLR = PIN1_bm

// PIN 6 (PIN_B3) <=> CN9 PIN 4
#define SET_RESET     PORTB.OUTSET = PIN3_bm
#define CLR_RESET     PORTB.OUTCLR = PIN3_bm

// PIN 7 (PIN_B2) <=> CN9 PIN 5
#define SET_CLOCK     PORTB.OUTSET = PIN2_bm
#define CLR_CLOCK     PORTB.OUTCLR = PIN2_bm

// doing bit shifting takes a variable amount
// of time depending on the how far the shift
// is.  Using the below lookup table to try
// and keep a consistent clock period when
// sending data.
uint8_t masks[8] = {
  0x01, 0x02, 0x04, 0x08,
  0x10, 0x20, 0x40, 0x80,
};

// The key data below comes from mame.  Each game has a .key file, which contains the 20 bytes.
// ie: https://github.com/mamedev/mame/blob/master/src/mame/capcom/cps2.cpp#L1900
#define CONFIG_SIZE   6
#define WATCHDOG_SIZE 6
#define KEY_SIZE      4

// There are only 10 unique combinations of the first 6 bytes
// of all key data.  In order to save about 800 bytes of flash
// we are using the below lookup table for these 6 bytes. 
// Without the lookup table the code compiles very near to 4k,
// which would put it at risk of not being viable on attiny4x4s
uint8_t config_table[][CONFIG_SIZE] = {
 { 0x01, 0x00, 0x02, 0x40, 0x00, 0x08 },  // 0x00
 { 0x01, 0x00, 0x02, 0x40, 0x00, 0x09 },  // 0x01
 { 0x01, 0x00, 0x02, 0x40, 0x00, 0x0a },  // 0x02
 { 0x07, 0x00, 0x02, 0x40, 0x00, 0x08 },  // 0x03
 { 0x07, 0x00, 0x02, 0x40, 0x00, 0x0a },  // 0x04
 { 0x0f, 0x00, 0x02, 0x40, 0x00, 0x08 },  // 0x05
 { 0x0f, 0x00, 0x02, 0x40, 0x00, 0x09 },  // 0x06
 { 0x0f, 0x00, 0x02, 0x40, 0x00, 0x0a },  // 0x07
 { 0x3f, 0x00, 0x02, 0x40, 0x00, 0x08 },  // 0x08
 { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },  // 0x09
};

struct game_data_t {
  uint8_t config_table_index;
  uint8_t watchdog[WATCHDOG_SIZE];
  uint8_t key2[KEY_SIZE];
  uint8_t key1[KEY_SIZE];
};

#define MAX_GAME_NUM 174
struct game_data_t game_data[MAX_GAME_NUM + 1] = {
  // config table                                                                                                          jumpers
  //    index                  watchdog config                         key#2                        key#1                  12345678 raw key filename
  {     0x06,       { 0x84, 0xc2, 0xeb, 0x7a, 0x3c, 0xa5 },  { 0xf4, 0x38, 0xeb, 0x65 },  { 0x79, 0x9c, 0xbf, 0x70 } }, // 00000000 1944.key
  {     0x06,       { 0x84, 0xc2, 0xeb, 0x7a, 0x3c, 0xa5 },  { 0x8b, 0x93, 0x46, 0x1d },  { 0x70, 0xe7, 0xaf, 0x10 } }, // 00000001 1944j.key
  {     0x06,       { 0x84, 0xc2, 0xeb, 0x7a, 0x3c, 0xa5 },  { 0x78, 0xfe, 0x6b, 0x44 },  { 0xc9, 0x39, 0xf2, 0xe0 } }, // 00000010 1944u.key
  {     0x04,       { 0x04, 0xc2, 0xa4, 0x02, 0x02, 0x21 },  { 0xe8, 0x85, 0x13, 0x2a },  { 0x2e, 0x46, 0x8b, 0x00 } }, // 00000011 19xx.key
  {     0x04,       { 0x04, 0xc2, 0xa4, 0x02, 0x02, 0x22 },  { 0x23, 0xb1, 0x6f, 0xb6 },  { 0xbc, 0xcb, 0x9c, 0xcc } }, // 00000100 19xxa.key
  {     0x04,       { 0x04, 0xc2, 0xa4, 0x02, 0x02, 0x23 },  { 0xf1, 0x1b, 0x7e, 0xd1 },  { 0x5b, 0x8a, 0x7e, 0x9c } }, // 00000101 19xxb.key
  {     0x04,       { 0x04, 0xc2, 0xa4, 0x02, 0x02, 0x23 },  { 0xe4, 0xda, 0x13, 0xbd },  { 0x7d, 0x76, 0x4a, 0xe8 } }, // 00000110 19xxh.key
  {     0x04,       { 0x04, 0xc2, 0xa4, 0x02, 0x02, 0x21 },  { 0xf8, 0x7f, 0xc0, 0x00 },  { 0x7e, 0xea, 0x20, 0x00 } }, // 00000111 19xxj.key
  {     0x04,       { 0x04, 0xc2, 0xa4, 0x02, 0x02, 0x20 },  { 0x04, 0x3c, 0x2f, 0xeb },  { 0xe0, 0x63, 0x81, 0xc0 } }, // 00001000 19xxu.key
  {     0x07,       { 0x70, 0x30, 0x04, 0x00, 0x10, 0x0b },  { 0xe0, 0x42, 0x71, 0x47 },  { 0x40, 0xca, 0xe5, 0xe4 } }, // 00001001 armwar.key
  {     0x07,       { 0x70, 0x30, 0x04, 0x00, 0x10, 0x08 },  { 0x42, 0x71, 0x47, 0x40 },  { 0xca, 0xe5, 0xe7, 0xe0 } }, // 00001010 armwara.key
  {     0x07,       { 0x70, 0x30, 0x04, 0x00, 0x10, 0x0a },  { 0x04, 0x27, 0x14, 0x74 },  { 0x0c, 0xae, 0x5e, 0x7c } }, // 00001011 armwarb.key
  {     0x07,       { 0x70, 0x30, 0x04, 0x00, 0x10, 0x0a },  { 0x5e, 0x7e, 0x04, 0x27 },  { 0x14, 0x74, 0x0c, 0xac } }, // 00001100 armwaru.key
  {     0x05,       { 0x04, 0xc0, 0xb1, 0x20, 0x79, 0xab },  { 0x34, 0xd9, 0xed, 0x4a },  { 0x7b, 0xc4, 0x12, 0xa0 } }, // 00001101 avsp.key
  {     0x05,       { 0x04, 0xc0, 0xb1, 0x20, 0x79, 0xa8 },  { 0x3a, 0x69, 0x49, 0xd2 },  { 0xf7, 0x3c, 0x5a, 0x0c } }, // 00001110 avspa.key
  {     0x05,       { 0x04, 0xc0, 0xb1, 0x20, 0x79, 0xab },  { 0xe9, 0xc6, 0xcf, 0x09 },  { 0x42, 0x5b, 0x52, 0x38 } }, // 00001111 avsph.key
  {     0x05,       { 0x04, 0xc0, 0xb1, 0x20, 0x79, 0xa8 },  { 0x98, 0x13, 0xb2, 0x29 },  { 0x7c, 0x74, 0xee, 0x5c } }, // 00010000 avspj.key
  {     0x05,       { 0x04, 0xc0, 0xb1, 0x20, 0x79, 0xab },  { 0x11, 0x6b, 0xbc, 0xce },  { 0x44, 0x21, 0xbc, 0xb4 } }, // 00010001 avspu.key
  {     0x04,       { 0x04, 0xc3, 0xa4, 0x02, 0x32, 0x02 },  { 0xb5, 0x1c, 0xf4, 0xf3 },  { 0xa6, 0x9e, 0xa6, 0x2c } }, // 00010010 batcir.key
  {     0x04,       { 0x04, 0xc3, 0xa4, 0x02, 0x32, 0x03 },  { 0x8d, 0xd0, 0x44, 0x67 },  { 0x4c, 0x06, 0xe9, 0xe0 } }, // 00010011 batcira.key
  {     0x04,       { 0x04, 0xc3, 0xa4, 0x02, 0x32, 0x00 },  { 0x5c, 0x40, 0x00, 0x00 },  { 0x6e, 0xcb, 0xfc, 0x00 } }, // 00010100 batcirj.key
  {     0x01,       { 0x84, 0xc3, 0xa2, 0xc8, 0xf3, 0x6a },  { 0xb7, 0xa1, 0xc7, 0xf9 },  { 0x8d, 0x23, 0x7f, 0x2c } }, // 00010101 choko.key
  {     0x04,       { 0x04, 0xc3, 0xa4, 0x00, 0x23, 0x02 },  { 0x0f, 0x9c, 0x21, 0x08 },  { 0x17, 0xe5, 0xd1, 0x98 } }, // 00010110 csclub.key
  {     0x04,       { 0x04, 0xc3, 0xa4, 0x00, 0x23, 0x03 },  { 0xb2, 0x50, 0xb5, 0x65 },  { 0x51, 0xed, 0x9b, 0x20 } }, // 00010111 cscluba.key
  {     0x04,       { 0x04, 0xc3, 0xa4, 0x00, 0x23, 0x00 },  { 0x2c, 0xa7, 0xb9, 0xd3 },  { 0x94, 0x54, 0xa0, 0x3c } }, // 00011000 csclubh.key
  {     0x04,       { 0x04, 0xc3, 0xa4, 0x00, 0x23, 0x00 },  { 0x0f, 0x20, 0x0d, 0xaa },  { 0x9f, 0x42, 0xd1, 0x48 } }, // 00011001 csclubj.key
  {     0x05,       { 0x70, 0xc3, 0xfc, 0x00, 0x70, 0xc2 },  { 0x02, 0x42, 0x42, 0x83 },  { 0x0a, 0x69, 0x0a, 0x88 } }, // 00011010 cybots.key
  {     0x05,       { 0x70, 0xc3, 0xfc, 0x00, 0x70, 0xc3 },  { 0x0a, 0x69, 0x09, 0x4a },  { 0x02, 0x42, 0x42, 0x80 } }, // 00011011 cybotsj.key
  {     0x05,       { 0x70, 0xc3, 0xfc, 0x00, 0x70, 0xc2 },  { 0x69, 0x0a, 0xaa, 0x02 },  { 0x42, 0x42, 0x83, 0x08 } }, // 00011100 cybotsu.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x83 },  { 0x1b, 0xeb, 0xe0, 0x6c },  { 0xf5, 0x64, 0x47, 0x84 } }, // 00011101 ddsom.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x82 },  { 0x8b, 0x05, 0x03, 0xde },  { 0xcf, 0x56, 0x63, 0x84 } }, // 00011110 ddsoma.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x81 },  { 0xcf, 0xf6, 0x97, 0x3d },  { 0x07, 0x96, 0x4a, 0x38 } }, // 00011111 ddsomb.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x83 },  { 0x81, 0x9e, 0xc1, 0x22 },  { 0x89, 0x0b, 0x21, 0x08 } }, // 00100000 ddsomh.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x82 },  { 0x88, 0x25, 0x6a, 0x30 },  { 0xa5, 0x7d, 0x25, 0xd4 } }, // 00100001 ddsomj.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x81 },  { 0x79, 0xf2, 0xcf, 0x61 },  { 0x92, 0x83, 0xe4, 0x48 } }, // 00100010 ddsomu.key
  {     0x03,       { 0x78, 0xc2, 0x60, 0x20, 0x00, 0x0a },  { 0x6e, 0xb1, 0x4c, 0xa0 },  { 0x41, 0xff, 0x9b, 0x88 } }, // 00100011 ddtod.key
  {     0x03,       { 0x78, 0xc2, 0x60, 0x20, 0x00, 0x0a },  { 0x20, 0x82, 0x3a, 0x62 },  { 0x82, 0x0d, 0x4d, 0xec } }, // 00100100 ddtoda.key
  {     0x03,       { 0x78, 0xc2, 0x60, 0x20, 0x00, 0x08 },  { 0x1e, 0xee, 0x90, 0x5e },  { 0x60, 0x22, 0x5a, 0x60 } }, // 00100101 ddtodh.key
  {     0x03,       { 0x78, 0xc2, 0x60, 0x20, 0x00, 0x0a },  { 0xd1, 0x47, 0x5b, 0x3c },  { 0xe7, 0x9c, 0x22, 0x88 } }, // 00100110 ddtodj.key
  {     0x03,       { 0x78, 0xc2, 0x60, 0x20, 0x00, 0x08 },  { 0x3f, 0x5b, 0x38, 0x92 },  { 0xf0, 0xe6, 0x14, 0xdc } }, // 00100111 ddtodu.key
  {     0x05,       { 0xc9, 0xf4, 0x89, 0x36, 0x8d, 0xb7 },  { 0x52, 0xff, 0xa0, 0x50 },  { 0x09, 0xc7, 0x6e, 0xc0 } }, // 00101000 dimahoo.key
  {     0x05,       { 0xc9, 0xf4, 0x89, 0x36, 0x8d, 0xb6 },  { 0x25, 0x95, 0xfc, 0x36 },  { 0x6b, 0xd6, 0xba, 0x98 } }, // 00101001 dimahoou.key
  {     0x05,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x18 },  { 0x24, 0x34, 0x40, 0x00 },  { 0x57, 0x94, 0x6f, 0x20 } }, // 00101010 dstlk.key
  {     0x05,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x1a },  { 0x3a, 0x61, 0x11, 0x80 },  { 0x67, 0x06, 0xe8, 0x10 } }, // 00101011 dstlka.key
  {     0x05,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x19 },  { 0x56, 0x5a, 0x22, 0x01 },  { 0xfd, 0xf1, 0x89, 0x10 } }, // 00101100 dstlkh.key
  {     0x05,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x1a },  { 0x62, 0x39, 0x10, 0x20 },  { 0x3f, 0x5c, 0x05, 0xe0 } }, // 00101101 dstlku.key
  {     0x03,       { 0x70, 0x43, 0x00, 0x02, 0x8b, 0x38 },  { 0xa9, 0xb8, 0x65, 0x74 },  { 0xee, 0x30, 0x23, 0x24 } }, // 00101110 ecofghtr.key
  {     0x03,       { 0x70, 0x43, 0x00, 0x02, 0x8b, 0x39 },  { 0x30, 0xa9, 0xb8, 0x65 },  { 0x76, 0x30, 0x23, 0x24 } }, // 00101111 ecofghtra.key
  {     0x03,       { 0x70, 0x43, 0x00, 0x02, 0x8b, 0x3b },  { 0x0a, 0x9b, 0x86, 0x57 },  { 0x4e, 0x30, 0x23, 0x24 } }, // 00110000 ecofghtrh.key
  {     0x03,       { 0x70, 0x43, 0x00, 0x02, 0x8b, 0x3a },  { 0x9b, 0x86, 0x57, 0x4e },  { 0xde, 0x30, 0x23, 0x24 } }, // 00110001 ecofghtru.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x20 },  { 0x07, 0xac, 0x1b, 0x60 },  { 0x25, 0x1e, 0xd2, 0x9c } }, // 00110010 gigawing.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x20 },  { 0x5b, 0xb6, 0x35, 0x99 },  { 0x68, 0x55, 0x82, 0xb0 } }, // 00110011 gigawinga.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x22 },  { 0xe7, 0xfb, 0xdd, 0xa4 },  { 0x36, 0x8b, 0x9f, 0xc0 } }, // 00110100 gigawingb.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x22 },  { 0x87, 0x16, 0xc0, 0x0a },  { 0x21, 0x0b, 0x95, 0xcc } }, // 00110101 gigawingh.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x22 },  { 0xa4, 0x42, 0xbd, 0xf5 },  { 0x5a, 0x61, 0x92, 0x20 } }, // 00110110 gigawingj.key
  {     0x05,       { 0xc9, 0xf4, 0x89, 0x36, 0x8d, 0xb5 },  { 0x1b, 0x5e, 0x12, 0x18 },  { 0x69, 0xf7, 0xbf, 0xa4 } }, // 00110111 gmahou.key
  {     0x05,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x10 },  { 0xe4, 0x63, 0x15, 0xfe },  { 0xee, 0xe5, 0xb1, 0x68 } }, // 00111000 hsf2.key
  {     0x05,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x12 },  { 0xf1, 0x43, 0x1d, 0x54 },  { 0x31, 0xb2, 0xdc, 0x74 } }, // 00111001 hsf2a.key
  {     0x05,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x13 },  { 0x89, 0xf0, 0x77, 0x6c },  { 0x1f, 0xd0, 0x6e, 0x98 } }, // 00111010 hsf2j.key
  {     0x05,       { 0x04, 0xc1, 0x29, 0xb3, 0x3a, 0xa2 },  { 0x6b, 0xf1, 0xbd, 0x25 },  { 0x9d, 0x50, 0x94, 0xd8 } }, // 00111011 jyangoku.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x02, 0x02, 0x38 },  { 0x2a, 0x98, 0xb2, 0xdc },  { 0xd4, 0xe0, 0x28, 0x28 } }, // 00111100 megaman2.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x02, 0x02, 0x3a },  { 0xa1, 0x42, 0x79, 0xaf },  { 0x51, 0x44, 0xa3, 0xf0 } }, // 00111101 megaman2a.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x02, 0x02, 0x38 },  { 0x2c, 0x21, 0x02, 0x91 },  { 0x4f, 0x3a, 0xa9, 0xb8 } }, // 00111110 megaman2h.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x80, 0xa6, 0x12 },  { 0x92, 0x90, 0x98, 0xa5 },  { 0x7f, 0x24, 0x4a, 0x80 } }, // 00111111 mmancp2u.key
  {     0x03,       { 0x0d, 0xb7, 0x88, 0xb7, 0xcd, 0x77 },  { 0xef, 0x98, 0x82, 0x0a },  { 0x7a, 0xf5, 0xe4, 0xd4 } }, // 01000000 mmatrix.key
  {     0x03,       { 0x0d, 0xb7, 0x88, 0xb7, 0xcd, 0x75 },  { 0x8a, 0xad, 0x7a, 0x6c },  { 0xd5, 0x2b, 0xb5, 0x50 } }, // 01000001 mmatrixa.key
  {     0x03,       { 0x0d, 0xb7, 0x88, 0xb7, 0xcd, 0x77 },  { 0x10, 0x66, 0xdd, 0x3a },  { 0xa5, 0xe0, 0x7e, 0xc8 } }, // 01000010 mmatrixj.key
  {     0x05,       { 0x84, 0xc2, 0xf8, 0xb3, 0x16, 0x47 },  { 0x61, 0x17, 0x8a, 0x9d },  { 0x8e, 0x0b, 0xbe, 0xa4 } }, // 01000011 mpang.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x83 },  { 0xa2, 0xea, 0xa7, 0x9d },  { 0x91, 0xde, 0x21, 0x60 } }, // 01000100 msh.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x82 },  { 0xf2, 0x8f, 0xa4, 0x72 },  { 0x03, 0x5d, 0x7a, 0x88 } }, // 01000101 msha.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x81 },  { 0x9c, 0x7f, 0x92, 0xd5 },  { 0xa0, 0x92, 0xa1, 0x78 } }, // 01000110 mshb.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x80 },  { 0x26, 0x0c, 0xe6, 0xb4 },  { 0x29, 0x68, 0xc8, 0xfc } }, // 01000111 mshh.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x81 },  { 0x0a, 0x12, 0xd6, 0x32 },  { 0x61, 0x04, 0x5e, 0x80 } }, // 01001000 mshj.key
  {     0x07,       { 0x04, 0xc1, 0x9a, 0x62, 0x60, 0x83 },  { 0x66, 0x33, 0xa1, 0x49 },  { 0xc9, 0x16, 0x83, 0x84 } }, // 01001001 mshu.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x23 },  { 0xf6, 0x93, 0xac, 0xe4 },  { 0x19, 0xd4, 0x87, 0x20 } }, // 01001010 mshvsf.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x20 },  { 0x6f, 0x7a, 0x48, 0xc2 },  { 0x19, 0x7f, 0x1d, 0x28 } }, // 01001011 mshvsfa.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x22 },  { 0x01, 0x1f, 0x3f, 0x6a },  { 0x4b, 0x8c, 0x59, 0x6c } }, // 01001100 mshvsfb.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x22 },  { 0xd2, 0x87, 0x57, 0x00 },  { 0x8f, 0xda, 0x25, 0xf8 } }, // 01001101 mshvsfh.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x20 },  { 0xb4, 0x1f, 0x91, 0x94 },  { 0x7e, 0x27, 0x0e, 0xe8 } }, // 01001110 mshvsfj.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x90, 0x20 },  { 0x53, 0x62, 0xbd, 0xce },  { 0x3a, 0x4a, 0xdb, 0x14 } }, // 01001111 mshvsfu.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x62, 0x12, 0x03 },  { 0x93, 0x7a, 0x58, 0xe1 },  { 0xed, 0x69, 0x00, 0x48 } }, // 01010000 mvsc.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x62, 0x12, 0x03 },  { 0xa2, 0xce, 0x82, 0x79 },  { 0x8d, 0xd4, 0x49, 0x3c } }, // 01010001 mvsca.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x62, 0x12, 0x00 },  { 0x67, 0xb5, 0x0e, 0x2b },  { 0x5d, 0xac, 0xb8, 0x40 } }, // 01010010 mvscb.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x62, 0x12, 0x03 },  { 0xdc, 0x63, 0x5a, 0x9f },  { 0x11, 0x78, 0xea, 0xe4 } }, // 01010011 mvsch.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x62, 0x12, 0x00 },  { 0x35, 0xf1, 0x23, 0xa4 },  { 0x72, 0x1a, 0x0f, 0xd4 } }, // 01010100 mvscj.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x62, 0x12, 0x02 },  { 0x80, 0x4e, 0x3d, 0xfb },  { 0x60, 0x8e, 0xd2, 0x58 } }, // 01010101 mvscu.key
  {     0x03,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x1a },  { 0x81, 0x68, 0x3f, 0x02 },  { 0x8a, 0x2e, 0x60, 0x20 } }, // 01010110 nwarr.key
  {     0x03,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x18 },  { 0x96, 0x18, 0x76, 0x70 },  { 0xc2, 0xc0, 0xa5, 0xc8 } }, // 01010111 nwarra.key
  {     0x03,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x18 },  { 0x11, 0x51, 0xb3, 0xb6 },  { 0x42, 0x39, 0x8f, 0xa0 } }, // 01011000 nwarrb.key
  {     0x03,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x18 },  { 0x10, 0x18, 0xa5, 0x52 },  { 0x03, 0x61, 0xa4, 0x8c } }, // 01011001 nwarrh.key
  {     0x03,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x19 },  { 0xe1, 0x7b, 0x63, 0xf0 },  { 0xc2, 0xf9, 0x48, 0x20 } }, // 01011010 nwarru.key
  {     0x05,       { 0x04, 0xc0, 0xbb, 0xe1, 0x22, 0xc3 },  { 0xac, 0xa7, 0xc4, 0x30 },  { 0x0f, 0x5d, 0x2f, 0xa4 } }, // 01011011 pfghtj.key
  {     0x07,       { 0x70, 0x30, 0x04, 0x00, 0x10, 0x09 },  { 0xe7, 0xe0, 0x42, 0x71 },  { 0x47, 0x40, 0xca, 0xe4 } }, // 01011100 pgear.key
  {     0x02,       { 0x04, 0xc2, 0x17, 0x1b, 0x2c, 0x77 },  { 0x3e, 0xc3, 0x23, 0xdd },  { 0x8c, 0x6d, 0x67, 0x18 } }, // 01011101 progear.key
  {     0x02,       { 0x04, 0xc2, 0x17, 0x1b, 0x2c, 0x75 },  { 0xeb, 0x64, 0xee, 0xfc },  { 0x52, 0x35, 0x46, 0x98 } }, // 01011110 progeara.key
  {     0x02,       { 0x04, 0xc2, 0x17, 0x1b, 0x2c, 0x75 },  { 0xf7, 0x8b, 0x7e, 0x71 },  { 0xa8, 0xed, 0xfb, 0xe4 } }, // 01011111 progearj.key
  {     0x01,       { 0x04, 0xc3, 0x39, 0x66, 0x3e, 0xa0 },  { 0xb5, 0xb2, 0xc0, 0x8d },  { 0x20, 0x7c, 0xa8, 0x14 } }, // 01100000 pzloop2.key
  {     0x07,       { 0x04, 0xc3, 0x3a, 0x63, 0x90, 0x40 },  { 0x67, 0x83, 0x59, 0xbf },  { 0x39, 0x5c, 0x80, 0x68 } }, // 01100001 qndream.key
  {     0x04,       { 0x70, 0x30, 0x04, 0x00, 0x10, 0x0b },  { 0x02, 0x48, 0x8b, 0xa3 },  { 0x93, 0x80, 0xa6, 0x60 } }, // 01100010 ringdest.key
  {     0x04,       { 0x70, 0x30, 0x04, 0x00, 0x10, 0x0b },  { 0x02, 0x12, 0x8b, 0xa3 },  { 0x93, 0x80, 0xa6, 0x60 } }, // 01100011 ringdesta.key
  {     0x04,       { 0x70, 0x30, 0x04, 0x00, 0x10, 0x0b },  { 0x00, 0x73, 0x8b, 0xa3 },  { 0x93, 0x80, 0xa6, 0x60 } }, // 01100100 ringdesth.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x80, 0xa6, 0x10 },  { 0x2d, 0x31, 0x7f, 0xb2 },  { 0x02, 0xaa, 0x13, 0x80 } }, // 01100101 rmancp2j.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x02, 0x02, 0x38 },  { 0x39, 0x22, 0xa8, 0x23 },  { 0x39, 0x4d, 0xe6, 0x30 } }, // 01100110 rockman2j.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x80, 0xa6, 0x13 },  { 0x61, 0x73, 0x90, 0x8d },  { 0xda, 0xea, 0x47, 0xc0 } }, // 01100111 sfa.key
  {     0x05,       { 0x04, 0xc2, 0x70, 0x33, 0x07, 0xa6 },  { 0x7e, 0x1f, 0x7f, 0x70 },  { 0xe7, 0xcd, 0x48, 0xfc } }, // 01101000 sfa2.key
  {     0x05,       { 0x04, 0xc2, 0x70, 0x33, 0x07, 0xa5 },  { 0x4a, 0x18, 0xbd, 0x45 },  { 0xa6, 0xf3, 0xf7, 0x60 } }, // 01101001 sfa2u.key
  {     0x05,       { 0x04, 0xc1, 0x18, 0xe0, 0x56, 0xbe },  { 0x0e, 0xec, 0x07, 0x90 },  { 0x1c, 0x4f, 0xf5, 0x58 } }, // 01101010 sfa3.key
  {     0x05,       { 0x04, 0xc1, 0x18, 0xe0, 0x56, 0xbd },  { 0xa5, 0x2d, 0xa2, 0x05 },  { 0x34, 0x0e, 0x10, 0xac } }, // 01101011 sfa3b.key
  {     0x05,       { 0x04, 0xc1, 0x18, 0xe0, 0x56, 0xbe },  { 0x1b, 0x17, 0xa3, 0x78 },  { 0xc7, 0xed, 0x10, 0x84 } }, // 01101100 sfa3h.key
  {     0x05,       { 0x04, 0xc1, 0x18, 0xe0, 0x56, 0xbc },  { 0x49, 0x30, 0xa7, 0x9a },  { 0x9c, 0x3f, 0x77, 0x9c } }, // 01101101 sfa3u.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x80, 0xa6, 0x10 },  { 0x60, 0x0b, 0xcf, 0xa5 },  { 0xb2, 0xd5, 0xf6, 0x90 } }, // 01101110 sfau.key
  {     0x05,       { 0x04, 0xc2, 0x70, 0x33, 0x07, 0xa7 },  { 0x84, 0xa4, 0x7b, 0x08 },  { 0xbc, 0x5d, 0x0f, 0xd4 } }, // 01101111 sfz2a.key
  {     0x05,       { 0x04, 0xc3, 0x39, 0xc4, 0x22, 0x25 },  { 0x96, 0x11, 0x80, 0x80 },  { 0x2c, 0x0d, 0x3a, 0x3c } }, // 01110000 sfz2al.key
  {     0x05,       { 0x04, 0xc3, 0x39, 0xc4, 0x22, 0x24 },  { 0xe3, 0xd7, 0x07, 0xfc },  { 0x51, 0x4a, 0xcf, 0x38 } }, // 01110001 sfz2alb.key
  {     0x05,       { 0x04, 0xc3, 0x39, 0xc4, 0x22, 0x25 },  { 0x8c, 0x40, 0xc0, 0x10 },  { 0xfb, 0x6a, 0x3e, 0xa4 } }, // 01110010 sfz2alh.key
  {     0x05,       { 0x04, 0xc3, 0x39, 0xc4, 0x22, 0x26 },  { 0xc8, 0xd1, 0x40, 0x14 },  { 0x44, 0xc2, 0x8a, 0x64 } }, // 01110011 sfz2alj.key
  {     0x05,       { 0x04, 0xc2, 0x70, 0x33, 0x07, 0xa5 },  { 0xd3, 0x74, 0x7e, 0x1a },  { 0x66, 0x8b, 0x20, 0xd4 } }, // 01110100 sfz2b.key
  {     0x05,       { 0x04, 0xc2, 0x70, 0x33, 0x07, 0xa7 },  { 0xe4, 0x43, 0x7a, 0x69 },  { 0x0a, 0xd1, 0x46, 0x7c } }, // 01110101 sfz2h.key
  {     0x05,       { 0x04, 0xc2, 0x70, 0x33, 0x07, 0xa6 },  { 0x22, 0x13, 0xb9, 0x6e },  { 0x65, 0xf8, 0xbf, 0x04 } }, // 01110110 sfz2j.key
  {     0x05,       { 0x04, 0xc2, 0x70, 0x33, 0x07, 0xa4 },  { 0xed, 0x8b, 0x7a, 0x94 },  { 0xe4, 0x7f, 0x53, 0x1c } }, // 01110111 sfz2n.key
  {     0x05,       { 0x04, 0xc1, 0x18, 0xe0, 0x56, 0xbd },  { 0xf8, 0xd0, 0x9c, 0x96 },  { 0x03, 0x27, 0x42, 0x64 } }, // 01111000 sfz3a.key
  {     0x05,       { 0x04, 0xc1, 0x18, 0xe0, 0x56, 0xbe },  { 0x7a, 0xd1, 0xf4, 0xc3 },  { 0x00, 0x7e, 0x4a, 0xf8 } }, // 01111001 sfz3j.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x80, 0xa6, 0x13 },  { 0xf9, 0x67, 0x61, 0x18 },  { 0x42, 0x8e, 0xf0, 0x9c } }, // 01111010 sfza.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x80, 0xa6, 0x10 },  { 0x05, 0x8d, 0x25, 0x7b },  { 0x2f, 0x6a, 0x0b, 0xdc } }, // 01111011 sfzb.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x80, 0xa6, 0x12 },  { 0x2f, 0xc9, 0x14, 0xea },  { 0x71, 0xc3, 0x5b, 0x84 } }, // 01111100 sfzh.key
  {     0x05,       { 0x04, 0xc0, 0x9a, 0x80, 0xa6, 0x12 },  { 0x8b, 0xc1, 0xe5, 0x0d },  { 0x79, 0xa3, 0x36, 0xc4 } }, // 01111101 sfzj.key
  {     0x05,       { 0x04, 0xc0, 0xbb, 0xe1, 0x22, 0xc1 },  { 0xfb, 0x77, 0xc1, 0xe9 },  { 0xba, 0x4b, 0x10, 0x84 } }, // 01111110 sgemf.key
  {     0x05,       { 0x04, 0xc0, 0xbb, 0xe1, 0x22, 0xc3 },  { 0x26, 0x5d, 0xf8, 0x8d },  { 0x52, 0x0b, 0x42, 0x40 } }, // 01111111 sgemfa.key
  {     0x05,       { 0x04, 0xc0, 0xbb, 0xe1, 0x22, 0xc0 },  { 0x91, 0x2f, 0xc4, 0xfb },  { 0x63, 0x97, 0x1a, 0x04 } }, // 10000000 sgemfh.key
  {     0x04,       { 0x70, 0x30, 0x04, 0x00, 0x10, 0x0b },  { 0x00, 0xa3, 0x03, 0xa2 },  { 0x41, 0x00, 0xa6, 0x60 } }, // 10000001 smbomb.key
  {     0x08,       { 0x04, 0xc2, 0x70, 0x32, 0x60, 0x67 },  { 0x9d, 0xe1, 0x06, 0xaa },  { 0x43, 0xd9, 0x1e, 0xec } }, // 10000010 spf2t.key
  {     0x08,       { 0x04, 0xc2, 0x70, 0x32, 0x60, 0x67 },  { 0x7c, 0xb3, 0xc1, 0xaf },  { 0x56, 0x1c, 0x48, 0xe4 } }, // 10000011 spf2ta.key
  {     0x08,       { 0x04, 0xc2, 0x70, 0x32, 0x60, 0x65 },  { 0xb6, 0x87, 0xc5, 0x13 },  { 0x54, 0xc6, 0xde, 0x28 } }, // 10000100 spf2th.key
  {     0x08,       { 0x04, 0xc2, 0x70, 0x32, 0x60, 0x66 },  { 0x86, 0x0f, 0xc2, 0xf8 },  { 0x2b, 0x85, 0x58, 0x38 } }, // 10000101 spf2tu.key
  {     0x08,       { 0x04, 0xc2, 0x70, 0x32, 0x60, 0x67 },  { 0xfd, 0xba, 0x42, 0x5d },  { 0x6b, 0x04, 0xd2, 0x34 } }, // 10000110 spf2xj.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x12 },  { 0x03, 0xde, 0xcf, 0x56 },  { 0x47, 0x9a, 0x8b, 0x10 } }, // 10000111 ssf2.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x11 },  { 0x20, 0x3d, 0xec, 0xf5 },  { 0x64, 0x79, 0xa8, 0xb0 } }, // 10001000 ssf2a.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x10 },  { 0xb1, 0x20, 0x3d, 0xec },  { 0xf5, 0x64, 0x79, 0xa8 } }, // 10001001 ssf2h.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x13 },  { 0xde, 0xcf, 0x56, 0x47 },  { 0x9a, 0x8b, 0x12, 0x00 } }, // 10001010 ssf2j.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x10 },  { 0xf0, 0xa3, 0x2d, 0xa9 },  { 0x03, 0x05, 0xc8, 0xa4 } }, // 10001011 ssf2t.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x12 },  { 0x38, 0xa1, 0x49, 0x99 },  { 0x00, 0x2c, 0x8c, 0xa4 } }, // 10001100 ssf2ta.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x13 },  { 0x9a, 0x8b, 0x12, 0x03 },  { 0xde, 0xcf, 0x56, 0x44 } }, // 10001101 ssf2tb.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x10 },  { 0x79, 0xa8, 0xb1, 0x20 },  { 0x3d, 0xec, 0xf5, 0x64 } }, // 10001110 ssf2tba.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x11 },  { 0x64, 0x79, 0xa8, 0xb1 },  { 0x20, 0x3d, 0xec, 0xf4 } }, // 10001111 ssf2tbh.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x12 },  { 0x8b, 0x12, 0x03, 0xde },  { 0xcf, 0x56, 0x47, 0x98 } }, // 10010000 ssf2tbj.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x11 },  { 0xa8, 0xb1, 0x20, 0x3d },  { 0xec, 0xf5, 0x64, 0x78 } }, // 10010001 ssf2tbu.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x11 },  { 0x30, 0xa2, 0x12, 0xb9 },  { 0x02, 0xd1, 0xc0, 0xa4 } }, // 10010010 ssf2th.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x13 },  { 0xf0, 0xa3, 0xb8, 0xc9 },  { 0x02, 0x45, 0x7c, 0xa4 } }, // 10010011 ssf2tu.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x10 },  { 0x3d, 0xec, 0xf5, 0x64 },  { 0x79, 0xa8, 0xb1, 0x20 } }, // 10010100 ssf2u.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x11 },  { 0xc0, 0xa0, 0xd6, 0x81 },  { 0x03, 0xa9, 0x50, 0xa4 } }, // 10010101 ssf2xj.key
  {     0x00,       { 0x70, 0x43, 0x80, 0x00, 0x00, 0x11 },  { 0x10, 0xa2, 0xcd, 0x79 },  { 0x03, 0x50, 0xf0, 0xa4 } }, // 10010110 ssf2xjr1r.key
  {     0x03,       { 0x70, 0x43, 0x00, 0x02, 0x8b, 0x39 },  { 0xb8, 0x65, 0x74, 0xed },  { 0xfe, 0x30, 0x23, 0x24 } }, // 10010111 uecology.key
  {     0x05,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x19 },  { 0xd6, 0x60, 0x59, 0x00 },  { 0x80, 0x43, 0x4f, 0xdc } }, // 10011000 vampj.key
  {     0x05,       { 0x04, 0xc1, 0x25, 0x80, 0x1b, 0x80 },  { 0x63, 0xc2, 0x35, 0x93 },  { 0x17, 0x5e, 0x0d, 0xb0 } }, // 10011001 vhunt2.key
  {     0x03,       { 0x70, 0x40, 0x00, 0x00, 0x1a, 0x19 },  { 0x3f, 0xae, 0x5c, 0x97 },  { 0x0d, 0x36, 0xb2, 0x20 } }, // 10011010 vhuntj.key
  {     0x05,       { 0x04, 0xc1, 0x59, 0x3b, 0xd7, 0x48 },  { 0x3b, 0x75, 0x76, 0x3a },  { 0x04, 0x6a, 0xcc, 0x1c } }, // 10011011 vsav.key
  {     0x05,       { 0x04, 0xc1, 0x25, 0x80, 0x1b, 0x83 },  { 0xed, 0xe3, 0xb0, 0x18 },  { 0xbc, 0x9e, 0x05, 0xac } }, // 10011100 vsav2.key
  {     0x05,       { 0x04, 0xc1, 0x59, 0x3b, 0xd7, 0x48 },  { 0xd5, 0x20, 0xb4, 0x24 },  { 0x32, 0x65, 0xdf, 0x88 } }, // 10011101 vsava.key
  {     0x05,       { 0x04, 0xc1, 0x59, 0x3b, 0xd7, 0x48 },  { 0x77, 0x2e, 0xf0, 0xf1 },  { 0x51, 0x17, 0xbf, 0x60 } }, // 10011110 vsavb.key
  {     0x05,       { 0x04, 0xc1, 0x59, 0x3b, 0xd7, 0x4a },  { 0xed, 0x57, 0xb7, 0x2e },  { 0xc4, 0xfb, 0x2d, 0x34 } }, // 10011111 vsavh.key
  {     0x05,       { 0x04, 0xc1, 0x59, 0x3b, 0xd7, 0x4a },  { 0x76, 0x04, 0x74, 0x97 },  { 0x31, 0xcb, 0xc5, 0x7c } }, // 10100000 vsavj.key
  {     0x05,       { 0x04, 0xc1, 0x59, 0x3b, 0xd7, 0x49 },  { 0x9f, 0x01, 0xf3, 0xa9 },  { 0xdc, 0x15, 0xd1, 0x94 } }, // 10100001 vsavu.key
  {     0x05,       { 0x04, 0xc1, 0x3a, 0x62, 0x03, 0x02 },  { 0x29, 0x00, 0x7f, 0xa4 },  { 0x96, 0xdd, 0x8f, 0x70 } }, // 10100010 xmcota.key
  {     0x05,       { 0x04, 0xc1, 0x3a, 0x62, 0x03, 0x00 },  { 0xe1, 0x87, 0xf3, 0x6d },  { 0x1c, 0x96, 0xa7, 0x80 } }, // 10100011 xmcotaa.key
  {     0x05,       { 0x04, 0xc1, 0x3a, 0x62, 0x03, 0x00 },  { 0xa9, 0x65, 0xac, 0xe3 },  { 0x10, 0x1f, 0xf7, 0x84 } }, // 10100100 xmcotab.key
  {     0x05,       { 0x04, 0xc1, 0x3a, 0x62, 0x03, 0x03 },  { 0xa1, 0x35, 0xa4, 0x14 },  { 0xb0, 0xec, 0x5e, 0xbc } }, // 10100101 xmcotah.key
  {     0x05,       { 0x04, 0xc1, 0x3a, 0x62, 0x03, 0x01 },  { 0xe6, 0xcf, 0x47, 0xd6 },  { 0xa3, 0x39, 0x01, 0x88 } }, // 10100110 xmcotaj.key
  {     0x05,       { 0x04, 0xc1, 0x3a, 0x62, 0x03, 0x03 },  { 0x4b, 0xda, 0x00, 0x66 },  { 0xcd, 0xfa, 0x95, 0x30 } }, // 10100111 xmcotau.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x93, 0x01 },  { 0x5c, 0xd5, 0x37, 0xf2 },  { 0x62, 0x87, 0xce, 0xf4 } }, // 10101000 xmvsf.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x93, 0x00 },  { 0x26, 0xdf, 0x56, 0x61 },  { 0xf0, 0xfc, 0x70, 0xb8 } }, // 10101001 xmvsfa.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x93, 0x00 },  { 0x3c, 0x86, 0x10, 0x35 },  { 0x49, 0xe6, 0xd5, 0xc4 } }, // 10101010 xmvsfb.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x93, 0x03 },  { 0xb2, 0x25, 0x7d, 0x08 },  { 0x2d, 0x37, 0xeb, 0x04 } }, // 10101011 xmvsfh.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x93, 0x00 },  { 0xd7, 0x3b, 0x02, 0x10 },  { 0xf7, 0x27, 0xec, 0x70 } }, // 10101100 xmvsfj.key
  {     0x07,       { 0x04, 0xc1, 0x3a, 0x63, 0x93, 0x02 },  { 0x0f, 0x72, 0x98, 0x7d },  { 0x2f, 0x03, 0x4f, 0xc8 } }, // 10101101 xmvsfu.key

  // also used with invalid jumper configs
  {     0x09,       { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },  { 0xff, 0xff, 0xff, 0xff },  { 0xff, 0xff, 0xff, 0xff } }, // 10101110 phoenix.key
    
};

void setup() { 

  delay(500);
  
  // set the programming pins as outputs
  PORTB.DIR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;

  // enable pull-ups on jumper pins
  PORTA.PIN0CTRL |= PORT_PULLUPEN_bm;
  PORTA.PIN1CTRL |= PORT_PULLUPEN_bm;
  PORTA.PIN2CTRL |= PORT_PULLUPEN_bm;
  PORTA.PIN3CTRL |= PORT_PULLUPEN_bm;
  PORTA.PIN4CTRL |= PORT_PULLUPEN_bm;
  PORTA.PIN5CTRL |= PORT_PULLUPEN_bm;
  PORTA.PIN6CTRL |= PORT_PULLUPEN_bm;
  PORTA.PIN7CTRL |= PORT_PULLUPEN_bm;

  // go into programming mode
  CLR_CLOCK;
  CLR_DATA;
  SET_SETUP;
  CLR_RESET;

  delay(100);

  uint8_t game_num = read_jumpers();
  if(game_num > MAX_GAME_NUM) {
    game_num = MAX_GAME_NUM;
  }

  for(uint8_t i = 0; i < CONFIG_SIZE; i++) {
    send_byte(config_table[game_data[game_num].config_table_index][i]);
  }

  for(uint8_t i = 0; i < WATCHDOG_SIZE; i++) {
    send_byte(game_data[game_num].watchdog[i]);
  }

  for(uint8_t i = 0; i < KEY_SIZE; i++) {
    send_byte(game_data[game_num].key2[i]);
  }

  for(uint8_t i = 0; i < KEY_SIZE; i++) {
    send_byte(game_data[game_num].key1[i]);
  }

  // exit programming mode
  CLR_SETUP;
  SET_RESET;
  CLR_CLOCK;
  CLR_DATA;

  // set all output pins as inputs
  PORTB.DIR = 0;

  // power down
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();    
}

void loop() {
}

void send_byte(uint8_t data) {
  for(int8_t bit_num = 7; bit_num >= 0;bit_num--) { 
    uint8_t bit = data & masks[bit_num];
    if(bit) {
      SET_DATA;
    } else {
      CLR_DATA;
    }
    
    SET_CLOCK;
    CLR_CLOCK;
  }
}

// JP1 = PA7 = bit7
// JP2 = PA6 = bit6
// JP3 = PA5 = bit5
// JP4 = PA6 = bit4
// JP5 = PA0 = bit3
// JP6 = PA1 = bit2
// JP7 = PA2 = bit1
// JP8 = PA3 = bit0
uint8_t read_jumpers() {
  uint8_t jumpers = 0;
  uint8_t raw;
  uint8_t bit;

  // negate the bits since we are using
  // pull-ups, jumper'd will be low
  raw = PORTA.IN ^ 0xff;

  // upper nible is in the correct order, but lower isnt
  jumpers = raw & 0xf0;

  // reverse the order of the lower nibble
  bit = (raw & 0x1) << 3;
  jumpers = jumpers | bit;

  bit = (raw & 0x2) << 1;
  jumpers = jumpers | bit;

  bit = (raw & 0x4) >> 1;
  jumpers = jumpers | bit;

  bit = (raw & 0x8) >> 3;
  jumpers = jumpers | bit;

  return jumpers;
}
