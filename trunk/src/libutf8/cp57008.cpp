//---------------------------------------------------------------------------
#include <adicpp/lconfig.h>
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
#if defined(EMBED_CP57008_SUPPORT) || defined(EMBED_ALL_CP)
extern const unsigned char cp57008s2utf8s[123] = {
  0xB0, 0xDE, 0x00, 0x00, 0x0E, 0x00, 0x9F, 0x00,
  0x00, 0xA2, 0xA3, 0x82, 0x0C, 0xA4, 0xAA, 0x85,
  0x0C, 0xAB, 0xAD, 0x8E, 0x0C, 0xAF, 0xB1, 0x92,
  0x0C, 0xB2, 0xC6, 0x94, 0x0C, 0xC8, 0xCD, 0xAA,
  0x0C, 0xCE, 0xD2, 0xAF, 0x0C, 0xD4, 0xD8, 0xB5,
  0x0C, 0xDA, 0xDF, 0xBE, 0x0C, 0xE0, 0xE2, 0xC6,
  0x0C, 0xE4, 0xE6, 0xCA, 0x0C, 0xE7, 0xE8, 0xCC,
  0x0C, 0xF1, 0xFA, 0xE6, 0x0C, 0x14, 0xA0, 0x3F,
  0x00, 0xA1, 0x3F, 0x00, 0xAE, 0x90, 0x0C, 0xC7,
  0xA8, 0x0C, 0xD3, 0xB3, 0x0C, 0xD9, 0x3F, 0x00,
  0xE3, 0xC8, 0x0C, 0xE9, 0x3F, 0x00, 0xEA, 0x2E,
  0x00, 0xEB, 0x3F, 0x00, 0xEC, 0x3F, 0x00, 0xED,
  0x3F, 0x00, 0xEE, 0x3F, 0x00, 0xEF, 0x3F, 0x00,
  0xF0, 0x3F, 0x00, 0xFB, 0x3F, 0x00, 0xFC, 0x3F,
  0x00, 0xFD, 0x3F, 0x00, 0xFE, 0x3F, 0x00, 0xFF,
  0x3F, 0x00, 0x00
};
//---------------------------------------------------------------------------
extern const unsigned char utf8s2cp57008s[3243] = {
  0x04, 0x3F, 0x0D, 0x01, 0x00, 0x00, 0x9F, 0x00,
  0x00, 0x82, 0x0C, 0x83, 0x0C, 0xA2, 0x85, 0x0C,
  0x8B, 0x0C, 0xA4, 0x8E, 0x0C, 0x90, 0x0C, 0xAB,
  0x92, 0x0C, 0x94, 0x0C, 0xAF, 0x95, 0x0C, 0xA8,
  0x0C, 0xB3, 0xAA, 0x0C, 0xAF, 0x0C, 0xC8, 0xB0,
  0x0C, 0xB3, 0x0C, 0xCF, 0xB5, 0x0C, 0xB9, 0x0C,
  0xD4, 0xBE, 0x0C, 0xC3, 0x0C, 0xDA, 0xC6, 0x0C,
  0xC8, 0x0C, 0xE0, 0xCA, 0x0C, 0xCC, 0x0C, 0xE4,
  0xE6, 0x0C, 0xEF, 0x0C, 0xF1, 0x01, 0x02, 0x00,
  0x00, 0x9F, 0x00, 0x00, 0x00, 0x01, 0x03, 0x00,
  0x00, 0x9F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04,
  0x00, 0x00, 0x9F, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0x01, 0x00, 0x01, 0xCD, 0x0C, 0xE8,
  0x05, 0x00, 0x02, 0x8C, 0x0C, 0xA6, 0xE9, 0xC4,
  0x0C, 0xE5, 0xE9, 0xDE, 0x0C, 0xC9, 0xE9, 0xE0,
  0x0C, 0xAA, 0xE9, 0xE1, 0x0C, 0xA7, 0xE9, 0x35,
  0x02, 0x03, 0x01, 0x09, 0xF5, 0x42, 0xA1, 0x02,
  0x09, 0xF5, 0x42, 0xA2, 0x03, 0x09, 0xEF, 0x42,
  0xA3, 0x05, 0x09, 0xF5, 0x42, 0xA4, 0x06, 0x09,
  0xF5, 0x42, 0xA5, 0x07, 0x09, 0xF5, 0x42, 0xA6,
  0x08, 0x09, 0xF5, 0x42, 0xA7, 0x09, 0x09, 0xF5,
  0x42, 0xA8, 0x0A, 0x09, 0xF5, 0x42, 0xA9, 0x0B,
  0x09, 0xF5, 0x42, 0xAA, 0x0D, 0x09, 0xF5, 0x42,
  0xAE, 0x0E, 0x09, 0xF5, 0x42, 0xAB, 0x0F, 0x09,
  0xF5, 0x42, 0xAC, 0x10, 0x09, 0xF5, 0x42, 0xAD,
  0x11, 0x09, 0xF5, 0x42, 0xB2, 0x12, 0x09, 0xF5,
  0x42, 0xAF, 0x13, 0x09, 0xF5, 0x42, 0xB0, 0x14,
  0x09, 0xF5, 0x42, 0xB1, 0x15, 0x09, 0xF5, 0x42,
  0xB3, 0x16, 0x09, 0xF5, 0x42, 0xB4, 0x17, 0x09,
  0xF5, 0x42, 0xB5, 0x18, 0x09, 0xF5, 0x42, 0xB6,
  0x19, 0x09, 0xF5, 0x42, 0xB7, 0x1A, 0x09, 0xF5,
  0x42, 0xB8, 0x1B, 0x09, 0xF5, 0x42, 0xB9, 0x1C,
  0x09, 0xF5, 0x42, 0xBA, 0x1D, 0x09, 0xF5, 0x42,
  0xBB, 0x1E, 0x09, 0xF5, 0x42, 0xBC, 0x1F, 0x09,
  0xF5, 0x42, 0xBD, 0x20, 0x09, 0xF5, 0x42, 0xBE,
  0x21, 0x09, 0xF5, 0x42, 0xBF, 0x22, 0x09, 0xF5,
  0x42, 0xC0, 0x23, 0x09, 0xF5, 0x42, 0xC1, 0x24,
  0x09, 0xF5, 0x42, 0xC2, 0x25, 0x09, 0xF5, 0x42,
  0xC3, 0x26, 0x09, 0xF5, 0x42, 0xC4, 0x27, 0x09,
  0xF5, 0x42, 0xC5, 0x28, 0x09, 0xF5, 0x42, 0xC6,
  0x29, 0x09, 0xF5, 0x42, 0xC7, 0x2A, 0x09, 0xF5,
  0x42, 0xC8, 0x2B, 0x09, 0xF5, 0x42, 0xC9, 0x2C,
  0x09, 0xF5, 0x42, 0xCA, 0x2D, 0x09, 0xF5, 0x42,
  0xCB, 0x2E, 0x09, 0xF5, 0x42, 0xCC, 0x2F, 0x09,
  0xF5, 0x42, 0xCD, 0x30, 0x09, 0xF5, 0x42, 0xCF,
  0x31, 0x09, 0xF5, 0x42, 0xD0, 0x32, 0x09, 0xF5,
  0x42, 0xD1, 0x33, 0x09, 0xF5, 0x42, 0xD2, 0x34,
  0x09, 0xF5, 0x42, 0xD3, 0x35, 0x09, 0xF5, 0x42,
  0xD4, 0x36, 0x09, 0xF5, 0x42, 0xD5, 0x37, 0x09,
  0xF5, 0x42, 0xD6, 0x38, 0x09, 0xF5, 0x42, 0xD7,
  0x39, 0x09, 0xEF, 0x42, 0xD8, 0x3C, 0x09, 0xF5,
  0x42, 0xE9, 0x3E, 0x09, 0xF5, 0x42, 0xDA, 0x3F,
  0x09, 0xF5, 0x42, 0xDB, 0x40, 0x09, 0xF5, 0x42,
  0xDC, 0x41, 0x09, 0xF5, 0x42, 0xDD, 0x42, 0x09,
  0xF5, 0x42, 0xDE, 0x43, 0x09, 0xF5, 0x42, 0xDF,
  0x45, 0x09, 0xF5, 0x42, 0xE3, 0x46, 0x09, 0xF5,
  0x42, 0xE0, 0x47, 0x09, 0xF5, 0x42, 0xE1, 0x48,
  0x09, 0xF5, 0x42, 0xE2, 0x49, 0x09, 0xF5, 0x42,
  0xE7, 0x4A, 0x09, 0xF5, 0x42, 0xE4, 0x4B, 0x09,
  0xF5, 0x42, 0xE5, 0x4C, 0x09, 0xF5, 0x42, 0xE6,
  0x4D, 0x09, 0xF5, 0x42, 0xE8, 0x5F, 0x09, 0xF5,
  0x42, 0xCE, 0x64, 0x09, 0xEF, 0x42, 0xEA, 0x66,
  0x09, 0xF5, 0x42, 0xF1, 0x67, 0x09, 0xF5, 0x42,
  0xF2, 0x68, 0x09, 0xF5, 0x42, 0xF3, 0x69, 0x09,
  0xF5, 0x42, 0xF4, 0x6A, 0x09, 0xF5, 0x42, 0xF5,
  0x6B, 0x09, 0xF5, 0x42, 0xF6, 0x6C, 0x09, 0xF5,
  0x42, 0xF7, 0x6D, 0x09, 0xF5, 0x42, 0xF8, 0x6E,
  0x09, 0xF5, 0x42, 0xF9, 0x6F, 0x09, 0xF5, 0x42,
  0xFA, 0x81, 0x09, 0xF5, 0x43, 0xA1, 0x82, 0x09,
  0xF5, 0x43, 0xA2, 0x83, 0x09, 0xEF, 0x43, 0xA3,
  0x85, 0x09, 0xF5, 0x43, 0xA4, 0x86, 0x09, 0xF5,
  0x43, 0xA5, 0x87, 0x09, 0xF5, 0x43, 0xA6, 0x88,
  0x09, 0xF5, 0x43, 0xA7, 0x89, 0x09, 0xF5, 0x43,
  0xA8, 0x8A, 0x09, 0xF5, 0x43, 0xA9, 0x8B, 0x09,
  0xF5, 0x43, 0xAA, 0x8F, 0x09, 0xF5, 0x43, 0xAB,
  0x90, 0x09, 0xF5, 0x43, 0xAD, 0x93, 0x09, 0xF5,
  0x43, 0xAF, 0x94, 0x09, 0xF5, 0x43, 0xB1, 0x95,
  0x09, 0xF5, 0x43, 0xB3, 0x96, 0x09, 0xF5, 0x43,
  0xB4, 0x97, 0x09, 0xF5, 0x43, 0xB5, 0x98, 0x09,
  0xF5, 0x43, 0xB6, 0x99, 0x09, 0xF5, 0x43, 0xB7,
  0x9A, 0x09, 0xF5, 0x43, 0xB8, 0x9B, 0x09, 0xF5,
  0x43, 0xB9, 0x9C, 0x09, 0xF5, 0x43, 0xBA, 0x9D,
  0x09, 0xF5, 0x43, 0xBB, 0x9E, 0x09, 0xF5, 0x43,
  0xBC, 0x9F, 0x09, 0xF5, 0x43, 0xBD, 0xA0, 0x09,
  0xF5, 0x43, 0xBE, 0xA1, 0x09, 0xF5, 0x43, 0xBF,
  0xA2, 0x09, 0xF5, 0x43, 0xC0, 0xA3, 0x09, 0xF5,
  0x43, 0xC1, 0xA4, 0x09, 0xF5, 0x43, 0xC2, 0xA5,
  0x09, 0xF5, 0x43, 0xC3, 0xA6, 0x09, 0xF5, 0x43,
  0xC4, 0xA7, 0x09, 0xF5, 0x43, 0xC5, 0xA8, 0x09,
  0xF5, 0x43, 0xC6, 0xAA, 0x09, 0xF5, 0x43, 0xC8,
  0xAB, 0x09, 0xF5, 0x43, 0xC9, 0xAC, 0x09, 0xF5,
  0x43, 0xCA, 0xAD, 0x09, 0xF5, 0x43, 0xCB, 0xAE,
  0x09, 0xF5, 0x43, 0xCC, 0xAF, 0x09, 0xF5, 0x43,
  0xCD, 0xB0, 0x09, 0xEF, 0x43, 0xCF, 0xB2, 0x09,
  0xEF, 0x43, 0xD1, 0xB6, 0x09, 0xF5, 0x43, 0xD5,
  0xB7, 0x09, 0xF5, 0x43, 0xD6, 0xB8, 0x09, 0xF5,
  0x43, 0xD7, 0xB9, 0x09, 0xF5, 0x43, 0xD8, 0xBC,
  0x09, 0xEF, 0x43, 0xE9, 0xBE, 0x09, 0xF5, 0x43,
  0xDA, 0xBF, 0x09, 0xF5, 0x43, 0xDB, 0xC0, 0x09,
  0xF5, 0x43, 0xDC, 0xC1, 0x09, 0xF5, 0x43, 0xDD,
  0xC2, 0x09, 0xF5, 0x43, 0xDE, 0xC3, 0x09, 0xF5,
  0x43, 0xDF, 0xC7, 0x09, 0xF5, 0x43, 0xE0, 0xC8,
  0x09, 0xF5, 0x43, 0xE2, 0xCB, 0x09, 0xF5, 0x43,
  0xE4, 0xCC, 0x09, 0xF5, 0x43, 0xE6, 0xCD, 0x09,
  0xEF, 0x43, 0xE8, 0xDF, 0x09, 0xF5, 0x43, 0xCE,
  0xE6, 0x09, 0xF5, 0x43, 0xF1, 0xE7, 0x09, 0xF5,
  0x43, 0xF2, 0xE8, 0x09, 0xF5, 0x43, 0xF3, 0xE9,
  0x09, 0xF5, 0x43, 0xF4, 0xEA, 0x09, 0xF5, 0x43,
  0xF5, 0xEB, 0x09, 0xF5, 0x43, 0xF6, 0xEC, 0x09,
  0xF5, 0x43, 0xF7, 0xED, 0x09, 0xF5, 0x43, 0xF8,
  0xEE, 0x09, 0xF5, 0x43, 0xF9, 0xEF, 0x09, 0xF5,
  0x43, 0xFA, 0x02, 0x0A, 0xEF, 0x4B, 0xA2, 0x05,
  0x0A, 0xF5, 0x4B, 0xA4, 0x06, 0x0A, 0xF5, 0x4B,
  0xA5, 0x07, 0x0A, 0xF5, 0x4B, 0xA6, 0x08, 0x0A,
  0xF5, 0x4B, 0xA7, 0x09, 0x0A, 0xF5, 0x4B, 0xA8,
  0x0A, 0x0A, 0xF5, 0x4B, 0xA9, 0x0F, 0x0A, 0xF5,
  0x4B, 0xAB, 0x10, 0x0A, 0xF5, 0x4B, 0xAD, 0x13,
  0x0A, 0xF5, 0x4B, 0xB0, 0x14, 0x0A, 0xF5, 0x4B,
  0xB1, 0x15, 0x0A, 0xF5, 0x4B, 0xB3, 0x16, 0x0A,
  0xF5, 0x4B, 0xB4, 0x17, 0x0A, 0xF5, 0x4B, 0xB5,
  0x18, 0x0A, 0xF5, 0x4B, 0xB6, 0x19, 0x0A, 0xF5,
  0x4B, 0xB7, 0x1A, 0x0A, 0xF5, 0x4B, 0xB8, 0x1B,
  0x0A, 0xF5, 0x4B, 0xB9, 0x1C, 0x0A, 0xF5, 0x4B,
  0xBA, 0x1D, 0x0A, 0xF5, 0x4B, 0xBB, 0x1E, 0x0A,
  0xF5, 0x4B, 0xBC, 0x1F, 0x0A, 0xF5, 0x4B, 0xBD,
  0x20, 0x0A, 0xF5, 0x4B, 0xBE, 0x21, 0x0A, 0xF5,
  0x4B, 0xBF, 0x22, 0x0A, 0xF5, 0x4B, 0xC0, 0x23,
  0x0A, 0xF5, 0x4B, 0xC1, 0x24, 0x0A, 0xF5, 0x4B,
  0xC2, 0x25, 0x0A, 0xF5, 0x4B, 0xC3, 0x26, 0x0A,
  0xF5, 0x4B, 0xC4, 0x27, 0x0A, 0xF5, 0x4B, 0xC5,
  0x28, 0x0A, 0xF5, 0x4B, 0xC6, 0x2A, 0x0A, 0xF5,
  0x4B, 0xC8, 0x2B, 0x0A, 0xF5, 0x4B, 0xC9, 0x2C,
  0x0A, 0xF5, 0x4B, 0xCA, 0x2D, 0x0A, 0xF5, 0x4B,
  0xCB, 0x2E, 0x0A, 0xF5, 0x4B, 0xCC, 0x2F, 0x0A,
  0xF5, 0x4B, 0xCD, 0x30, 0x0A, 0xEF, 0x4B, 0xCF,
  0x32, 0x0A, 0xF5, 0x4B, 0xD1, 0x33, 0x0A, 0xF5,
  0x4B, 0xD2, 0x35, 0x0A, 0xF5, 0x4B, 0xD4, 0x36,
  0x0A, 0xF5, 0x4B, 0xD5, 0x38, 0x0A, 0xF5, 0x4B,
  0xD7, 0x39, 0x0A, 0xF5, 0x4B, 0xD8, 0x3C, 0x0A,
  0xEF, 0x4B, 0xE9, 0x3E, 0x0A, 0xF5, 0x4B, 0xDA,
  0x3F, 0x0A, 0xF5, 0x4B, 0xDB, 0x40, 0x0A, 0xF5,
  0x4B, 0xDC, 0x41, 0x0A, 0xF5, 0x4B, 0xDD, 0x42,
  0x0A, 0xEF, 0x4B, 0xDE, 0x47, 0x0A, 0xF5, 0x4B,
  0xE0, 0x48, 0x0A, 0xF5, 0x4B, 0xE2, 0x4B, 0x0A,
  0xF5, 0x4B, 0xE4, 0x4C, 0x0A, 0xF5, 0x4B, 0xE6,
  0x4D, 0x0A, 0xEF, 0x4B, 0xE8, 0x66, 0x0A, 0xF5,
  0x4B, 0xF1, 0x67, 0x0A, 0xF5, 0x4B, 0xF2, 0x68,
  0x0A, 0xF5, 0x4B, 0xF3, 0x69, 0x0A, 0xF5, 0x4B,
  0xF4, 0x6A, 0x0A, 0xF5, 0x4B, 0xF5, 0x6B, 0x0A,
  0xF5, 0x4B, 0xF6, 0x6C, 0x0A, 0xF5, 0x4B, 0xF7,
  0x6D, 0x0A, 0xF5, 0x4B, 0xF8, 0x6E, 0x0A, 0xF5,
  0x4B, 0xF9, 0x6F, 0x0A, 0xF5, 0x4B, 0xFA, 0x81,
  0x0A, 0xF5, 0x4A, 0xA1, 0x82, 0x0A, 0xF5, 0x4A,
  0xA2, 0x83, 0x0A, 0xEF, 0x4A, 0xA3, 0x85, 0x0A,
  0xF5, 0x4A, 0xA4, 0x86, 0x0A, 0xF5, 0x4A, 0xA5,
  0x87, 0x0A, 0xF5, 0x4A, 0xA6, 0x88, 0x0A, 0xF5,
  0x4A, 0xA7, 0x89, 0x0A, 0xF5, 0x4A, 0xA8, 0x8A,
  0x0A, 0xF5, 0x4A, 0xA9, 0x8B, 0x0A, 0xEF, 0x4A,
  0xAA, 0x8D, 0x0A, 0xEF, 0x4A, 0xAE, 0x8F, 0x0A,
  0xF5, 0x4A, 0xAB, 0x90, 0x0A, 0xF5, 0x4A, 0xAD,
  0x91, 0x0A, 0xEF, 0x4A, 0xB2, 0x93, 0x0A, 0xF5,
  0x4A, 0xB0, 0x94, 0x0A, 0xF5, 0x4A, 0xB1, 0x95,
  0x0A, 0xF5, 0x4A, 0xB3, 0x96, 0x0A, 0xF5, 0x4A,
  0xB4, 0x97, 0x0A, 0xF5, 0x4A, 0xB5, 0x98, 0x0A,
  0xF5, 0x4A, 0xB6, 0x99, 0x0A, 0xF5, 0x4A, 0xB7,
  0x9A, 0x0A, 0xF5, 0x4A, 0xB8, 0x9B, 0x0A, 0xF5,
  0x4A, 0xB9, 0x9C, 0x0A, 0xF5, 0x4A, 0xBA, 0x9D,
  0x0A, 0xF5, 0x4A, 0xBB, 0x9E, 0x0A, 0xF5, 0x4A,
  0xBC, 0x9F, 0x0A, 0xF5, 0x4A, 0xBD, 0xA0, 0x0A,
  0xF5, 0x4A, 0xBE, 0xA1, 0x0A, 0xF5, 0x4A, 0xBF,
  0xA2, 0x0A, 0xF5, 0x4A, 0xC0, 0xA3, 0x0A, 0xF5,
  0x4A, 0xC1, 0xA4, 0x0A, 0xF5, 0x4A, 0xC2, 0xA5,
  0x0A, 0xF5, 0x4A, 0xC3, 0xA6, 0x0A, 0xF5, 0x4A,
  0xC4, 0xA7, 0x0A, 0xF5, 0x4A, 0xC5, 0xA8, 0x0A,
  0xF5, 0x4A, 0xC6, 0xAA, 0x0A, 0xF5, 0x4A, 0xC8,
  0xAB, 0x0A, 0xF5, 0x4A, 0xC9, 0xAC, 0x0A, 0xF5,
  0x4A, 0xCA, 0xAD, 0x0A, 0xF5, 0x4A, 0xCB, 0xAE,
  0x0A, 0xF5, 0x4A, 0xCC, 0xAF, 0x0A, 0xF5, 0x4A,
  0xCD, 0xB0, 0x0A, 0xEF, 0x4A, 0xCF, 0xB2, 0x0A,
  0xF5, 0x4A, 0xD1, 0xB3, 0x0A, 0xF5, 0x4A, 0xD2,
  0xB5, 0x0A, 0xF5, 0x4A, 0xD4, 0xB6, 0x0A, 0xF5,
  0x4A, 0xD5, 0xB7, 0x0A, 0xF5, 0x4A, 0xD6, 0xB8,
  0x0A, 0xF5, 0x4A, 0xD7, 0xB9, 0x0A, 0xEF, 0x4A,
  0xD8, 0xBC, 0x0A, 0xF5, 0x4A, 0xE9, 0xBE, 0x0A,
  0xF5, 0x4A, 0xDA, 0xBF, 0x0A, 0xF5, 0x4A, 0xDB,
  0xC0, 0x0A, 0xF5, 0x4A, 0xDC, 0xC1, 0x0A, 0xF5,
  0x4A, 0xDD, 0xC2, 0x0A, 0xF5, 0x4A, 0xDE, 0xC3,
  0x0A, 0xF5, 0x4A, 0xDF, 0xC5, 0x0A, 0xF5, 0x4A,
  0xE3, 0xC7, 0x0A, 0xF5, 0x4A, 0xE0, 0xC8, 0x0A,
  0xF5, 0x4A, 0xE2, 0xC9, 0x0A, 0xEF, 0x4A, 0xE7,
  0xCB, 0x0A, 0xF5, 0x4A, 0xE4, 0xCC, 0x0A, 0xF5,
  0x4A, 0xE6, 0xCD, 0x0A, 0xEF, 0x4A, 0xE8, 0xE6,
  0x0A, 0xF5, 0x4A, 0xF1, 0xE7, 0x0A, 0xF5, 0x4A,
  0xF2, 0xE8, 0x0A, 0xF5, 0x4A, 0xF3, 0xE9, 0x0A,
  0xF5, 0x4A, 0xF4, 0xEA, 0x0A, 0xF5, 0x4A, 0xF5,
  0xEB, 0x0A, 0xF5, 0x4A, 0xF6, 0xEC, 0x0A, 0xF5,
  0x4A, 0xF7, 0xED, 0x0A, 0xF5, 0x4A, 0xF8, 0xEE,
  0x0A, 0xF5, 0x4A, 0xF9, 0xEF, 0x0A, 0xF5, 0x4A,
  0xFA, 0x01, 0x0B, 0xF5, 0x47, 0xA1, 0x02, 0x0B,
  0xF5, 0x47, 0xA2, 0x03, 0x0B, 0xEF, 0x47, 0xA3,
  0x05, 0x0B, 0xF5, 0x47, 0xA4, 0x06, 0x0B, 0xF5,
  0x47, 0xA5, 0x07, 0x0B, 0xF5, 0x47, 0xA6, 0x08,
  0x0B, 0xF5, 0x47, 0xA7, 0x09, 0x0B, 0xF5, 0x47,
  0xA8, 0x0A, 0x0B, 0xF5, 0x47, 0xA9, 0x0B, 0x0B,
  0xF5, 0x47, 0xAA, 0x0F, 0x0B, 0xF5, 0x47, 0xAB,
  0x10, 0x0B, 0xF5, 0x47, 0xAD, 0x13, 0x0B, 0xF5,
  0x47, 0xB0, 0x14, 0x0B, 0xF5, 0x47, 0xB1, 0x15,
  0x0B, 0xF5, 0x47, 0xB3, 0x16, 0x0B, 0xF5, 0x47,
  0xB4, 0x17, 0x0B, 0xF5, 0x47, 0xB5, 0x18, 0x0B,
  0xF5, 0x47, 0xB6, 0x19, 0x0B, 0xF5, 0x47, 0xB7,
  0x1A, 0x0B, 0xF5, 0x47, 0xB8, 0x1B, 0x0B, 0xF5,
  0x47, 0xB9, 0x1C, 0x0B, 0xF5, 0x47, 0xBA, 0x1D,
  0x0B, 0xF5, 0x47, 0xBB, 0x1E, 0x0B, 0xF5, 0x47,
  0xBC, 0x1F, 0x0B, 0xF5, 0x47, 0xBD, 0x20, 0x0B,
  0xF5, 0x47, 0xBE, 0x21, 0x0B, 0xF5, 0x47, 0xBF,
  0x22, 0x0B, 0xF5, 0x47, 0xC0, 0x23, 0x0B, 0xF5,
  0x47, 0xC1, 0x24, 0x0B, 0xF5, 0x47, 0xC2, 0x25,
  0x0B, 0xF5, 0x47, 0xC3, 0x26, 0x0B, 0xF5, 0x47,
  0xC4, 0x27, 0x0B, 0xF5, 0x47, 0xC5, 0x28, 0x0B,
  0xF5, 0x47, 0xC6, 0x2A, 0x0B, 0xF5, 0x47, 0xC8,
  0x2B, 0x0B, 0xF5, 0x47, 0xC9, 0x2C, 0x0B, 0xF5,
  0x47, 0xCA, 0x2D, 0x0B, 0xF5, 0x47, 0xCB, 0x2E,
  0x0B, 0xF5, 0x47, 0xCC, 0x2F, 0x0B, 0xF5, 0x47,
  0xCD, 0x30, 0x0B, 0xEF, 0x47, 0xCF, 0x32, 0x0B,
  0xF5, 0x47, 0xD1, 0x33, 0x0B, 0xF5, 0x47, 0xD2,
  0x36, 0x0B, 0xF5, 0x47, 0xD5, 0x37, 0x0B, 0xF5,
  0x47, 0xD6, 0x38, 0x0B, 0xF5, 0x47, 0xD7, 0x39,
  0x0B, 0xF5, 0x47, 0xD8, 0x3C, 0x0B, 0xF5, 0x47,
  0xE9, 0x3E, 0x0B, 0xF5, 0x47, 0xDA, 0x3F, 0x0B,
  0xF5, 0x47, 0xDB, 0x40, 0x0B, 0xF5, 0x47, 0xDC,
  0x41, 0x0B, 0xF5, 0x47, 0xDD, 0x42, 0x0B, 0xF5,
  0x47, 0xDE, 0x43, 0x0B, 0xF5, 0x47, 0xDF, 0x47,
  0x0B, 0xF5, 0x47, 0xE0, 0x48, 0x0B, 0xF5, 0x47,
  0xE2, 0x4B, 0x0B, 0xF5, 0x47, 0xE4, 0x4C, 0x0B,
  0xF5, 0x47, 0xE6, 0x4D, 0x0B, 0xEF, 0x47, 0xE8,
  0x5F, 0x0B, 0xF5, 0x47, 0xCE, 0x66, 0x0B, 0xF5,
  0x47, 0xF1, 0x67, 0x0B, 0xF5, 0x47, 0xF2, 0x68,
  0x0B, 0xF5, 0x47, 0xF3, 0x69, 0x0B, 0xF5, 0x47,
  0xF4, 0x6A, 0x0B, 0xF5, 0x47, 0xF5, 0x6B, 0x0B,
  0xF5, 0x47, 0xF6, 0x6C, 0x0B, 0xF5, 0x47, 0xF7,
  0x6D, 0x0B, 0xF5, 0x47, 0xF8, 0x6E, 0x0B, 0xF5,
  0x47, 0xF9, 0x6F, 0x0B, 0xF5, 0x47, 0xFA, 0x82,
  0x0B, 0xF5, 0x44, 0xA2, 0x83, 0x0B, 0xF5, 0x44,
  0xA3, 0x85, 0x0B, 0xF5, 0x44, 0xA4, 0x86, 0x0B,
  0xF5, 0x44, 0xA5, 0x87, 0x0B, 0xF5, 0x44, 0xA6,
  0x88, 0x0B, 0xF5, 0x44, 0xA7, 0x89, 0x0B, 0xF5,
  0x44, 0xA8, 0x8A, 0x0B, 0xF5, 0x44, 0xA9, 0x8F,
  0x0B, 0xF5, 0x44, 0xAB, 0x90, 0x0B, 0xF5, 0x44,
  0xAD, 0x92, 0x0B, 0xF5, 0x44, 0xAF, 0x93, 0x0B,
  0xF5, 0x44, 0xB0, 0x94, 0x0B, 0xF5, 0x44, 0xB1,
  0x95, 0x0B, 0xF5, 0x44, 0xB3, 0x99, 0x0B, 0xF5,
  0x44, 0xB7, 0x9A, 0x0B, 0xF5, 0x44, 0xB8, 0x9C,
  0x0B, 0xEF, 0x44, 0xBA, 0x9E, 0x0B, 0xF5, 0x44,
  0xBC, 0x9F, 0x0B, 0xF5, 0x44, 0xBD, 0xA3, 0x0B,
  0xF5, 0x44, 0xC1, 0xA4, 0x0B, 0xF5, 0x44, 0xC2,
  0xA8, 0x0B, 0xF5, 0x44, 0xC6, 0xA9, 0x0B, 0xF5,
  0x44, 0xC7, 0xAA, 0x0B, 0xEF, 0x44, 0xC8, 0xAE,
  0x0B, 0xF5, 0x44, 0xCC, 0xAF, 0x0B, 0xF5, 0x44,
  0xCD, 0xB0, 0x0B, 0xF5, 0x44, 0xCF, 0xB1, 0x0B,
  0xF5, 0x44, 0xD0, 0xB2, 0x0B, 0xF5, 0x44, 0xD1,
  0xB3, 0x0B, 0xF5, 0x44, 0xD2, 0xB4, 0x0B, 0xF5,
  0x44, 0xD3, 0xB5, 0x0B, 0xF5, 0x44, 0xD4, 0xB7,
  0x0B, 0xF5, 0x44, 0xD5, 0xB8, 0x0B, 0xF5, 0x44,
  0xD7, 0xB9, 0x0B, 0xEF, 0x44, 0xD8, 0xBE, 0x0B,
  0xF5, 0x44, 0xDA, 0xBF, 0x0B, 0xF5, 0x44, 0xDB,
  0xC0, 0x0B, 0xF5, 0x44, 0xDC, 0xC1, 0x0B, 0xF5,
  0x44, 0xDD, 0xC2, 0x0B, 0xEF, 0x44, 0xDE, 0xC6,
  0x0B, 0xF5, 0x44, 0xE0, 0xC7, 0x0B, 0xF5, 0x44,
  0xE1, 0xC8, 0x0B, 0xEF, 0x44, 0xE2, 0xCA, 0x0B,
  0xF5, 0x44, 0xE4, 0xCB, 0x0B, 0xF5, 0x44, 0xE5,
  0xCC, 0x0B, 0xF5, 0x44, 0xE6, 0xCD, 0x0B, 0xF5,
  0x44, 0xE8, 0xE7, 0x0B, 0xF5, 0x44, 0xF2, 0xE8,
  0x0B, 0xF5, 0x44, 0xF3, 0xE9, 0x0B, 0xF5, 0x44,
  0xF4, 0xEA, 0x0B, 0xF5, 0x44, 0xF5, 0xEB, 0x0B,
  0xF5, 0x44, 0xF6, 0xEC, 0x0B, 0xF5, 0x44, 0xF7,
  0xED, 0x0B, 0xF5, 0x44, 0xF8, 0xEE, 0x0B, 0xF5,
  0x44, 0xF9, 0xEF, 0x0B, 0xEF, 0x44, 0xFA, 0x01,
  0x0C, 0xF5, 0x45, 0xA1, 0x02, 0x0C, 0xF5, 0x45,
  0xA2, 0x03, 0x0C, 0xEF, 0x45, 0xA3, 0x05, 0x0C,
  0xF5, 0x45, 0xA4, 0x06, 0x0C, 0xF5, 0x45, 0xA5,
  0x07, 0x0C, 0xF5, 0x45, 0xA6, 0x08, 0x0C, 0xF5,
  0x45, 0xA7, 0x09, 0x0C, 0xF5, 0x45, 0xA8, 0x0A,
  0x0C, 0xF5, 0x45, 0xA9, 0x0B, 0x0C, 0xF5, 0x45,
  0xAA, 0x0E, 0x0C, 0xF5, 0x45, 0xAB, 0x0F, 0x0C,
  0xF5, 0x45, 0xAC, 0x10, 0x0C, 0xEF, 0x45, 0xAD,
  0x12, 0x0C, 0xF5, 0x45, 0xAF, 0x13, 0x0C, 0xF5,
  0x45, 0xB0, 0x14, 0x0C, 0xF5, 0x45, 0xB1, 0x15,
  0x0C, 0xF5, 0x45, 0xB3, 0x16, 0x0C, 0xF5, 0x45,
  0xB4, 0x17, 0x0C, 0xF5, 0x45, 0xB5, 0x18, 0x0C,
  0xF5, 0x45, 0xB6, 0x19, 0x0C, 0xF5, 0x45, 0xB7,
  0x1A, 0x0C, 0xF5, 0x45, 0xB8, 0x1B, 0x0C, 0xF5,
  0x45, 0xB9, 0x1C, 0x0C, 0xF5, 0x45, 0xBA, 0x1D,
  0x0C, 0xF5, 0x45, 0xBB, 0x1E, 0x0C, 0xF5, 0x45,
  0xBC, 0x1F, 0x0C, 0xF5, 0x45, 0xBD, 0x20, 0x0C,
  0xF5, 0x45, 0xBE, 0x21, 0x0C, 0xF5, 0x45, 0xBF,
  0x22, 0x0C, 0xF5, 0x45, 0xC0, 0x23, 0x0C, 0xF5,
  0x45, 0xC1, 0x24, 0x0C, 0xF5, 0x45, 0xC2, 0x25,
  0x0C, 0xF5, 0x45, 0xC3, 0x26, 0x0C, 0xF5, 0x45,
  0xC4, 0x27, 0x0C, 0xF5, 0x45, 0xC5, 0x28, 0x0C,
  0xEF, 0x45, 0xC6, 0x2A, 0x0C, 0xF5, 0x45, 0xC8,
  0x2B, 0x0C, 0xF5, 0x45, 0xC9, 0x2C, 0x0C, 0xF5,
  0x45, 0xCA, 0x2D, 0x0C, 0xF5, 0x45, 0xCB, 0x2E,
  0x0C, 0xF5, 0x45, 0xCC, 0x2F, 0x0C, 0xF5, 0x45,
  0xCD, 0x30, 0x0C, 0xF5, 0x45, 0xCF, 0x31, 0x0C,
  0xF5, 0x45, 0xD0, 0x32, 0x0C, 0xF5, 0x45, 0xD1,
  0x33, 0x0C, 0xF5, 0x45, 0xD2, 0x35, 0x0C, 0xF5,
  0x45, 0xD4, 0x36, 0x0C, 0xF5, 0x45, 0xD5, 0x37,
  0x0C, 0xF5, 0x45, 0xD6, 0x38, 0x0C, 0xF5, 0x45,
  0xD7, 0x39, 0x0C, 0xEF, 0x45, 0xD8, 0x3E, 0x0C,
  0xF5, 0x45, 0xDA, 0x3F, 0x0C, 0xF5, 0x45, 0xDB,
  0x40, 0x0C, 0xF5, 0x45, 0xDC, 0x41, 0x0C, 0xF5,
  0x45, 0xDD, 0x42, 0x0C, 0xF5, 0x45, 0xDE, 0x43,
  0x0C, 0xF5, 0x45, 0xDF, 0x46, 0x0C, 0xF5, 0x45,
  0xE0, 0x47, 0x0C, 0xF5, 0x45, 0xE1, 0x48, 0x0C,
  0xEF, 0x45, 0xE2, 0x4A, 0x0C, 0xF5, 0x45, 0xE4,
  0x4B, 0x0C, 0xF5, 0x45, 0xE5, 0x4C, 0x0C, 0xF5,
  0x45, 0xE6, 0x4D, 0x0C, 0xF5, 0x45, 0xE8, 0x66,
  0x0C, 0xF5, 0x45, 0xF1, 0x67, 0x0C, 0xF5, 0x45,
  0xF2, 0x68, 0x0C, 0xF5, 0x45, 0xF3, 0x69, 0x0C,
  0xF5, 0x45, 0xF4, 0x6A, 0x0C, 0xF5, 0x45, 0xF5,
  0x6B, 0x0C, 0xF5, 0x45, 0xF6, 0x6C, 0x0C, 0xF5,
  0x45, 0xF7, 0x6D, 0x0C, 0xF5, 0x45, 0xF8, 0x6E,
  0x0C, 0xF5, 0x45, 0xF9, 0x6F, 0x0C, 0xF5, 0x45,
  0xFA, 0x02, 0x0D, 0xF5, 0x49, 0xA2, 0x03, 0x0D,
  0xF5, 0x49, 0xA3, 0x05, 0x0D, 0xF5, 0x49, 0xA4,
  0x06, 0x0D, 0xF5, 0x49, 0xA5, 0x07, 0x0D, 0xF5,
  0x49, 0xA6, 0x08, 0x0D, 0xF5, 0x49, 0xA7, 0x09,
  0x0D, 0xF5, 0x49, 0xA8, 0x0A, 0x0D, 0xF5, 0x49,
  0xA9, 0x0B, 0x0D, 0xF5, 0x49, 0xAA, 0x0E, 0x0D,
  0xF5, 0x49, 0xAB, 0x0F, 0x0D, 0xF5, 0x49, 0xAC,
  0x10, 0x0D, 0xEF, 0x49, 0xAD, 0x12, 0x0D, 0xF5,
  0x49, 0xAF, 0x13, 0x0D, 0xF5, 0x49, 0xB0, 0x14,
  0x0D, 0xF5, 0x49, 0xB1, 0x15, 0x0D, 0xF5, 0x49,
  0xB3, 0x16, 0x0D, 0xF5, 0x49, 0xB4, 0x17, 0x0D,
  0xF5, 0x49, 0xB5, 0x18, 0x0D, 0xF5, 0x49, 0xB6,
  0x19, 0x0D, 0xF5, 0x49, 0xB7, 0x1A, 0x0D, 0xF5,
  0x49, 0xB8, 0x1B, 0x0D, 0xF5, 0x49, 0xB9, 0x1C,
  0x0D, 0xF5, 0x49, 0xBA, 0x1D, 0x0D, 0xF5, 0x49,
  0xBB, 0x1E, 0x0D, 0xF5, 0x49, 0xBC, 0x1F, 0x0D,
  0xF5, 0x49, 0xBD, 0x20, 0x0D, 0xF5, 0x49, 0xBE,
  0x21, 0x0D, 0xF5, 0x49, 0xBF, 0x22, 0x0D, 0xF5,
  0x49, 0xC0, 0x23, 0x0D, 0xF5, 0x49, 0xC1, 0x24,
  0x0D, 0xF5, 0x49, 0xC2, 0x25, 0x0D, 0xF5, 0x49,
  0xC3, 0x26, 0x0D, 0xF5, 0x49, 0xC4, 0x27, 0x0D,
  0xF5, 0x49, 0xC5, 0x28, 0x0D, 0xEF, 0x49, 0xC6,
  0x2A, 0x0D, 0xF5, 0x49, 0xC8, 0x2B, 0x0D, 0xF5,
  0x49, 0xC9, 0x2C, 0x0D, 0xF5, 0x49, 0xCA, 0x2D,
  0x0D, 0xF5, 0x49, 0xCB, 0x2E, 0x0D, 0xF5, 0x49,
  0xCC, 0x2F, 0x0D, 0xF5, 0x49, 0xCD, 0x30, 0x0D,
  0xF5, 0x49, 0xCF, 0x31, 0x0D, 0xF5, 0x49, 0xD0,
  0x32, 0x0D, 0xF5, 0x49, 0xD1, 0x33, 0x0D, 0xF5,
  0x49, 0xD2, 0x34, 0x0D, 0xF5, 0x49, 0xD3, 0x35,
  0x0D, 0xF5, 0x49, 0xD4, 0x36, 0x0D, 0xF5, 0x49,
  0xD5, 0x37, 0x0D, 0xF5, 0x49, 0xD6, 0x38, 0x0D,
  0xF5, 0x49, 0xD7, 0x39, 0x0D, 0xF5, 0x49, 0xD8,
  0x3E, 0x0D, 0xF5, 0x49, 0xDA, 0x3F, 0x0D, 0xF5,
  0x49, 0xDB, 0x40, 0x0D, 0xF5, 0x49, 0xDC, 0x41,
  0x0D, 0xF5, 0x49, 0xDD, 0x42, 0x0D, 0xF5, 0x49,
  0xDE, 0x43, 0x0D, 0xF5, 0x49, 0xDF, 0x46, 0x0D,
  0xF5, 0x49, 0xE0, 0x47, 0x0D, 0xF5, 0x49, 0xE1,
  0x48, 0x0D, 0xEF, 0x49, 0xE2, 0x4A, 0x0D, 0xF5,
  0x49, 0xE4, 0x4B, 0x0D, 0xF5, 0x49, 0xE5, 0x4C,
  0x0D, 0xF5, 0x49, 0xE6, 0x4D, 0x0D, 0xF5, 0x49,
  0xE8, 0x66, 0x0D, 0xF5, 0x49, 0xF1, 0x67, 0x0D,
  0xF5, 0x49, 0xF2, 0x68, 0x0D, 0xF5, 0x49, 0xF3,
  0x69, 0x0D, 0xF5, 0x49, 0xF4, 0x6A, 0x0D, 0xF5,
  0x49, 0xF5, 0x6B, 0x0D, 0xF5, 0x49, 0xF6, 0x6C,
  0x0D, 0xF5, 0x49, 0xF7, 0x6D, 0x0D, 0xF5, 0x49,
  0xF8, 0x6E, 0x0D, 0xF5, 0x49, 0xF9, 0x6F, 0x0D,
  0xF5, 0x49, 0xFA, 0x2F, 0x00, 0x04, 0x0C, 0x09,
  0xF5, 0x42, 0xA6, 0xE9, 0x3D, 0x09, 0xF5, 0x42,
  0xEA, 0xE9, 0x44, 0x09, 0xF5, 0x42, 0xDF, 0xE9,
  0x50, 0x09, 0xEF, 0x42, 0xA1, 0xE9, 0x52, 0x09,
  0xEF, 0x42, 0xF0, 0xB8, 0x58, 0x09, 0xF5, 0x42,
  0xB3, 0xE9, 0x59, 0x09, 0xF5, 0x42, 0xB4, 0xE9,
  0x5A, 0x09, 0xF5, 0x42, 0xB5, 0xE9, 0x5B, 0x09,
  0xF5, 0x42, 0xBA, 0xE9, 0x5C, 0x09, 0xF5, 0x42,
  0xBF, 0xE9, 0x5D, 0x09, 0xF5, 0x42, 0xC0, 0xE9,
  0x5E, 0x09, 0xF5, 0x42, 0xC9, 0xE9, 0x60, 0x09,
  0xF5, 0x42, 0xAA, 0xE9, 0x61, 0x09, 0xF5, 0x42,
  0xA7, 0xE9, 0x62, 0x09, 0xF5, 0x42, 0xDB, 0xE9,
  0x63, 0x09, 0xF5, 0x42, 0xDC, 0xE9, 0x70, 0x09,
  0xEF, 0x42, 0xF0, 0xBF, 0x8C, 0x09, 0xF5, 0x43,
  0xA6, 0xE9, 0xC4, 0x09, 0xEF, 0x43, 0xDF, 0xE9,
  0xDC, 0x09, 0xF5, 0x43, 0xBF, 0xE9, 0xDD, 0x09,
  0xF5, 0x43, 0xC0, 0xE9, 0xE0, 0x09, 0xF5, 0x43,
  0xAA, 0xE9, 0xE1, 0x09, 0xF5, 0x43, 0xA7, 0xE9,
  0xE2, 0x09, 0xF5, 0x43, 0xDB, 0xE9, 0xE3, 0x09,
  0xEF, 0x43, 0xDC, 0xE9, 0x59, 0x0A, 0xF5, 0x4B,
  0xB4, 0xE9, 0x5A, 0x0A, 0xF5, 0x4B, 0xB5, 0xE9,
  0x5B, 0x0A, 0xF5, 0x4B, 0xBA, 0xE9, 0x5C, 0x0A,
  0xF5, 0x4B, 0xC0, 0xE9, 0x5E, 0x0A, 0xEF, 0x4B,
  0xC9, 0xE9, 0xBD, 0x0A, 0xF5, 0x4A, 0xEA, 0xE9,
  0xC4, 0x0A, 0xF5, 0x4A, 0xDF, 0xE9, 0xD0, 0x0A,
  0xEF, 0x4A, 0xA1, 0xE9, 0xE0, 0x0A, 0xEF, 0x4A,
  0xAA, 0xE9, 0x0C, 0x0B, 0xF5, 0x47, 0xA6, 0xE9,
  0x3D, 0x0B, 0xF5, 0x47, 0xEA, 0xE9, 0x5C, 0x0B,
  0xF5, 0x47, 0xBF, 0xE9, 0x5D, 0x0B, 0xF5, 0x47,
  0xC0, 0xE9, 0x60, 0x0B, 0xF5, 0x47, 0xAA, 0xE9,
  0x61, 0x0B, 0xEF, 0x47, 0xA7, 0xE9, 0x0C, 0x0C,
  0xF5, 0x45, 0xA6, 0xE9, 0x44, 0x0C, 0xEF, 0x45,
  0xDF, 0xE9, 0x60, 0x0C, 0xF5, 0x45, 0xAA, 0xE9,
  0x61, 0x0C, 0xF5, 0x45, 0xA7, 0xE9, 0x0C, 0x0D,
  0xF5, 0x49, 0xA6, 0xE9, 0x60, 0x0D, 0xF5, 0x49,
  0xAA, 0xE9, 0x61, 0x0D, 0xF5, 0x49, 0xA7, 0xE9,
  0xFF, 0xFF, 0xFF
};
#endif
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
