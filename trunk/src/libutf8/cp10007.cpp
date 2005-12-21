//---------------------------------------------------------------------------
#include <adicpp/lconfig.h>
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
#if defined(EMBED_CP10007_SUPPORT) || defined(EMBED_ALL_CP)
extern const unsigned char cp10007s2utf8s[204] = {
  0x17, 0x27, 0x00, 0x00, 0x08, 0x00, 0x7F, 0x00,
  0x00, 0x80, 0x9F, 0x10, 0x04, 0xA2, 0xA3, 0xA2,
  0x00, 0xB2, 0xB3, 0x64, 0x22, 0xD0, 0xD1, 0x13,
  0x20, 0xD2, 0xD3, 0x1C, 0x20, 0xD4, 0xD5, 0x18,
  0x20, 0xE0, 0xFE, 0x30, 0x04, 0x37, 0xA0, 0x20,
  0x20, 0xA1, 0xB0, 0x00, 0xA4, 0xA7, 0x00, 0xA5,
  0x22, 0x20, 0xA6, 0xB6, 0x00, 0xA7, 0x06, 0x04,
  0xA8, 0xAE, 0x00, 0xA9, 0xA9, 0x00, 0xAA, 0x22,
  0x21, 0xAB, 0x02, 0x04, 0xAC, 0x52, 0x04, 0xAD,
  0x60, 0x22, 0xAE, 0x03, 0x04, 0xAF, 0x53, 0x04,
  0xB0, 0x1E, 0x22, 0xB1, 0xB1, 0x00, 0xB4, 0x56,
  0x04, 0xB5, 0xB5, 0x00, 0xB6, 0x02, 0x22, 0xB7,
  0x08, 0x04, 0xB8, 0x04, 0x04, 0xB9, 0x54, 0x04,
  0xBA, 0x07, 0x04, 0xBB, 0x57, 0x04, 0xBC, 0x09,
  0x04, 0xBD, 0x59, 0x04, 0xBE, 0x0A, 0x04, 0xBF,
  0x5A, 0x04, 0xC0, 0x58, 0x04, 0xC1, 0x05, 0x04,
  0xC2, 0xAC, 0x00, 0xC3, 0x1A, 0x22, 0xC4, 0x92,
  0x01, 0xC5, 0x48, 0x22, 0xC6, 0x06, 0x22, 0xC7,
  0xAB, 0x00, 0xC8, 0xBB, 0x00, 0xC9, 0x26, 0x20,
  0xCA, 0xA0, 0x00, 0xCB, 0x0B, 0x04, 0xCC, 0x5B,
  0x04, 0xCD, 0x0C, 0x04, 0xCE, 0x5C, 0x04, 0xCF,
  0x55, 0x04, 0xD6, 0xF7, 0x00, 0xD7, 0x1E, 0x20,
  0xD8, 0x0E, 0x04, 0xD9, 0x5E, 0x04, 0xDA, 0x0F,
  0x04, 0xDB, 0x5F, 0x04, 0xDC, 0x16, 0x21, 0xDD,
  0x01, 0x04, 0xDE, 0x51, 0x04, 0xDF, 0x4F, 0x04,
  0xFF, 0xA4, 0x00, 0x00
};
//---------------------------------------------------------------------------
extern const unsigned char utf8s2cp10007s[217] = {
  0x01, 0x3F, 0x08, 0x01, 0x00, 0x00, 0x7F, 0x00,
  0x00, 0xA2, 0x00, 0xA3, 0x00, 0xA2, 0x10, 0x04,
  0x2F, 0x04, 0x80, 0x30, 0x04, 0x4E, 0x04, 0xE0,
  0x13, 0x20, 0x14, 0x20, 0xD0, 0x18, 0x20, 0x19,
  0x20, 0xD4, 0x1C, 0x20, 0x1D, 0x20, 0xD2, 0x64,
  0x22, 0x65, 0x22, 0xB2, 0xFF, 0xFF, 0x37, 0x00,
  0x01, 0xA0, 0x00, 0xCA, 0xA4, 0x00, 0xFF, 0xA7,
  0x00, 0xA4, 0xA9, 0x00, 0xA9, 0xAB, 0x00, 0xC7,
  0xAC, 0x00, 0xC2, 0xAE, 0x00, 0xA8, 0xB0, 0x00,
  0xA1, 0xB1, 0x00, 0xB1, 0xB5, 0x00, 0xB5, 0xB6,
  0x00, 0xA6, 0xBB, 0x00, 0xC8, 0xF7, 0x00, 0xD6,
  0x92, 0x01, 0xC4, 0x01, 0x04, 0xDD, 0x02, 0x04,
  0xAB, 0x03, 0x04, 0xAE, 0x04, 0x04, 0xB8, 0x05,
  0x04, 0xC1, 0x06, 0x04, 0xA7, 0x07, 0x04, 0xBA,
  0x08, 0x04, 0xB7, 0x09, 0x04, 0xBC, 0x0A, 0x04,
  0xBE, 0x0B, 0x04, 0xCB, 0x0C, 0x04, 0xCD, 0x0E,
  0x04, 0xD8, 0x0F, 0x04, 0xDA, 0x4F, 0x04, 0xDF,
  0x51, 0x04, 0xDE, 0x52, 0x04, 0xAC, 0x53, 0x04,
  0xAF, 0x54, 0x04, 0xB9, 0x55, 0x04, 0xCF, 0x56,
  0x04, 0xB4, 0x57, 0x04, 0xBB, 0x58, 0x04, 0xC0,
  0x59, 0x04, 0xBD, 0x5A, 0x04, 0xBF, 0x5B, 0x04,
  0xCC, 0x5C, 0x04, 0xCE, 0x5E, 0x04, 0xD9, 0x5F,
  0x04, 0xDB, 0x1E, 0x20, 0xD7, 0x20, 0x20, 0xA0,
  0x22, 0x20, 0xA5, 0x26, 0x20, 0xC9, 0x16, 0x21,
  0xDC, 0x22, 0x21, 0xAA, 0x02, 0x22, 0xB6, 0x06,
  0x22, 0xC6, 0x1A, 0x22, 0xC3, 0x1E, 0x22, 0xB0,
  0x48, 0x22, 0xC5, 0x60, 0x22, 0xAD, 0xFF, 0xFF,
  0xFF
};
#endif
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
