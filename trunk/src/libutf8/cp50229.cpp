//---------------------------------------------------------------------------
#include <adicpp/lconfig.h>
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
#if defined(EMBED_CP50229_SUPPORT) || defined(EMBED_ALL_CP)
extern const unsigned char cp50229s2utf8s[188] = {
  0x35, 0xC4, 0x00, 0x00, 0x0D, 0x00, 0x0D, 0x00,
  0x00, 0x10, 0x1A, 0x10, 0x00, 0x1C, 0x7F, 0x1C,
  0x00, 0x80, 0x81, 0x02, 0x04, 0x86, 0x87, 0x20,
  0x20, 0x91, 0x92, 0x18, 0x20, 0x93, 0x94, 0x1C,
  0x20, 0x96, 0x97, 0x13, 0x20, 0xA6, 0xA7, 0xA6,
  0x00, 0xAB, 0xAE, 0xAB, 0x00, 0xB0, 0xB1, 0xB0,
  0x00, 0xB5, 0xB7, 0xB5, 0x00, 0xC0, 0xFF, 0x10,
  0x04, 0x2B, 0x82, 0x1A, 0x20, 0x83, 0x53, 0x04,
  0x84, 0x1E, 0x20, 0x85, 0x26, 0x20, 0x88, 0xAC,
  0x20, 0x89, 0x30, 0x20, 0x8A, 0x09, 0x04, 0x8B,
  0x39, 0x20, 0x8C, 0x0A, 0x04, 0x8D, 0x0C, 0x04,
  0x8E, 0x0B, 0x04, 0x8F, 0x0F, 0x04, 0x90, 0x52,
  0x04, 0x95, 0x22, 0x20, 0x98, 0x98, 0x00, 0x99,
  0x22, 0x21, 0x9A, 0x59, 0x04, 0x9B, 0x3A, 0x20,
  0x9C, 0x5A, 0x04, 0x9D, 0x5C, 0x04, 0x9E, 0x5B,
  0x04, 0x9F, 0x5F, 0x04, 0xA0, 0xA0, 0x00, 0xA1,
  0x0E, 0x04, 0xA2, 0x5E, 0x04, 0xA3, 0x08, 0x04,
  0xA4, 0xA4, 0x00, 0xA5, 0x90, 0x04, 0xA8, 0x01,
  0x04, 0xA9, 0xA9, 0x00, 0xAA, 0x04, 0x04, 0xAF,
  0x07, 0x04, 0xB2, 0x06, 0x04, 0xB3, 0x56, 0x04,
  0xB4, 0x91, 0x04, 0xB8, 0x51, 0x04, 0xB9, 0x16,
  0x21, 0xBA, 0x54, 0x04, 0xBB, 0xBB, 0x00, 0xBC,
  0x58, 0x04, 0xBD, 0x05, 0x04, 0xBE, 0x55, 0x04,
  0xBF, 0x57, 0x04, 0x00
};
//---------------------------------------------------------------------------
extern const unsigned char utf8s2cp50229s[73] = {
  0x05, 0x3F, 0x01, 0x01, 0x00, 0x00, 0x9F, 0x00,
  0x00, 0x01, 0x02, 0x00, 0x00, 0x9F, 0x00, 0x00,
  0x00, 0x01, 0x03, 0x00, 0x00, 0x9F, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x9F, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00,
  0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
  0xFF, 0x02, 0x00, 0x01, 0xF0, 0xF8, 0xA0, 0xF1,
  0xF8, 0xFF, 0x00, 0x00, 0x02, 0x00, 0x00, 0x03,
  0x00, 0x00, 0x04, 0x00, 0x00, 0x05, 0xFF, 0xFF,
  0xFF
};
#endif
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
