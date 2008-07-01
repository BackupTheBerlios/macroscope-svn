#include <stdlib.h>
#include <string.h>
#include <fbcpp/utf8embed.h>
//---------------------------------------------------------------------------
int main(int /*argc*/, char* /*argv[]*/)
{
  static const char win1251[] = "qwertyuiop[]asdfghjkl;'zxcvbnm,./`1234567890-=\\" "QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>?~!@#$%^&*()_+|" "יצףךוםדרשחץתפûגאןנמכהז‎ÿקסלטעüב‏¸.ÉÖÓÊÅÍÃØÙÇÕÚÔÛÂÀÏÐÎËÄÆÝ‗×ÑÌÈÒÜÁÞ,¨!\"¹;:?/";
  char              utf8[sizeof(win1251) * 6];
  memset(utf8, 0, sizeof(utf8));
  utf8::mbcs2utf8s(1251, utf8, sizeof(utf8), win1251, -1);
  utf8::utf8s2Lower(utf8, sizeof(utf8), utf8, sizeof(utf8));
  char  cp1251[sizeof(win1251)];
  memset(cp1251, 0, sizeof(cp1251));
  utf8::utf8s2mbcs(1251, cp1251, sizeof(cp1251), utf8, -1);
  int c = memcmp(win1251, cp1251, sizeof(win1251));

  return 0;
}
//---------------------------------------------------------------------------
