//---------------------------------------------------------------------------
#include <adicpp/lconfig.h>
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
#if defined(EMBED_CP850_SUPPORT) || defined(EMBED_ALL_CP)
extern const unsigned char cp850s2utf8s[375] = {
  0x52, 0x03, 0x00, 0x00, 0x08, 0x00, 0x7F, 0x00,
  0x00, 0x88, 0x89, 0xEA, 0x00, 0x8E, 0x8F, 0xC4,
  0x00, 0xB0, 0xB2, 0x91, 0x25, 0xB5, 0xB6, 0xC1,
  0x00, 0xD2, 0xD3, 0xCA, 0x00, 0xD6, 0xD8, 0xCD,
  0x00, 0xE9, 0xEA, 0xDA, 0x00, 0x70, 0x80, 0xC7,
  0x00, 0x81, 0xFC, 0x00, 0x82, 0xE9, 0x00, 0x83,
  0xE2, 0x00, 0x84, 0xE4, 0x00, 0x85, 0xE0, 0x00,
  0x86, 0xE5, 0x00, 0x87, 0xE7, 0x00, 0x8A, 0xE8,
  0x00, 0x8B, 0xEF, 0x00, 0x8C, 0xEE, 0x00, 0x8D,
  0xEC, 0x00, 0x90, 0xC9, 0x00, 0x91, 0xE6, 0x00,
  0x92, 0xC6, 0x00, 0x93, 0xF4, 0x00, 0x94, 0xF6,
  0x00, 0x95, 0xF2, 0x00, 0x96, 0xFB, 0x00, 0x97,
  0xF9, 0x00, 0x98, 0xFF, 0x00, 0x99, 0xD6, 0x00,
  0x9A, 0xDC, 0x00, 0x9B, 0xF8, 0x00, 0x9C, 0xA3,
  0x00, 0x9D, 0xD8, 0x00, 0x9E, 0xD7, 0x00, 0x9F,
  0x92, 0x01, 0xA0, 0xE1, 0x00, 0xA1, 0xED, 0x00,
  0xA2, 0xF3, 0x00, 0xA3, 0xFA, 0x00, 0xA4, 0xF1,
  0x00, 0xA5, 0xD1, 0x00, 0xA6, 0xAA, 0x00, 0xA7,
  0xBA, 0x00, 0xA8, 0xBF, 0x00, 0xA9, 0xAE, 0x00,
  0xAA, 0xAC, 0x00, 0xAB, 0xBD, 0x00, 0xAC, 0xBC,
  0x00, 0xAD, 0xA1, 0x00, 0xAE, 0xAB, 0x00, 0xAF,
  0xBB, 0x00, 0xB3, 0x02, 0x25, 0xB4, 0x24, 0x25,
  0xB7, 0xC0, 0x00, 0xB8, 0xA9, 0x00, 0xB9, 0x63,
  0x25, 0xBA, 0x51, 0x25, 0xBB, 0x57, 0x25, 0xBC,
  0x5D, 0x25, 0xBD, 0xA2, 0x00, 0xBE, 0xA5, 0x00,
  0xBF, 0x10, 0x25, 0xC0, 0x14, 0x25, 0xC1, 0x34,
  0x25, 0xC2, 0x2C, 0x25, 0xC3, 0x1C, 0x25, 0xC4,
  0x00, 0x25, 0xC5, 0x3C, 0x25, 0xC6, 0xE3, 0x00,
  0xC7, 0xC3, 0x00, 0xC8, 0x5A, 0x25, 0xC9, 0x54,
  0x25, 0xCA, 0x69, 0x25, 0xCB, 0x66, 0x25, 0xCC,
  0x60, 0x25, 0xCD, 0x50, 0x25, 0xCE, 0x6C, 0x25,
  0xCF, 0xA4, 0x00, 0xD0, 0xF0, 0x00, 0xD1, 0xD0,
  0x00, 0xD4, 0xC8, 0x00, 0xD5, 0x31, 0x01, 0xD9,
  0x18, 0x25, 0xDA, 0x0C, 0x25, 0xDB, 0x88, 0x25,
  0xDC, 0x84, 0x25, 0xDD, 0xA6, 0x00, 0xDE, 0xCC,
  0x00, 0xDF, 0x80, 0x25, 0xE0, 0xD3, 0x00, 0xE1,
  0xDF, 0x00, 0xE2, 0xD4, 0x00, 0xE3, 0xD2, 0x00,
  0xE4, 0xF5, 0x00, 0xE5, 0xD5, 0x00, 0xE6, 0xB5,
  0x00, 0xE7, 0xFE, 0x00, 0xE8, 0xDE, 0x00, 0xEB,
  0xD9, 0x00, 0xEC, 0xFD, 0x00, 0xED, 0xDD, 0x00,
  0xEE, 0xAF, 0x00, 0xEF, 0xB4, 0x00, 0xF0, 0xAD,
  0x00, 0xF1, 0xB1, 0x00, 0xF2, 0x17, 0x20, 0xF3,
  0xBE, 0x00, 0xF4, 0xB6, 0x00, 0xF5, 0xA7, 0x00,
  0xF6, 0xF7, 0x00, 0xF7, 0xB8, 0x00, 0xF8, 0xB0,
  0x00, 0xF9, 0xA8, 0x00, 0xFA, 0xB7, 0x00, 0xFB,
  0xB9, 0x00, 0xFC, 0xB3, 0x00, 0xFD, 0xB2, 0x00,
  0xFE, 0xA0, 0x25, 0xFF, 0xA0, 0x00, 0x00
};
//---------------------------------------------------------------------------
extern const unsigned char utf8s2cp850s[1490] = {
  0x01, 0x3F, 0x19, 0x01, 0x00, 0x00, 0x7F, 0x00,
  0x00, 0xC1, 0x00, 0xC2, 0x00, 0xB5, 0xC4, 0x00,
  0xC5, 0x00, 0x8E, 0xCA, 0x00, 0xCB, 0x00, 0xD2,
  0xCD, 0x00, 0xCF, 0x00, 0xD6, 0xDA, 0x00, 0xDB,
  0x00, 0xE9, 0xEA, 0x00, 0xEB, 0x00, 0x88, 0x78,
  0x01, 0x79, 0x01, 0x59, 0xAE, 0x01, 0xAF, 0x01,
  0x54, 0xB4, 0x03, 0xB5, 0x03, 0x64, 0xC3, 0x03,
  0xC4, 0x03, 0x73, 0x74, 0x20, 0x77, 0x20, 0x34,
  0x84, 0x20, 0x89, 0x20, 0x34, 0x1A, 0x21, 0x1B,
  0x21, 0x51, 0x2C, 0x21, 0x2D, 0x21, 0x42, 0x30,
  0x21, 0x31, 0x21, 0x45, 0x6A, 0x22, 0x6B, 0x22,
  0xAE, 0x20, 0x23, 0x21, 0x23, 0x28, 0x91, 0x25,
  0x93, 0x25, 0xB0, 0x3A, 0x26, 0x3B, 0x26, 0x01,
  0x65, 0x26, 0x66, 0x26, 0x03, 0x6A, 0x26, 0x6B,
  0x26, 0x0D, 0x0A, 0x30, 0x0B, 0x30, 0xAE, 0x01,
  0xFF, 0x1E, 0xFF, 0x21, 0x20, 0xFF, 0x5E, 0xFF,
  0x40, 0xFF, 0xFF, 0xC3, 0x01, 0x01, 0xA0, 0x00,
  0xFF, 0xA1, 0x00, 0xAD, 0xA2, 0x00, 0xBD, 0xA3,
  0x00, 0x9C, 0xA4, 0x00, 0xCF, 0xA5, 0x00, 0xBE,
  0xA6, 0x00, 0xDD, 0xA7, 0x00, 0xF5, 0xA8, 0x00,
  0xF9, 0xA9, 0x00, 0xB8, 0xAA, 0x00, 0xA6, 0xAB,
  0x00, 0xAE, 0xAC, 0x00, 0xAA, 0xAD, 0x00, 0xF0,
  0xAE, 0x00, 0xA9, 0xAF, 0x00, 0xEE, 0xB0, 0x00,
  0xF8, 0xB1, 0x00, 0xF1, 0xB2, 0x00, 0xFD, 0xB3,
  0x00, 0xFC, 0xB4, 0x00, 0xEF, 0xB5, 0x00, 0xE6,
  0xB6, 0x00, 0xF4, 0xB7, 0x00, 0xFA, 0xB8, 0x00,
  0xF7, 0xB9, 0x00, 0xFB, 0xBA, 0x00, 0xA7, 0xBB,
  0x00, 0xAF, 0xBC, 0x00, 0xAC, 0xBD, 0x00, 0xAB,
  0xBE, 0x00, 0xF3, 0xBF, 0x00, 0xA8, 0xC0, 0x00,
  0xB7, 0xC3, 0x00, 0xC7, 0xC6, 0x00, 0x92, 0xC7,
  0x00, 0x80, 0xC8, 0x00, 0xD4, 0xC9, 0x00, 0x90,
  0xCC, 0x00, 0xDE, 0xD0, 0x00, 0xD1, 0xD1, 0x00,
  0xA5, 0xD2, 0x00, 0xE3, 0xD3, 0x00, 0xE0, 0xD4,
  0x00, 0xE2, 0xD5, 0x00, 0xE5, 0xD6, 0x00, 0x99,
  0xD7, 0x00, 0x9E, 0xD8, 0x00, 0x9D, 0xD9, 0x00,
  0xEB, 0xDC, 0x00, 0x9A, 0xDD, 0x00, 0xED, 0xDE,
  0x00, 0xE8, 0xDF, 0x00, 0xE1, 0xE0, 0x00, 0x85,
  0xE1, 0x00, 0xA0, 0xE2, 0x00, 0x83, 0xE3, 0x00,
  0xC6, 0xE4, 0x00, 0x84, 0xE5, 0x00, 0x86, 0xE6,
  0x00, 0x91, 0xE7, 0x00, 0x87, 0xE8, 0x00, 0x8A,
  0xE9, 0x00, 0x82, 0xEC, 0x00, 0x8D, 0xED, 0x00,
  0xA1, 0xEE, 0x00, 0x8C, 0xEF, 0x00, 0x8B, 0xF0,
  0x00, 0xD0, 0xF1, 0x00, 0xA4, 0xF2, 0x00, 0x95,
  0xF3, 0x00, 0xA2, 0xF4, 0x00, 0x93, 0xF5, 0x00,
  0xE4, 0xF6, 0x00, 0x94, 0xF7, 0x00, 0xF6, 0xF8,
  0x00, 0x9B, 0xF9, 0x00, 0x97, 0xFA, 0x00, 0xA3,
  0xFB, 0x00, 0x96, 0xFC, 0x00, 0x81, 0xFD, 0x00,
  0xEC, 0xFE, 0x00, 0xE7, 0xFF, 0x00, 0x98, 0x00,
  0x01, 0x41, 0x01, 0x01, 0x61, 0x02, 0x01, 0x41,
  0x03, 0x01, 0x61, 0x04, 0x01, 0x41, 0x05, 0x01,
  0x61, 0x06, 0x01, 0x43, 0x07, 0x01, 0x63, 0x08,
  0x01, 0x43, 0x09, 0x01, 0x63, 0x0A, 0x01, 0x43,
  0x0B, 0x01, 0x63, 0x0C, 0x01, 0x43, 0x0D, 0x01,
  0x63, 0x0E, 0x01, 0x44, 0x0F, 0x01, 0x64, 0x10,
  0x01, 0x44, 0x11, 0x01, 0x64, 0x12, 0x01, 0x45,
  0x13, 0x01, 0x65, 0x14, 0x01, 0x45, 0x15, 0x01,
  0x65, 0x16, 0x01, 0x45, 0x17, 0x01, 0x65, 0x18,
  0x01, 0x45, 0x19, 0x01, 0x65, 0x1A, 0x01, 0x45,
  0x1B, 0x01, 0x65, 0x1C, 0x01, 0x47, 0x1D, 0x01,
  0x67, 0x1E, 0x01, 0x47, 0x1F, 0x01, 0x67, 0x20,
  0x01, 0x47, 0x21, 0x01, 0x67, 0x22, 0x01, 0x47,
  0x23, 0x01, 0x67, 0x24, 0x01, 0x48, 0x25, 0x01,
  0x68, 0x26, 0x01, 0x48, 0x27, 0x01, 0x68, 0x28,
  0x01, 0x49, 0x29, 0x01, 0x69, 0x2A, 0x01, 0x49,
  0x2B, 0x01, 0x69, 0x2C, 0x01, 0x49, 0x2D, 0x01,
  0x69, 0x2E, 0x01, 0x49, 0x2F, 0x01, 0x69, 0x30,
  0x01, 0x49, 0x31, 0x01, 0xD5, 0x34, 0x01, 0x4A,
  0x35, 0x01, 0x6A, 0x36, 0x01, 0x4B, 0x37, 0x01,
  0x6B, 0x39, 0x01, 0x4C, 0x3A, 0x01, 0x6C, 0x3B,
  0x01, 0x4C, 0x3C, 0x01, 0x6C, 0x3D, 0x01, 0x4C,
  0x3E, 0x01, 0x6C, 0x41, 0x01, 0x4C, 0x42, 0x01,
  0x6C, 0x43, 0x01, 0x4E, 0x44, 0x01, 0x6E, 0x45,
  0x01, 0x4E, 0x46, 0x01, 0x6E, 0x47, 0x01, 0x4E,
  0x48, 0x01, 0x6E, 0x4C, 0x01, 0x4F, 0x4D, 0x01,
  0x6F, 0x4E, 0x01, 0x4F, 0x4F, 0x01, 0x6F, 0x50,
  0x01, 0x4F, 0x51, 0x01, 0x6F, 0x52, 0x01, 0x4F,
  0x53, 0x01, 0x6F, 0x54, 0x01, 0x52, 0x55, 0x01,
  0x72, 0x56, 0x01, 0x52, 0x57, 0x01, 0x72, 0x58,
  0x01, 0x52, 0x59, 0x01, 0x72, 0x5A, 0x01, 0x53,
  0x5B, 0x01, 0x73, 0x5C, 0x01, 0x53, 0x5D, 0x01,
  0x73, 0x5E, 0x01, 0x53, 0x5F, 0x01, 0x73, 0x60,
  0x01, 0x53, 0x61, 0x01, 0x73, 0x62, 0x01, 0x54,
  0x63, 0x01, 0x74, 0x64, 0x01, 0x54, 0x65, 0x01,
  0x74, 0x66, 0x01, 0x54, 0x67, 0x01, 0x74, 0x68,
  0x01, 0x55, 0x69, 0x01, 0x75, 0x6A, 0x01, 0x55,
  0x6B, 0x01, 0x75, 0x6C, 0x01, 0x55, 0x6D, 0x01,
  0x75, 0x6E, 0x01, 0x55, 0x6F, 0x01, 0x75, 0x70,
  0x01, 0x55, 0x71, 0x01, 0x75, 0x72, 0x01, 0x55,
  0x73, 0x01, 0x75, 0x74, 0x01, 0x57, 0x75, 0x01,
  0x77, 0x76, 0x01, 0x59, 0x77, 0x01, 0x79, 0x7A,
  0x01, 0x7A, 0x7B, 0x01, 0x5A, 0x7C, 0x01, 0x7A,
  0x7D, 0x01, 0x5A, 0x7E, 0x01, 0x7A, 0x80, 0x01,
  0x62, 0x89, 0x01, 0x44, 0x91, 0x01, 0xA5, 0x92,
  0x01, 0xA5, 0x97, 0x01, 0x49, 0x9A, 0x01, 0x6C,
  0x9F, 0x01, 0x55, 0xA0, 0x01, 0x55, 0xA1, 0x01,
  0x6F, 0xA9, 0x01, 0x53, 0xAB, 0x01, 0x74, 0xB0,
  0x01, 0x75, 0xB6, 0x01, 0x5A, 0xC0, 0x01, 0xB3,
  0xC3, 0x01, 0x21, 0xCD, 0x01, 0x41, 0xCE, 0x01,
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
  0x01, 0x6A, 0x61, 0x02, 0x67, 0x78, 0x02, 0x9B,
  0xB9, 0x02, 0xEF, 0xBA, 0x02, 0x22, 0xBC, 0x02,
  0x27, 0xC4, 0x02, 0x5E, 0xC6, 0x02, 0x5E, 0xC8,
  0x02, 0x27, 0xC9, 0x02, 0xC4, 0xCA, 0x02, 0xEF,
  0xCB, 0x02, 0x27, 0xCD, 0x02, 0x5F, 0xDA, 0x02,
  0xF8, 0xDC, 0x02, 0x7E, 0x00, 0x03, 0x27, 0x01,
  0x03, 0xEF, 0x02, 0x03, 0x5E, 0x03, 0x03, 0x7E,
  0x04, 0x03, 0xF4, 0x05, 0x03, 0xF4, 0x08, 0x03,
  0xF9, 0x0A, 0x03, 0xF8, 0x0E, 0x03, 0x22, 0x27,
  0x03, 0xF7, 0x31, 0x03, 0x65, 0x32, 0x03, 0x65,
  0x7E, 0x03, 0x3B, 0x93, 0x03, 0x47, 0x98, 0x03,
  0x9D, 0xA3, 0x03, 0x53, 0xA6, 0x03, 0x46, 0xA9,
  0x03, 0x4F, 0xB1, 0x03, 0x61, 0xB2, 0x03, 0xE1,
  0xBC, 0x03, 0xE6, 0xC0, 0x03, 0x70, 0xC6, 0x03,
  0x66, 0xBB, 0x04, 0x68, 0x89, 0x05, 0x3A, 0x6A,
  0x06, 0x25, 0x00, 0x20, 0x26, 0x01, 0x20, 0x26,
  0x02, 0x20, 0x26, 0x03, 0x20, 0x26, 0x04, 0x20,
  0x26, 0x05, 0x20, 0x26, 0x06, 0x20, 0x20, 0x10,
  0x20, 0x33, 0x11, 0x20, 0x33, 0x13, 0x20, 0x33,
  0x14, 0x20, 0x33, 0x17, 0x20, 0xF2, 0x18, 0x20,
  0x2D, 0x19, 0x20, 0x2D, 0x1A, 0x20, 0x27, 0x1C,
  0x20, 0x28, 0x1D, 0x20, 0x28, 0x1E, 0x20, 0x22,
  0x20, 0x20, 0xC5, 0x21, 0x20, 0xCE, 0x22, 0x20,
  0x07, 0x24, 0x20, 0x07, 0x26, 0x20, 0x2E, 0x30,
  0x20, 0x25, 0x32, 0x20, 0xEF, 0x35, 0x20, 0xEF,
  0x39, 0x20, 0x3C, 0x3A, 0x20, 0x3E, 0x3C, 0x20,
  0x13, 0x44, 0x20, 0x2F, 0x70, 0x20, 0x30, 0x78,
  0x20, 0x39, 0x7F, 0x20, 0x6E, 0x80, 0x20, 0x30,
  0x81, 0x20, 0xFB, 0x82, 0x20, 0xFD, 0x83, 0x20,
  0xFC, 0xA4, 0x20, 0x9C, 0xA7, 0x20, 0x50, 0xDD,
  0x20, 0x4F, 0x02, 0x21, 0x43, 0x07, 0x21, 0x45,
  0x0A, 0x21, 0x67, 0x0B, 0x21, 0x4E, 0x0C, 0x21,
  0x4E, 0x0D, 0x21, 0x48, 0x0E, 0x21, 0x68, 0x10,
  0x21, 0x4F, 0x11, 0x21, 0x4F, 0x12, 0x21, 0x4C,
  0x13, 0x21, 0x6C, 0x15, 0x21, 0x4E, 0x18, 0x21,
  0x56, 0x19, 0x21, 0x56, 0x1C, 0x21, 0x58, 0x1D,
  0x21, 0x52, 0x22, 0x21, 0x54, 0x24, 0x21, 0x5A,
  0x26, 0x21, 0x4F, 0x28, 0x21, 0x5A, 0x2A, 0x21,
  0x4B, 0x2B, 0x21, 0x8F, 0x2E, 0x21, 0x6B, 0x2F,
  0x21, 0x6B, 0x33, 0x21, 0x4D, 0x34, 0x21, 0x6F,
  0x90, 0x21, 0x1B, 0x91, 0x21, 0x18, 0x92, 0x21,
  0x1A, 0x93, 0x21, 0x19, 0x94, 0x21, 0x1D, 0x95,
  0x21, 0x12, 0xA8, 0x21, 0x17, 0x05, 0x22, 0x9D,
  0x11, 0x22, 0x53, 0x12, 0x22, 0x2D, 0x13, 0x22,
  0xF1, 0x15, 0x22, 0x35, 0x16, 0x22, 0x35, 0x17,
  0x22, 0x2A, 0x18, 0x22, 0xF8, 0x19, 0x22, 0x07,
  0x1A, 0x22, 0x56, 0x1E, 0x22, 0x38, 0x1F, 0x22,
  0x1C, 0x23, 0x22, 0xB3, 0x29, 0x22, 0x6E, 0x36,
  0x22, 0x3A, 0x3C, 0x22, 0x7E, 0x48, 0x22, 0x7E,
  0x61, 0x22, 0x3D, 0x64, 0x22, 0x43, 0x65, 0x22,
  0x43, 0xC5, 0x22, 0xFA, 0x02, 0x23, 0x7F, 0x03,
  0x23, 0x5E, 0x10, 0x23, 0xAA, 0x29, 0x23, 0x3C,
  0x2A, 0x23, 0x3E, 0x00, 0x25, 0xC4, 0x02, 0x25,
  0xB3, 0x0C, 0x25, 0xDA, 0x10, 0x25, 0xBF, 0x14,
  0x25, 0xC0, 0x18, 0x25, 0xD9, 0x1C, 0x25, 0xC3,
  0x24, 0x25, 0xB4, 0x2C, 0x25, 0xC2, 0x34, 0x25,
  0xC1, 0x3C, 0x25, 0xC5, 0x50, 0x25, 0xCD, 0x51,
  0x25, 0xBA, 0x52, 0x25, 0xCF, 0x53, 0x25, 0xCF,
  0x54, 0x25, 0xC9, 0x55, 0x25, 0xC1, 0x56, 0x25,
  0xC1, 0x57, 0x25, 0xBB, 0x58, 0x25, 0xCE, 0x59,
  0x25, 0xCE, 0x5A, 0x25, 0xC8, 0x5B, 0x25, 0xC2,
  0x5C, 0x25, 0xC2, 0x5D, 0x25, 0xBC, 0x5E, 0x25,
  0xD2, 0x5F, 0x25, 0xD2, 0x60, 0x25, 0xCC, 0x61,
  0x25, 0xBF, 0x62, 0x25, 0xBF, 0x63, 0x25, 0xB9,
  0x64, 0x25, 0xD1, 0x65, 0x25, 0xD1, 0x66, 0x25,
  0xCB, 0x67, 0x25, 0xD0, 0x68, 0x25, 0xD0, 0x69,
  0x25, 0xCA, 0x6A, 0x25, 0xD4, 0x6B, 0x25, 0xD4,
  0x6C, 0x25, 0xCE, 0x80, 0x25, 0xDF, 0x84, 0x25,
  0xDC, 0x88, 0x25, 0xDB, 0x8C, 0x25, 0xDB, 0x90,
  0x25, 0xDB, 0xA0, 0x25, 0xFE, 0xAC, 0x25, 0x16,
  0xB2, 0x25, 0x1E, 0xBA, 0x25, 0x10, 0xBC, 0x25,
  0x1F, 0xC4, 0x25, 0x11, 0xCB, 0x25, 0x09, 0xD8,
  0x25, 0x08, 0xD9, 0x25, 0x0A, 0x3C, 0x26, 0x0F,
  0x40, 0x26, 0x0C, 0x42, 0x26, 0x0B, 0x60, 0x26,
  0x06, 0x63, 0x26, 0x05, 0x13, 0x27, 0x56, 0x58,
  0x27, 0xB3, 0x00, 0x30, 0x20, 0x07, 0x30, 0x4F,
  0x08, 0x30, 0x3C, 0x09, 0x30, 0x3E, 0x1A, 0x30,
  0x5B, 0x1B, 0x30, 0x5D, 0xFB, 0x30, 0xFA, 0xFF,
  0xFF, 0xFF
};
#endif
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
