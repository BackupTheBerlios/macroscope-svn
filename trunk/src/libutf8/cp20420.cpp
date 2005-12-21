//---------------------------------------------------------------------------
#include <adicpp/lconfig.h>
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
#if defined(EMBED_CP20420_SUPPORT) || defined(EMBED_ALL_CP)
extern const unsigned char cp20420s2utf8s[453] = {
  0xC4, 0x4F, 0x00, 0x00, 0x20, 0x00, 0x03, 0x00,
  0x00, 0x09, 0x0A, 0x8D, 0x00, 0x0B, 0x13, 0x0B,
  0x00, 0x18, 0x19, 0x18, 0x00, 0x1C, 0x1F, 0x1C,
  0x00, 0x20, 0x24, 0x80, 0x00, 0x28, 0x2C, 0x88,
  0x00, 0x2D, 0x2F, 0x05, 0x00, 0x30, 0x31, 0x90,
  0x00, 0x33, 0x36, 0x93, 0x00, 0x38, 0x3B, 0x98,
  0x00, 0x3C, 0x3D, 0x14, 0x00, 0x42, 0x43, 0x7C,
  0xFE, 0x46, 0x49, 0x80, 0xFE, 0x51, 0x52, 0x84,
  0xFE, 0x56, 0x58, 0x8D, 0xFE, 0x6E, 0x6F, 0x3E,
  0x00, 0x81, 0x89, 0x61, 0x00, 0x91, 0x99, 0x6A,
  0x00, 0x9A, 0xA0, 0xC9, 0xFE, 0xA2, 0xA9, 0x73,
  0x00, 0xAA, 0xAB, 0xD0, 0xFE, 0xB2, 0xB5, 0xF5,
  0xFE, 0xB8, 0xB9, 0xFB, 0xFE, 0xC1, 0xC9, 0x41,
  0x00, 0xD1, 0xD9, 0x4A, 0x00, 0xDA, 0xDE, 0xEF,
  0xFE, 0xE2, 0xE9, 0x53, 0x00, 0xEA, 0xEB, 0x61,
  0x06, 0xED, 0xEF, 0x63, 0x06, 0xF0, 0xF9, 0x30,
  0x00, 0xFB, 0xFE, 0x66, 0x06, 0x6A, 0x04, 0x9C,
  0x00, 0x05, 0x09, 0x00, 0x06, 0x86, 0x00, 0x07,
  0x7F, 0x00, 0x08, 0x97, 0x00, 0x14, 0x9D, 0x00,
  0x15, 0x85, 0x00, 0x16, 0x08, 0x00, 0x17, 0x87,
  0x00, 0x1A, 0x92, 0x00, 0x1B, 0x8F, 0x00, 0x25,
  0x0A, 0x00, 0x26, 0x17, 0x00, 0x27, 0x1B, 0x00,
  0x32, 0x16, 0x00, 0x37, 0x04, 0x00, 0x3E, 0x9E,
  0x00, 0x3F, 0x1A, 0x00, 0x40, 0x20, 0x00, 0x41,
  0xA0, 0x00, 0x44, 0x40, 0x06, 0x45, 0xFC, 0xF8,
  0x4A, 0xA2, 0x00, 0x4B, 0x2E, 0x00, 0x4C, 0x3C,
  0x00, 0x4D, 0x28, 0x00, 0x4E, 0x2B, 0x00, 0x4F,
  0x7C, 0x00, 0x50, 0x26, 0x00, 0x53, 0x1A, 0x00,
  0x54, 0x1A, 0x00, 0x55, 0x8B, 0xFE, 0x59, 0x91,
  0xFE, 0x5A, 0x21, 0x00, 0x5B, 0x24, 0x00, 0x5C,
  0x2A, 0x00, 0x5D, 0x29, 0x00, 0x5E, 0x3B, 0x00,
  0x5F, 0xAC, 0x00, 0x60, 0x2D, 0x00, 0x61, 0x2F,
  0x00, 0x62, 0x93, 0xFE, 0x63, 0x95, 0xFE, 0x64,
  0x97, 0xFE, 0x65, 0x99, 0xFE, 0x66, 0x9B, 0xFE,
  0x67, 0x9D, 0xFE, 0x68, 0x9F, 0xFE, 0x69, 0xA1,
  0xFE, 0x6A, 0xA6, 0x00, 0x6B, 0x2C, 0x00, 0x6C,
  0x25, 0x00, 0x6D, 0x5F, 0x00, 0x70, 0xA3, 0xFE,
  0x71, 0xA5, 0xFE, 0x72, 0xA7, 0xFE, 0x73, 0xA9,
  0xFE, 0x74, 0xAB, 0xFE, 0x75, 0xAD, 0xFE, 0x76,
  0xAF, 0xFE, 0x77, 0xF6, 0xF8, 0x78, 0xB3, 0xFE,
  0x79, 0x0C, 0x06, 0x7A, 0x3A, 0x00, 0x7B, 0x23,
  0x00, 0x7C, 0x40, 0x00, 0x7D, 0x27, 0x00, 0x7E,
  0x3D, 0x00, 0x7F, 0x22, 0x00, 0x80, 0xF5, 0xF8,
  0x8A, 0xB7, 0xFE, 0x8B, 0xF4, 0xF8, 0x8C, 0xBB,
  0xFE, 0x8D, 0xF7, 0xF8, 0x8E, 0xBF, 0xFE, 0x8F,
  0xC3, 0xFE, 0x90, 0xC7, 0xFE, 0xA1, 0xF7, 0x00,
  0xAC, 0xD3, 0xFE, 0xAD, 0xD5, 0xFE, 0xAE, 0xD7,
  0xFE, 0xAF, 0xD9, 0xFE, 0xB0, 0xDB, 0xFE, 0xB1,
  0xDD, 0xFE, 0xB6, 0x1A, 0x00, 0xB7, 0x1A, 0x00,
  0xBA, 0xDF, 0xFE, 0xBB, 0xE1, 0xFE, 0xBC, 0xE3,
  0xFE, 0xBD, 0xE5, 0xFE, 0xBE, 0xE7, 0xFE, 0xBF,
  0xE9, 0xFE, 0xC0, 0x1B, 0x06, 0xCA, 0xAD, 0x00,
  0xCB, 0xEB, 0xFE, 0xCC, 0x1A, 0x00, 0xCD, 0xEC,
  0xFE, 0xCE, 0x1A, 0x00, 0xCF, 0xED, 0xFE, 0xD0,
  0x1F, 0x06, 0xDF, 0x60, 0x06, 0xE0, 0xD7, 0x00,
  0xE1, 0x07, 0x20, 0xEC, 0x1A, 0x00, 0xFA, 0x1A,
  0x00, 0xFF, 0x9F, 0x00, 0x00
};
//---------------------------------------------------------------------------
extern const unsigned char utf8s2cp20420s[805] = {
  0x01, 0x6F, 0x2C, 0x01, 0x00, 0x00, 0x03, 0x00,
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
  0x9B, 0x00, 0x38, 0x21, 0x06, 0x22, 0x06, 0x46,
  0x26, 0x06, 0x27, 0x06, 0x55, 0x29, 0x06, 0x2A,
  0x06, 0x62, 0x2F, 0x06, 0x33, 0x06, 0x73, 0x37,
  0x06, 0x38, 0x06, 0x8F, 0x61, 0x06, 0x62, 0x06,
  0xEA, 0x63, 0x06, 0x65, 0x06, 0xED, 0x66, 0x06,
  0x69, 0x06, 0xFB, 0x7C, 0xFE, 0x7D, 0xFE, 0x42,
  0x80, 0xFE, 0x83, 0xFE, 0x46, 0x84, 0xFE, 0x85,
  0xFE, 0x51, 0x8D, 0xFE, 0x8F, 0xFE, 0x56, 0xA2,
  0xFE, 0xA3, 0xFE, 0x6F, 0xC9, 0xFE, 0xCF, 0xFE,
  0x9A, 0xD0, 0xFE, 0xD1, 0xFE, 0xAA, 0xEF, 0xFE,
  0xF3, 0xFE, 0xDA, 0xF5, 0xFE, 0xF8, 0xFE, 0xB2,
  0xFB, 0xFE, 0xFC, 0xFE, 0xB8, 0x10, 0xFF, 0x19,
  0xFF, 0xF0, 0x21, 0xFF, 0x29, 0xFF, 0xC1, 0x2A,
  0xFF, 0x32, 0xFF, 0xD1, 0x33, 0xFF, 0x3A, 0xFF,
  0xE2, 0x41, 0xFF, 0x49, 0xFF, 0x81, 0x4A, 0xFF,
  0x52, 0xFF, 0x91, 0x53, 0xFF, 0x5A, 0xFF, 0xA2,
  0xFF, 0xFF, 0xBF, 0x00, 0x01, 0x04, 0x00, 0x37,
  0x08, 0x00, 0x16, 0x09, 0x00, 0x05, 0x0A, 0x00,
  0x25, 0x16, 0x00, 0x32, 0x17, 0x00, 0x26, 0x1A,
  0x00, 0x3F, 0x1B, 0x00, 0x27, 0x20, 0x00, 0x40,
  0x21, 0x00, 0x5A, 0x22, 0x00, 0x7F, 0x23, 0x00,
  0x7B, 0x24, 0x00, 0x5B, 0x25, 0x00, 0x6C, 0x26,
  0x00, 0x50, 0x27, 0x00, 0x7D, 0x28, 0x00, 0x4D,
  0x29, 0x00, 0x5D, 0x2A, 0x00, 0x5C, 0x2B, 0x00,
  0x4E, 0x2C, 0x00, 0x6B, 0x2D, 0x00, 0x60, 0x2E,
  0x00, 0x4B, 0x2F, 0x00, 0x61, 0x3A, 0x00, 0x7A,
  0x3B, 0x00, 0x5E, 0x3C, 0x00, 0x4C, 0x3D, 0x00,
  0x7E, 0x3E, 0x00, 0x6E, 0x40, 0x00, 0x7C, 0x5F,
  0x00, 0x6D, 0x7C, 0x00, 0x4F, 0x7F, 0x00, 0x07,
  0x85, 0x00, 0x15, 0x86, 0x00, 0x06, 0x87, 0x00,
  0x17, 0x8F, 0x00, 0x1B, 0x92, 0x00, 0x1A, 0x97,
  0x00, 0x08, 0x9C, 0x00, 0x04, 0x9D, 0x00, 0x14,
  0x9E, 0x00, 0x3E, 0x9F, 0x00, 0xFF, 0xA0, 0x00,
  0x41, 0xA2, 0x00, 0x4A, 0xA6, 0x00, 0x6A, 0xAC,
  0x00, 0x5F, 0xAD, 0x00, 0xCA, 0xD7, 0x00, 0xE0,
  0xF7, 0x00, 0xA1, 0x0C, 0x06, 0x79, 0x1B, 0x06,
  0xC0, 0x1F, 0x06, 0xD0, 0x23, 0x06, 0x49, 0x24,
  0x06, 0x52, 0x28, 0x06, 0x58, 0x2B, 0x06, 0x65,
  0x2C, 0x06, 0x67, 0x2D, 0x06, 0x69, 0x2E, 0x06,
  0x71, 0x34, 0x06, 0x80, 0x35, 0x06, 0x8B, 0x36,
  0x06, 0x8D, 0x39, 0x06, 0x9A, 0x3A, 0x06, 0x9E,
  0x40, 0x06, 0x44, 0x41, 0x06, 0xAB, 0x42, 0x06,
  0xAD, 0x43, 0x06, 0xAF, 0x44, 0x06, 0xB1, 0x45,
  0x06, 0xBB, 0x46, 0x06, 0xBD, 0x47, 0x06, 0xBF,
  0x48, 0x06, 0xCF, 0x49, 0x06, 0xDA, 0x4A, 0x06,
  0xDC, 0x51, 0x06, 0x42, 0x60, 0x06, 0xDF, 0x6A,
  0x06, 0x6C, 0x6B, 0x06, 0x6B, 0x6C, 0x06, 0x4B,
  0x6D, 0x06, 0x5C, 0x07, 0x20, 0xE1, 0xF4, 0xF8,
  0x8B, 0xF5, 0xF8, 0x80, 0xF6, 0xF8, 0x77, 0xF7,
  0xF8, 0x8D, 0xFC, 0xF8, 0x45, 0x86, 0xFE, 0x58,
  0x8B, 0xFE, 0x5B, 0x8C, 0xFE, 0x5B, 0x90, 0xFE,
  0x5E, 0x91, 0xFE, 0x5F, 0x92, 0xFE, 0x5F, 0x93,
  0xFE, 0x68, 0x94, 0xFE, 0x68, 0x95, 0xFE, 0x69,
  0x96, 0xFE, 0x69, 0x97, 0xFE, 0x6A, 0x98, 0xFE,
  0x6A, 0x99, 0xFE, 0x6B, 0x9A, 0xFE, 0x6B, 0x9B,
  0xFE, 0x6C, 0x9C, 0xFE, 0x6C, 0x9D, 0xFE, 0x6D,
  0x9E, 0xFE, 0x6D, 0x9F, 0xFE, 0x6E, 0xA0, 0xFE,
  0x6E, 0xA1, 0xFE, 0x6F, 0xA4, 0xFE, 0x76, 0xA5,
  0xFE, 0x77, 0xA6, 0xFE, 0x77, 0xA7, 0xFE, 0x78,
  0xA8, 0xFE, 0x78, 0xA9, 0xFE, 0x79, 0xAA, 0xFE,
  0x79, 0xAB, 0xFE, 0x7A, 0xAC, 0xFE, 0x7A, 0xAD,
  0xFE, 0x7B, 0xAE, 0xFE, 0x7B, 0xAF, 0xFE, 0x7C,
  0xB0, 0xFE, 0x7C, 0xB3, 0xFE, 0x7E, 0xB4, 0xFE,
  0x7E, 0xB7, 0xFE, 0x90, 0xB8, 0xFE, 0x90, 0xBB,
  0xFE, 0x92, 0xBC, 0xFE, 0x92, 0xBF, 0xFE, 0x94,
  0xC0, 0xFE, 0x94, 0xC1, 0xFE, 0x95, 0xC2, 0xFE,
  0x95, 0xC3, 0xFE, 0x95, 0xC4, 0xFE, 0x95, 0xC5,
  0xFE, 0x96, 0xC6, 0xFE, 0x96, 0xC7, 0xFE, 0x96,
  0xC8, 0xFE, 0x96, 0xD2, 0xFE, 0xB1, 0xD3, 0xFE,
  0xB2, 0xD4, 0xFE, 0xB2, 0xD5, 0xFE, 0xB3, 0xD6,
  0xFE, 0xB3, 0xD7, 0xFE, 0xB4, 0xD8, 0xFE, 0xB4,
  0xD9, 0xFE, 0xB5, 0xDA, 0xFE, 0xB5, 0xDB, 0xFE,
  0xB6, 0xDC, 0xFE, 0xB6, 0xDD, 0xFE, 0xB7, 0xDE,
  0xFE, 0xB7, 0xDF, 0xFE, 0xC0, 0xE0, 0xFE, 0xC0,
  0xE1, 0xFE, 0xC1, 0xE2, 0xFE, 0xC1, 0xE3, 0xFE,
  0xC2, 0xE4, 0xFE, 0xC2, 0xE5, 0xFE, 0xC3, 0xE6,
  0xFE, 0xC3, 0xE7, 0xFE, 0xC4, 0xE8, 0xFE, 0xC4,
  0xE9, 0xFE, 0xC5, 0xEA, 0xFE, 0xC5, 0xEB, 0xFE,
  0xCB, 0xEC, 0xFE, 0xCD, 0xED, 0xFE, 0xD5, 0xEE,
  0xFE, 0xD5, 0xF4, 0xFE, 0xE4, 0x01, 0xFF, 0x5A,
  0x02, 0xFF, 0x7F, 0x03, 0xFF, 0x7B, 0x04, 0xFF,
  0x5B, 0x05, 0xFF, 0x6C, 0x06, 0xFF, 0x50, 0x07,
  0xFF, 0x7D, 0x08, 0xFF, 0x4D, 0x09, 0xFF, 0x5D,
  0x0A, 0xFF, 0x5C, 0x0B, 0xFF, 0x4E, 0x0C, 0xFF,
  0x6B, 0x0D, 0xFF, 0x60, 0x0E, 0xFF, 0x4B, 0x0F,
  0xFF, 0x61, 0x1A, 0xFF, 0x7A, 0x1B, 0xFF, 0x5E,
  0x1C, 0xFF, 0x4C, 0x1D, 0xFF, 0x7E, 0x1E, 0xFF,
  0x6E, 0x20, 0xFF, 0x7C, 0x3F, 0xFF, 0x6D, 0x5C,
  0xFF, 0x4F, 0xFF, 0xFF, 0xFF
};
#endif
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
