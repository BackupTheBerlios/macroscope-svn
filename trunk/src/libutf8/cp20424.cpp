//---------------------------------------------------------------------------
#include <adicpp/lconfig.h>
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
#if defined(EMBED_CP20424_SUPPORT) || defined(EMBED_ALL_CP)
extern const unsigned char cp20424s2utf8s[454] = {
  0xC8, 0x4F, 0x00, 0x00, 0x18, 0x00, 0x03, 0x00,
  0x00, 0x09, 0x0A, 0x8D, 0x00, 0x0B, 0x13, 0x0B,
  0x00, 0x18, 0x19, 0x18, 0x00, 0x1C, 0x1F, 0x1C,
  0x00, 0x20, 0x24, 0x80, 0x00, 0x28, 0x2C, 0x88,
  0x00, 0x2D, 0x2F, 0x05, 0x00, 0x30, 0x31, 0x90,
  0x00, 0x33, 0x36, 0x93, 0x00, 0x38, 0x3B, 0x98,
  0x00, 0x3C, 0x3D, 0x14, 0x00, 0x41, 0x49, 0xD0,
  0x05, 0x51, 0x59, 0xD9, 0x05, 0x62, 0x69, 0xE2,
  0x05, 0x6E, 0x6F, 0x3E, 0x00, 0x81, 0x89, 0x61,
  0x00, 0x91, 0x99, 0x6A, 0x00, 0xA2, 0xA9, 0x73,
  0x00, 0xB7, 0xB9, 0xBC, 0x00, 0xC1, 0xC9, 0x41,
  0x00, 0xD1, 0xD9, 0x4A, 0x00, 0xE2, 0xE9, 0x53,
  0x00, 0xF0, 0xF9, 0x30, 0x00, 0x75, 0x04, 0x9C,
  0x00, 0x05, 0x09, 0x00, 0x06, 0x86, 0x00, 0x07,
  0x7F, 0x00, 0x08, 0x97, 0x00, 0x14, 0x9D, 0x00,
  0x15, 0x85, 0x00, 0x16, 0x08, 0x00, 0x17, 0x87,
  0x00, 0x1A, 0x92, 0x00, 0x1B, 0x8F, 0x00, 0x25,
  0x0A, 0x00, 0x26, 0x17, 0x00, 0x27, 0x1B, 0x00,
  0x32, 0x16, 0x00, 0x37, 0x04, 0x00, 0x3E, 0x9E,
  0x00, 0x3F, 0x1A, 0x00, 0x40, 0x20, 0x00, 0x4A,
  0xA2, 0x00, 0x4B, 0x2E, 0x00, 0x4C, 0x3C, 0x00,
  0x4D, 0x28, 0x00, 0x4E, 0x2B, 0x00, 0x4F, 0x7C,
  0x00, 0x50, 0x26, 0x00, 0x5A, 0x21, 0x00, 0x5B,
  0x24, 0x00, 0x5C, 0x2A, 0x00, 0x5D, 0x29, 0x00,
  0x5E, 0x3B, 0x00, 0x5F, 0xAC, 0x00, 0x60, 0x2D,
  0x00, 0x61, 0x2F, 0x00, 0x6A, 0xA6, 0x00, 0x6B,
  0x2C, 0x00, 0x6C, 0x25, 0x00, 0x6D, 0x5F, 0x00,
  0x70, 0x1A, 0x00, 0x71, 0xEA, 0x05, 0x72, 0x1A,
  0x00, 0x73, 0x1A, 0x00, 0x74, 0xA0, 0x00, 0x75,
  0x1A, 0x00, 0x76, 0x1A, 0x00, 0x77, 0x1A, 0x00,
  0x78, 0x17, 0x20, 0x79, 0x60, 0x00, 0x7A, 0x3A,
  0x00, 0x7B, 0x23, 0x00, 0x7C, 0x40, 0x00, 0x7D,
  0x27, 0x00, 0x7E, 0x3D, 0x00, 0x7F, 0x22, 0x00,
  0x80, 0x1A, 0x00, 0x8A, 0xAB, 0x00, 0x8B, 0xBB,
  0x00, 0x8C, 0x1A, 0x00, 0x8D, 0x1A, 0x00, 0x8E,
  0x1A, 0x00, 0x8F, 0xB1, 0x00, 0x90, 0xB0, 0x00,
  0x9A, 0x1A, 0x00, 0x9B, 0x1A, 0x00, 0x9C, 0x1A,
  0x00, 0x9D, 0xB8, 0x00, 0x9E, 0x1A, 0x00, 0x9F,
  0xA4, 0x00, 0xA0, 0xB5, 0x00, 0xA1, 0x7E, 0x00,
  0xAA, 0x1A, 0x00, 0xAB, 0x1A, 0x00, 0xAC, 0x1A,
  0x00, 0xAD, 0x1A, 0x00, 0xAE, 0x1A, 0x00, 0xAF,
  0xAE, 0x00, 0xB0, 0x5E, 0x00, 0xB1, 0xA3, 0x00,
  0xB2, 0xA5, 0x00, 0xB3, 0x22, 0x20, 0xB4, 0xA9,
  0x00, 0xB5, 0xA7, 0x00, 0xB6, 0xB6, 0x00, 0xBA,
  0x5B, 0x00, 0xBB, 0x5D, 0x00, 0xBC, 0x3E, 0x20,
  0xBD, 0xA8, 0x00, 0xBE, 0xB4, 0x00, 0xBF, 0xD7,
  0x00, 0xC0, 0x7B, 0x00, 0xCA, 0xAD, 0x00, 0xCB,
  0x1A, 0x00, 0xCC, 0x1A, 0x00, 0xCD, 0x1A, 0x00,
  0xCE, 0x1A, 0x00, 0xCF, 0x1A, 0x00, 0xD0, 0x7D,
  0x00, 0xDA, 0xB9, 0x00, 0xDB, 0x1A, 0x00, 0xDC,
  0x1A, 0x00, 0xDD, 0x1A, 0x00, 0xDE, 0x1A, 0x00,
  0xDF, 0x1A, 0x00, 0xE0, 0x5C, 0x00, 0xE1, 0xF7,
  0x00, 0xEA, 0xB2, 0x00, 0xEB, 0x1A, 0x00, 0xEC,
  0x1A, 0x00, 0xED, 0x1A, 0x00, 0xEE, 0x1A, 0x00,
  0xEF, 0x1A, 0x00, 0xFA, 0xB3, 0x00, 0xFB, 0x1A,
  0x00, 0xFC, 0x1A, 0x00, 0xFD, 0x1A, 0x00, 0xFE,
  0x1A, 0x00, 0xFF, 0x9F, 0x00, 0x00
};
//---------------------------------------------------------------------------
extern const unsigned char utf8s2cp20424s[495] = {
  0x01, 0x6F, 0x1E, 0x01, 0x00, 0x00, 0x03, 0x00,
  0x00, 0x05, 0x00, 0x07, 0x00, 0x2D, 0x0B, 0x00,
  0x13, 0x00, 0x0B, 0x14, 0x00, 0x15, 0x00, 0x3C,
  0x18, 0x00, 0x19, 0x00, 0x18, 0x1C, 0x00, 0x1F,
  0x00, 0x1C, 0x30, 0x00, 0x39, 0x00, 0xF0, 0x41,
  0x00, 0x49, 0x00, 0xC1, 0x4A, 0x00, 0x52, 0x00,
  0xD1, 0x53, 0x00, 0x5A, 0x00, 0xE2, 0x61, 0x00,
  0x69, 0x00, 0x81, 0x6A, 0x00, 0x72, 0x00, 0x91,
  0x73, 0x00, 0x7A, 0x00, 0xA2, 0x80, 0x00, 0x84,
  0x00, 0x20, 0x88, 0x00, 0x8C, 0x00, 0x28, 0x8D,
  0x00, 0x8E, 0x00, 0x09, 0x90, 0x00, 0x91, 0x00,
  0x30, 0x93, 0x00, 0x96, 0x00, 0x33, 0x98, 0x00,
  0x9B, 0x00, 0x38, 0xBC, 0x00, 0xBE, 0x00, 0xB7,
  0xD0, 0x05, 0xD8, 0x05, 0x41, 0xD9, 0x05, 0xE1,
  0x05, 0x51, 0xE2, 0x05, 0xE9, 0x05, 0x62, 0x10,
  0xFF, 0x19, 0xFF, 0xF0, 0x21, 0xFF, 0x29, 0xFF,
  0xC1, 0x2A, 0xFF, 0x32, 0xFF, 0xD1, 0x33, 0xFF,
  0x3A, 0xFF, 0xE2, 0x41, 0xFF, 0x49, 0xFF, 0x81,
  0x4A, 0xFF, 0x52, 0xFF, 0x91, 0x53, 0xFF, 0x5A,
  0xFF, 0xA2, 0xFF, 0xFF, 0x6F, 0x00, 0x01, 0x04,
  0x00, 0x37, 0x08, 0x00, 0x16, 0x09, 0x00, 0x05,
  0x0A, 0x00, 0x25, 0x16, 0x00, 0x32, 0x17, 0x00,
  0x26, 0x1A, 0x00, 0x3F, 0x1B, 0x00, 0x27, 0x20,
  0x00, 0x40, 0x21, 0x00, 0x5A, 0x22, 0x00, 0x7F,
  0x23, 0x00, 0x7B, 0x24, 0x00, 0x5B, 0x25, 0x00,
  0x6C, 0x26, 0x00, 0x50, 0x27, 0x00, 0x7D, 0x28,
  0x00, 0x4D, 0x29, 0x00, 0x5D, 0x2A, 0x00, 0x5C,
  0x2B, 0x00, 0x4E, 0x2C, 0x00, 0x6B, 0x2D, 0x00,
  0x60, 0x2E, 0x00, 0x4B, 0x2F, 0x00, 0x61, 0x3A,
  0x00, 0x7A, 0x3B, 0x00, 0x5E, 0x3C, 0x00, 0x4C,
  0x3D, 0x00, 0x7E, 0x3E, 0x00, 0x6E, 0x40, 0x00,
  0x7C, 0x5B, 0x00, 0xBA, 0x5C, 0x00, 0xE0, 0x5D,
  0x00, 0xBB, 0x5E, 0x00, 0xB0, 0x5F, 0x00, 0x6D,
  0x60, 0x00, 0x79, 0x7B, 0x00, 0xC0, 0x7C, 0x00,
  0x4F, 0x7D, 0x00, 0xD0, 0x7E, 0x00, 0xA1, 0x7F,
  0x00, 0x07, 0x85, 0x00, 0x15, 0x86, 0x00, 0x06,
  0x87, 0x00, 0x17, 0x8F, 0x00, 0x1B, 0x92, 0x00,
  0x1A, 0x97, 0x00, 0x08, 0x9C, 0x00, 0x04, 0x9D,
  0x00, 0x14, 0x9E, 0x00, 0x3E, 0x9F, 0x00, 0xFF,
  0xA0, 0x00, 0x74, 0xA2, 0x00, 0x4A, 0xA3, 0x00,
  0xB1, 0xA4, 0x00, 0x9F, 0xA5, 0x00, 0xB2, 0xA6,
  0x00, 0x6A, 0xA7, 0x00, 0xB5, 0xA8, 0x00, 0xBD,
  0xA9, 0x00, 0xB4, 0xAB, 0x00, 0x8A, 0xAC, 0x00,
  0x5F, 0xAD, 0x00, 0xCA, 0xAE, 0x00, 0xAF, 0xB0,
  0x00, 0x90, 0xB1, 0x00, 0x8F, 0xB2, 0x00, 0xEA,
  0xB3, 0x00, 0xFA, 0xB4, 0x00, 0xBE, 0xB5, 0x00,
  0xA0, 0xB6, 0x00, 0xB6, 0xB8, 0x00, 0x9D, 0xB9,
  0x00, 0xDA, 0xBB, 0x00, 0x8B, 0xD7, 0x00, 0xBF,
  0xF7, 0x00, 0xE1, 0xEA, 0x05, 0x71, 0x17, 0x20,
  0x78, 0x22, 0x20, 0xB3, 0x3E, 0x20, 0xBC, 0x01,
  0xFF, 0x5A, 0x02, 0xFF, 0x7F, 0x03, 0xFF, 0x7B,
  0x04, 0xFF, 0x5B, 0x05, 0xFF, 0x6C, 0x06, 0xFF,
  0x50, 0x07, 0xFF, 0x7D, 0x08, 0xFF, 0x4D, 0x09,
  0xFF, 0x5D, 0x0A, 0xFF, 0x5C, 0x0B, 0xFF, 0x4E,
  0x0C, 0xFF, 0x6B, 0x0D, 0xFF, 0x60, 0x0E, 0xFF,
  0x4B, 0x0F, 0xFF, 0x61, 0x1A, 0xFF, 0x7A, 0x1B,
  0xFF, 0x5E, 0x1C, 0xFF, 0x4C, 0x1D, 0xFF, 0x7E,
  0x1E, 0xFF, 0x6E, 0x20, 0xFF, 0x7C, 0x3B, 0xFF,
  0xBA, 0x3C, 0xFF, 0xE0, 0x3D, 0xFF, 0xBB, 0x3E,
  0xFF, 0xB0, 0x3F, 0xFF, 0x6D, 0x40, 0xFF, 0x79,
  0x5B, 0xFF, 0xC0, 0x5C, 0xFF, 0x4F, 0x5D, 0xFF,
  0xD0, 0x5E, 0xFF, 0xA1, 0xFF, 0xFF, 0xFF
};
#endif
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
