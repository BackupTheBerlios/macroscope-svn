//---------------------------------------------------------------------------
#include <adicpp/lconfig.h>
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
#if defined(EMBED_CP38598_SUPPORT) || defined(EMBED_ALL_CP)
extern const unsigned char cp38598s2utf8s[54] = {
  0xC6, 0x96, 0x00, 0x00, 0x08, 0x00, 0xA0, 0x00,
  0x00, 0xA2, 0xA9, 0xA2, 0x00, 0xAB, 0xAE, 0xAB,
  0x00, 0xB0, 0xB9, 0xB0, 0x00, 0xBB, 0xBE, 0xBB,
  0x00, 0xBF, 0xDE, 0x9D, 0xF7, 0xE0, 0xFA, 0xD0,
  0x05, 0xFB, 0xFF, 0xBD, 0xF7, 0x05, 0xA1, 0x9C,
  0xF7, 0xAA, 0xD7, 0x00, 0xAF, 0x3E, 0x20, 0xBA,
  0xF7, 0x00, 0xDF, 0x17, 0x20, 0x00
};
//---------------------------------------------------------------------------
extern const unsigned char utf8s2cp38598s[862] = {
  0x01, 0x3F, 0x0E, 0x01, 0x00, 0x00, 0xA0, 0x00,
  0x00, 0xA2, 0x00, 0xA9, 0x00, 0xA2, 0xAB, 0x00,
  0xAE, 0x00, 0xAB, 0xB0, 0x00, 0xB9, 0x00, 0xB0,
  0xBB, 0x00, 0xBE, 0x00, 0xBB, 0xD1, 0x00, 0xD2,
  0x00, 0x4E, 0xF1, 0x00, 0xF2, 0x00, 0x6E, 0x78,
  0x01, 0x79, 0x01, 0x59, 0xAE, 0x01, 0xAF, 0x01,
  0x54, 0xD0, 0x05, 0xEA, 0x05, 0xE0, 0x9D, 0xF7,
  0xBC, 0xF7, 0xBF, 0xBD, 0xF7, 0xC1, 0xF7, 0xFB,
  0x01, 0xFF, 0x1E, 0xFF, 0x21, 0x20, 0xFF, 0x5E,
  0xFF, 0x40, 0xFF, 0xFF, 0x04, 0x01, 0x01, 0xA1,
  0x00, 0x21, 0xAA, 0x00, 0x61, 0xBA, 0x00, 0x6F,
  0xC0, 0x00, 0x47, 0xC1, 0x00, 0x47, 0xC2, 0x00,
  0x47, 0xC3, 0x00, 0x47, 0xC4, 0x00, 0x47, 0xC5,
  0x00, 0x47, 0xC6, 0x00, 0x41, 0xC7, 0x00, 0x43,
  0xC8, 0x00, 0x4B, 0xC9, 0x00, 0x4B, 0xCA, 0x00,
  0x4B, 0xCB, 0x00, 0x4B, 0xCC, 0x00, 0x4F, 0xCD,
  0x00, 0x4F, 0xCE, 0x00, 0x4F, 0xCF, 0x00, 0x4F,
  0xD0, 0x00, 0x44, 0xD3, 0x00, 0x55, 0xD4, 0x00,
  0x55, 0xD5, 0x00, 0x55, 0xD6, 0x00, 0x4F, 0xD7,
  0x00, 0xAA, 0xD8, 0x00, 0x4F, 0xD9, 0x00, 0x5B,
  0xDA, 0x00, 0x5B, 0xDB, 0x00, 0x5B, 0xDC, 0x00,
  0x5B, 0xDD, 0x00, 0x59, 0xE0, 0x00, 0x67, 0xE1,
  0x00, 0x67, 0xE2, 0x00, 0x67, 0xE3, 0x00, 0x67,
  0xE4, 0x00, 0x67, 0xE5, 0x00, 0x67, 0xE6, 0x00,
  0x61, 0xE7, 0x00, 0x63, 0xE8, 0x00, 0x6B, 0xE9,
  0x00, 0x6B, 0xEA, 0x00, 0x6B, 0xEB, 0x00, 0x6B,
  0xEC, 0x00, 0x6F, 0xED, 0x00, 0x6F, 0xEE, 0x00,
  0x6F, 0xEF, 0x00, 0x6F, 0xF3, 0x00, 0x75, 0xF4,
  0x00, 0x75, 0xF5, 0x00, 0x75, 0xF6, 0x00, 0x6F,
  0xF7, 0x00, 0xBA, 0xF8, 0x00, 0x6F, 0xF9, 0x00,
  0x7B, 0xFA, 0x00, 0x7B, 0xFB, 0x00, 0x7B, 0xFC,
  0x00, 0x7B, 0xFD, 0x00, 0x79, 0xFF, 0x00, 0x79,
  0x00, 0x01, 0x41, 0x01, 0x01, 0x61, 0x02, 0x01,
  0x41, 0x03, 0x01, 0x61, 0x04, 0x01, 0x41, 0x05,
  0x01, 0x61, 0x06, 0x01, 0x43, 0x07, 0x01, 0x63,
  0x08, 0x01, 0x43, 0x09, 0x01, 0x63, 0x0A, 0x01,
  0x43, 0x0B, 0x01, 0x63, 0x0C, 0x01, 0x43, 0x0D,
  0x01, 0x63, 0x0E, 0x01, 0x44, 0x0F, 0x01, 0x64,
  0x10, 0x01, 0x44, 0x11, 0x01, 0x64, 0x12, 0x01,
  0x45, 0x13, 0x01, 0x65, 0x14, 0x01, 0x45, 0x15,
  0x01, 0x65, 0x16, 0x01, 0x45, 0x17, 0x01, 0x65,
  0x18, 0x01, 0x45, 0x19, 0x01, 0x65, 0x1A, 0x01,
  0x45, 0x1B, 0x01, 0x65, 0x1C, 0x01, 0x47, 0x1D,
  0x01, 0x67, 0x1E, 0x01, 0x47, 0x1F, 0x01, 0x67,
  0x20, 0x01, 0x47, 0x21, 0x01, 0x67, 0x22, 0x01,
  0x47, 0x23, 0x01, 0x67, 0x24, 0x01, 0x48, 0x25,
  0x01, 0x68, 0x26, 0x01, 0x48, 0x27, 0x01, 0x68,
  0x28, 0x01, 0x49, 0x29, 0x01, 0x69, 0x2A, 0x01,
  0x49, 0x2B, 0x01, 0x69, 0x2C, 0x01, 0x49, 0x2D,
  0x01, 0x69, 0x2E, 0x01, 0x49, 0x2F, 0x01, 0x69,
  0x30, 0x01, 0x49, 0x31, 0x01, 0x69, 0x34, 0x01,
  0x4A, 0x35, 0x01, 0x6A, 0x36, 0x01, 0x4B, 0x37,
  0x01, 0x6B, 0x39, 0x01, 0x4C, 0x3A, 0x01, 0x6C,
  0x3B, 0x01, 0x4C, 0x3C, 0x01, 0x6C, 0x3D, 0x01,
  0x4C, 0x3E, 0x01, 0x6C, 0x41, 0x01, 0x4C, 0x42,
  0x01, 0x6C, 0x43, 0x01, 0x4E, 0x44, 0x01, 0x6E,
  0x45, 0x01, 0x4E, 0x46, 0x01, 0x6E, 0x47, 0x01,
  0x4E, 0x48, 0x01, 0x6E, 0x4C, 0x01, 0x4F, 0x4D,
  0x01, 0x6F, 0x4E, 0x01, 0x4F, 0x4F, 0x01, 0x6F,
  0x50, 0x01, 0x4F, 0x51, 0x01, 0x6F, 0x52, 0x01,
  0x4F, 0x53, 0x01, 0x6F, 0x54, 0x01, 0x52, 0x55,
  0x01, 0x72, 0x56, 0x01, 0x52, 0x57, 0x01, 0x72,
  0x58, 0x01, 0x52, 0x59, 0x01, 0x72, 0x5A, 0x01,
  0x53, 0x5B, 0x01, 0x73, 0x5C, 0x01, 0x53, 0x5D,
  0x01, 0x73, 0x5E, 0x01, 0x53, 0x5F, 0x01, 0x73,
  0x60, 0x01, 0x53, 0x61, 0x01, 0x73, 0x62, 0x01,
  0x54, 0x63, 0x01, 0x74, 0x64, 0x01, 0x54, 0x65,
  0x01, 0x74, 0x66, 0x01, 0x54, 0x67, 0x01, 0x74,
  0x68, 0x01, 0x55, 0x69, 0x01, 0x75, 0x6A, 0x01,
  0x55, 0x6B, 0x01, 0x75, 0x6C, 0x01, 0x55, 0x6D,
  0x01, 0x75, 0x6E, 0x01, 0x55, 0x6F, 0x01, 0x75,
  0x70, 0x01, 0x55, 0x71, 0x01, 0x75, 0x72, 0x01,
  0x55, 0x73, 0x01, 0x75, 0x74, 0x01, 0x57, 0x75,
  0x01, 0x77, 0x76, 0x01, 0x59, 0x77, 0x01, 0x79,
  0x7A, 0x01, 0x7A, 0x7B, 0x01, 0x5A, 0x7C, 0x01,
  0x7A, 0x7D, 0x01, 0x5A, 0x7E, 0x01, 0x7A, 0x80,
  0x01, 0x62, 0x89, 0x01, 0x44, 0x91, 0x01, 0x46,
  0x92, 0x01, 0x66, 0x97, 0x01, 0x49, 0x9A, 0x01,
  0x6C, 0x9F, 0x01, 0x55, 0xA0, 0x01, 0x55, 0xA1,
  0x01, 0x6F, 0xAB, 0x01, 0x74, 0xB0, 0x01, 0x75,
  0xB6, 0x01, 0x7A, 0xCD, 0x01, 0x41, 0xCE, 0x01,
  0x61, 0xCF, 0x01, 0x49, 0xD0, 0x01, 0x69, 0xD1,
  0x01, 0x4F, 0xD2, 0x01, 0x6F, 0xD3, 0x01, 0x55,
  0xD4, 0x01, 0x75, 0xD5, 0x01, 0x55, 0xD6, 0x01,
  0x75, 0xD7, 0x01, 0x55, 0xD8, 0x01, 0x75, 0xD9,
  0x01, 0x55, 0xDA, 0x01, 0x75, 0xDB, 0x01, 0x55,
  0xDC, 0x01, 0x75, 0xDE, 0x01, 0x41, 0xDF, 0x01,
  0x61, 0xE4, 0x01, 0x47, 0xE5, 0x01, 0x67, 0xE6,
  0x01, 0x47, 0xE7, 0x01, 0x67, 0xE8, 0x01, 0x4B,
  0xE9, 0x01, 0x6B, 0xEA, 0x01, 0x4F, 0xEB, 0x01,
  0x6F, 0xEC, 0x01, 0x4F, 0xED, 0x01, 0x6F, 0xF0,
  0x01, 0x6A, 0x61, 0x02, 0x67, 0xB9, 0x02, 0x27,
  0xBA, 0x02, 0x22, 0xBC, 0x02, 0x27, 0xC4, 0x02,
  0x5E, 0xC6, 0x02, 0x5E, 0xC8, 0x02, 0x27, 0xCB,
  0x02, 0x60, 0xCD, 0x02, 0x5F, 0xDC, 0x02, 0x7E,
  0x00, 0x03, 0x60, 0x02, 0x03, 0x5E, 0x03, 0x03,
  0x7E, 0x0E, 0x03, 0x22, 0x31, 0x03, 0x65, 0x32,
  0x03, 0x65, 0x00, 0x20, 0x26, 0x01, 0x20, 0x26,
  0x02, 0x20, 0x26, 0x03, 0x20, 0x26, 0x04, 0x20,
  0x26, 0x05, 0x20, 0x26, 0x06, 0x20, 0x20, 0x10,
  0x20, 0x33, 0x11, 0x20, 0x33, 0x13, 0x20, 0x33,
  0x14, 0x20, 0x33, 0x17, 0x20, 0xDF, 0x18, 0x20,
  0x2D, 0x19, 0x20, 0x2D, 0x1A, 0x20, 0x2C, 0x1C,
  0x20, 0x28, 0x1D, 0x20, 0x28, 0x1E, 0x20, 0x22,
  0x22, 0x20, 0x2E, 0x26, 0x20, 0x2E, 0x32, 0x20,
  0x27, 0x35, 0x20, 0x60, 0x39, 0x20, 0x3C, 0x3A,
  0x20, 0x3E, 0x3E, 0x20, 0xAF, 0x22, 0x21, 0x54,
  0x9C, 0xF7, 0xA1, 0xFF, 0xFF, 0xFF
};
#endif
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
