eac=yes

AC_ARG_WITH(
  [cp37],
  AC_HELP_STRING([--with-cp37],[IBM EBCDIC US Canada]),
  AC_DEFINE([EMBED_CP37_SUPPORT],[1],[Define to 1 if you want IBM EBCDIC US Canada support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp437],
  AC_HELP_STRING([--with-cp437],[OEM United States]),
  AC_DEFINE([EMBED_CP437_SUPPORT],[1],[Define to 1 if you want OEM United States support.]),
  [eac=no]
)
    
AC_ARG_WITH(
  [cp500],
  AC_HELP_STRING([--with-cp500],[IBM EBCDIC International]),
  AC_DEFINE([EMBED_CP500_SUPPORT],[1],[Define to 1 if you want IBM EBCDIC International support.]),
  [eac=no]
)
    
AC_ARG_WITH(
  [cp708],
  AC_HELP_STRING([--with-cp708],[codepage 708]),
  AC_DEFINE([EMBED_CP708_SUPPORT],[1],[Define to 1 if you want codepage 708 support.]),
  [eac=no]
)
    
AC_ARG_WITH(
  [cp720],
  AC_HELP_STRING([--with-cp720],[codepage 720]),
  AC_DEFINE([EMBED_CP708_SUPPORT],[1],[Define to 1 if you want codepage 720 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp737],
  AC_HELP_STRING([--with-cp737],[OEM Greek 437G]),
  AC_DEFINE([EMBED_CP737_SUPPORT],[1],[Define to 1 if you want codepage 720 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp775],
  AC_HELP_STRING([--with-cp775],[OEM Baltic]),
  AC_DEFINE([EMBED_CP775_SUPPORT],[1],[Define to 1 if you want OEM Baltic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp850],
  AC_HELP_STRING([--with-cp850],[OEM Multilingual Latin 1]),
  AC_DEFINE([EMBED_CP850_SUPPORT],[1],[Define to 1 if you want OEM Multilingual Latin 1 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp852],
  AC_HELP_STRING([--with-cp852],[OEM Slovak Latin 2]),
  AC_DEFINE([EMBED_CP852_SUPPORT],[1],[Define to 1 if you want OEM Slovak Latin 2 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp855],
  AC_HELP_STRING([--with-cp855],[OEM Cyrillic]),
  AC_DEFINE([EMBED_CP855_SUPPORT],[1],[Define to 1 if you want OEM Cyrillic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp857],
  AC_HELP_STRING([--with-cp857],[OEM Turkish]),
  AC_DEFINE([EMBED_CP857_SUPPORT],[1],[Define to 1 if you want OEM Turkish support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp857],
  AC_HELP_STRING([--with-cp858],[codepage 858]),
  AC_DEFINE([EMBED_CP858_SUPPORT],[1],[Define to 1 if you want codepage 858 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp860],
  AC_HELP_STRING([--with-cp860],[OEM Portuguese]),
  AC_DEFINE([EMBED_CP860_SUPPORT],[1],[Define to 1 if you want OEM Portuguese support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp861],
  AC_HELP_STRING([--with-cp861],[OEM Icelandic]),
  AC_DEFINE([EMBED_CP861_SUPPORT],[1],[Define to 1 if you want OEM Icelandic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp862],
  AC_HELP_STRING([--with-cp862],[OEM Hebrew]),
  AC_DEFINE([EMBED_CP862_SUPPORT],[1],[Define to 1 if you want OEM Hebrew support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp863],
  AC_HELP_STRING([--with-cp863],[OEM Canadian French]),
  AC_DEFINE([EMBED_CP863_SUPPORT],[1],[Define to 1 if you want OEM Canadian French support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp864],
  AC_HELP_STRING([--with-cp864],[OEM Arabic]),
  AC_DEFINE([EMBED_CP864_SUPPORT],[1],[Define to 1 if you want OEM Arabic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp865],
  AC_HELP_STRING([--with-cp865],[OEM Nordic]),
  AC_DEFINE([EMBED_CP865_SUPPORT],[1],[Define to 1 if you want OEM Nordic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp866],
  AC_HELP_STRING([--with-cp866],[OEM Russian]),
  AC_DEFINE([EMBED_CP866_SUPPORT],[1],[Define to 1 if you want OEM Russian support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp869],
  AC_HELP_STRING([--with-cp869],[OEM Greek]),
  AC_DEFINE([EMBED_CP869_SUPPORT],[1],[Define to 1 if you want OEM Greek support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp870],
  AC_HELP_STRING([--with-cp870],[codepage 870]),
  AC_DEFINE([EMBED_CP870_SUPPORT],[1],[Define to 1 if you want codepage 870 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp874],
  AC_HELP_STRING([--with-cp874],[ANSI/OEM Thai]),
  AC_DEFINE([EMBED_CP874_SUPPORT],[1],[Define to 1 if you want ANSI/OEM Thai support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp875],
  AC_HELP_STRING([--with-cp875],[IBM EBCDIC Greek]),
  AC_DEFINE([EMBED_CP875_SUPPORT],[1],[Define to 1 if you want IBM EBCDIC Greek support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp932],
  AC_HELP_STRING([--with-cp932],[ANSI/OEM Japanese Shift-JIS]),
  AC_DEFINE([EMBED_CP932_SUPPORT],[1],[Define to 1 if you want ANSI/OEM Japanese Shift-JIS support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp936],
  AC_HELP_STRING([--with-cp936],[ANSI/OEM Simplified Chinese GBK]),
  AC_DEFINE([EMBED_CP936_SUPPORT],[1],[Define to 1 if you want ANSI/OEM Simplified Chinese GBK support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp949],
  AC_HELP_STRING([--with-cp949],[ANSI/OEM Korean Unified Hangul]),
  AC_DEFINE([EMBED_CP949_SUPPORT],[1],[Define to 1 if you want ANSI/OEM Korean Unified Hangul support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp950],
  AC_HELP_STRING([--with-cp950],[ANSI/OEM Traditional Chinese Big5]),
  AC_DEFINE([EMBED_CP950_SUPPORT],[1],[Define to 1 if you want ANSI/OEM Traditional Chinese Big5 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1026],
  AC_HELP_STRING([--with-cp1026],[IBM EBCDIC Latin 5 Turkish]),
  AC_DEFINE([EMBED_CP1026_SUPPORT],[1],[Define to 1 if you want IBM EBCDIC Latin 5 Turkish support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1047],
  AC_HELP_STRING([--with-cp1047],[codepage 1047]),
  AC_DEFINE([EMBED_CP1047_SUPPORT],[1],[Define to 1 if you want codepage 1047 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1140],
  AC_HELP_STRING([--with-cp1140],[codepage 1140]),
  AC_DEFINE([EMBED_CP1140_SUPPORT],[1],[Define to 1 if you want codepage 1140 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1141],
  AC_HELP_STRING([--with-cp1141],[codepage 1141]),
  AC_DEFINE([EMBED_CP1141_SUPPORT],[1],[Define to 1 if you want codepage 1141 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1142],
  AC_HELP_STRING([--with-cp1142],[codepage 1142]),
  AC_DEFINE([EMBED_CP1142_SUPPORT],[1],[Define to 1 if you want codepage 1142 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1143],
  AC_HELP_STRING([--with-cp1143],[codepage 1143]),
  AC_DEFINE([EMBED_CP1143_SUPPORT],[1],[Define to 1 if you want codepage 1143 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1144],
  AC_HELP_STRING([--with-cp1144],[codepage 1144]),
  AC_DEFINE([EMBED_CP1144_SUPPORT],[1],[Define to 1 if you want codepage 1144 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1145],
  AC_HELP_STRING([--with-cp1145],[codepage 1145]),
  AC_DEFINE([EMBED_CP1145_SUPPORT],[1],[Define to 1 if you want codepage 1145 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1146],
  AC_HELP_STRING([--with-cp1146],[codepage 1146]),
  AC_DEFINE([EMBED_CP1146_SUPPORT],[1],[Define to 1 if you want codepage 1146 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1147],
  AC_HELP_STRING([--with-cp1147],[codepage 1147]),
  AC_DEFINE([EMBED_CP1147_SUPPORT],[1],[Define to 1 if you want codepage 1147 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1148],
  AC_HELP_STRING([--with-cp1148],[codepage 1148]),
  AC_DEFINE([EMBED_CP1148_SUPPORT],[1],[Define to 1 if you want codepage 1148 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1149],
  AC_HELP_STRING([--with-cp1149],[codepage 1149]),
  AC_DEFINE([EMBED_CP1149_SUPPORT],[1],[Define to 1 if you want codepage 1149 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1250],
  AC_HELP_STRING([--with-cp1250],[ANSI Eastern Europe]),
  AC_DEFINE([EMBED_CP1250_SUPPORT],[1],[Define to 1 if you want ANSI Eastern Europe support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1251],
  AC_HELP_STRING([--with-cp1251],[ANSI Cyrillic]),
  AC_DEFINE([EMBED_CP1251_SUPPORT],[1],[Define to 1 if you want ANSI Cyrillic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1252],
  AC_HELP_STRING([--with-cp1252],[ANSI Latin 1]),
  AC_DEFINE([EMBED_CP1252_SUPPORT],[1],[Define to 1 if you want ANSI Latin 1 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1253],
  AC_HELP_STRING([--with-cp1253],[ANSI Greek]),
  AC_DEFINE([EMBED_CP1253_SUPPORT],[1],[Define to 1 if you want ANSI Greek support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1254],
  AC_HELP_STRING([--with-cp1254],[ANSI Turkish]),
  AC_DEFINE([EMBED_CP1254_SUPPORT],[1],[Define to 1 if you want ANSI Turkish support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1255],
  AC_HELP_STRING([--with-cp1255],[ANSI Hebrew]),
  AC_DEFINE([EMBED_CP1255_SUPPORT],[1],[Define to 1 if you want ANSI Hebrew support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1256],
  AC_HELP_STRING([--with-cp1256],[ANSI Arabic]),
  AC_DEFINE([EMBED_CP1256_SUPPORT],[1],[Define to 1 if you want ANSI Arabic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1257],
  AC_HELP_STRING([--with-cp1257],[ANSI Baltic]),
  AC_DEFINE([EMBED_CP1257_SUPPORT],[1],[Define to 1 if you want ANSI Baltic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1258],
  AC_HELP_STRING([--with-cp1258],[ANSI/OEM Viet Nam]),
  AC_DEFINE([EMBED_CP1258_SUPPORT],[1],[Define to 1 if you want ANSI/OEM Viet Nam support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp1361],
  AC_HELP_STRING([--with-cp1361],[codepage 1361]),
  AC_DEFINE([EMBED_CP1361_SUPPORT],[1],[Define to 1 if you want codepage 1361 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10000],
  AC_HELP_STRING([--with-cp10000],[Mac Roman]),
  AC_DEFINE([EMBED_CP10000_SUPPORT],[1],[Define to 1 if you want Mac Roman support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10001],
  AC_HELP_STRING([--with-cp10001],[codepage 10001]),
  AC_DEFINE([EMBED_CP10001_SUPPORT],[1],[Define to 1 if you want codepage 10001 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10002],
  AC_HELP_STRING([--with-cp10002],[codepage 10002]),
  AC_DEFINE([EMBED_CP10002_SUPPORT],[1],[Define to 1 if you want codepage 10002 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10003],
  AC_HELP_STRING([--with-cp10003],[codepage 10003]),
  AC_DEFINE([EMBED_CP10003_SUPPORT],[1],[Define to 1 if you want codepage 10003 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10004],
  AC_HELP_STRING([--with-cp10004],[codepage 10004]),
  AC_DEFINE([EMBED_CP10004_SUPPORT],[1],[Define to 1 if you want codepage 10004 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10005],
  AC_HELP_STRING([--with-cp10005],[codepage 10005]),
  AC_DEFINE([EMBED_CP10005_SUPPORT],[1],[Define to 1 if you want codepage 10005 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10006],
  AC_HELP_STRING([--with-cp10006],[Mac Greek]),
  AC_DEFINE([EMBED_CP10006_SUPPORT],[1],[Define to 1 if you want Mac Greek support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10007],
  AC_HELP_STRING([--with-cp10007],[Mac Cyrillic]),
  AC_DEFINE([EMBED_CP10007_SUPPORT],[1],[Define to 1 if you want Mac Cyrillic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10008],
  AC_HELP_STRING([--with-cp10008],[codepage 10008]),
  AC_DEFINE([EMBED_CP10008_SUPPORT],[1],[Define to 1 if you want codepage 10008 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10010],
  AC_HELP_STRING([--with-cp10010],[codepage 10010]),
  AC_DEFINE([EMBED_CP10010_SUPPORT],[1],[Define to 1 if you want codepage 10010 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10017],
  AC_HELP_STRING([--with-cp10017],[codepage 10017]),
  AC_DEFINE([EMBED_CP10017_SUPPORT],[1],[Define to 1 if you want codepage 10017 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10021],
  AC_HELP_STRING([--with-cp10021],[codepage 10021]),
  AC_DEFINE([EMBED_CP10021_SUPPORT],[1],[Define to 1 if you want codepage 10021 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10029],
  AC_HELP_STRING([--with-cp10029],[Mac Latin 2]),
  AC_DEFINE([EMBED_CP10029_SUPPORT],[1],[Define to 1 if you want Mac Latin 2 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10079],
  AC_HELP_STRING([--with-cp10079],[Mac Icelandic]),
  AC_DEFINE([EMBED_CP10079_SUPPORT],[1],[Define to 1 if you want Mac Icelandic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10081],
  AC_HELP_STRING([--with-cp10081],[Mac Turkish]),
  AC_DEFINE([EMBED_CP10081_SUPPORT],[1],[Define to 1 if you want Mac Turkish support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp10082],
  AC_HELP_STRING([--with-cp10082],[codepage 10082]),
  AC_DEFINE([EMBED_CP10082_SUPPORT],[1],[Define to 1 if you want codepage 10082 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20000],
  AC_HELP_STRING([--with-cp20000],[codepage 20000]),
  AC_DEFINE([EMBED_CP20000_SUPPORT],[1],[Define to 1 if you want codepage 20000 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20001],
  AC_HELP_STRING([--with-cp20001],[codepage 20001]),
  AC_DEFINE([EMBED_CP20001_SUPPORT],[1],[Define to 1 if you want codepage 20001 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20002],
  AC_HELP_STRING([--with-cp20002],[codepage 20002]),
  AC_DEFINE([EMBED_CP20002_SUPPORT],[1],[Define to 1 if you want codepage 20002 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20003],
  AC_HELP_STRING([--with-cp20003],[codepage 20003]),
  AC_DEFINE([EMBED_CP20003_SUPPORT],[1],[Define to 1 if you want codepage 20003 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20004],
  AC_HELP_STRING([--with-cp20004],[codepage 20004]),
  AC_DEFINE([EMBED_CP20004_SUPPORT],[1],[Define to 1 if you want codepage 20004 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20005],
  AC_HELP_STRING([--with-cp20005],[codepage 20005]),
  AC_DEFINE([EMBED_CP20005_SUPPORT],[1],[Define to 1 if you want codepage 20005 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20105],
  AC_HELP_STRING([--with-cp20105],[codepage 20105]),
  AC_DEFINE([EMBED_CP20105_SUPPORT],[1],[Define to 1 if you want codepage 20105 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20106],
  AC_HELP_STRING([--with-cp20106],[codepage 20106]),
  AC_DEFINE([EMBED_CP20106_SUPPORT],[1],[Define to 1 if you want codepage 20106 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20107],
  AC_HELP_STRING([--with-cp20107],[codepage 20107]),
  AC_DEFINE([EMBED_CP20107_SUPPORT],[1],[Define to 1 if you want codepage 20107 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20108],
  AC_HELP_STRING([--with-cp20108],[codepage 20108]),
  AC_DEFINE([EMBED_CP20108_SUPPORT],[1],[Define to 1 if you want codepage 20108 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20127],
  AC_HELP_STRING([--with-cp20127],[codepage 20127]),
  AC_DEFINE([EMBED_CP20127_SUPPORT],[1],[Define to 1 if you want codepage 20127 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20261],
  AC_HELP_STRING([--with-cp20261],[codepage 20261]),
  AC_DEFINE([EMBED_CP20261_SUPPORT],[1],[Define to 1 if you want codepage 20261 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20269],
  AC_HELP_STRING([--with-cp20269],[codepage 20269]),
  AC_DEFINE([EMBED_CP20269_SUPPORT],[1],[Define to 1 if you want codepage 20269 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20273],
  AC_HELP_STRING([--with-cp20273],[codepage 20273]),
  AC_DEFINE([EMBED_CP20273_SUPPORT],[1],[Define to 1 if you want codepage 20273 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20277],
  AC_HELP_STRING([--with-cp20277],[codepage 20277]),
  AC_DEFINE([EMBED_CP20277_SUPPORT],[1],[Define to 1 if you want codepage 20277 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20278],
  AC_HELP_STRING([--with-cp20278],[codepage 20278]),
  AC_DEFINE([EMBED_CP20278_SUPPORT],[1],[Define to 1 if you want codepage 20278 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20280],
  AC_HELP_STRING([--with-cp20280],[codepage 20280]),
  AC_DEFINE([EMBED_CP20280_SUPPORT],[1],[Define to 1 if you want codepage 20280 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20284],
  AC_HELP_STRING([--with-cp20284],[codepage 20284]),
  AC_DEFINE([EMBED_CP20284_SUPPORT],[1],[Define to 1 if you want codepage 20284 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20285],
  AC_HELP_STRING([--with-cp20285],[codepage 20285]),
  AC_DEFINE([EMBED_CP20285_SUPPORT],[1],[Define to 1 if you want codepage 20285 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20290],
  AC_HELP_STRING([--with-cp20290],[codepage 20290]),
  AC_DEFINE([EMBED_CP20290_SUPPORT],[1],[Define to 1 if you want codepage 20290 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20297],
  AC_HELP_STRING([--with-cp20297],[codepage 20297]),
  AC_DEFINE([EMBED_CP20297_SUPPORT],[1],[Define to 1 if you want codepage 20297 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20420],
  AC_HELP_STRING([--with-cp20420],[codepage 20420]),
  AC_DEFINE([EMBED_CP20420_SUPPORT],[1],[Define to 1 if you want codepage 20420 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20423],
  AC_HELP_STRING([--with-cp20423],[codepage 20423]),
  AC_DEFINE([EMBED_CP20423_SUPPORT],[1],[Define to 1 if you want codepage 20423 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20424],
  AC_HELP_STRING([--with-cp20424],[codepage 20424]),
  AC_DEFINE([EMBED_CP20424_SUPPORT],[1],[Define to 1 if you want codepage 20424 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20833],
  AC_HELP_STRING([--with-cp20833],[codepage 20833]),
  AC_DEFINE([EMBED_CP20833_SUPPORT],[1],[Define to 1 if you want codepage 20833 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20838],
  AC_HELP_STRING([--with-cp20838],[codepage 20838]),
  AC_DEFINE([EMBED_CP20838_SUPPORT],[1],[Define to 1 if you want codepage 20838 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20866],
  AC_HELP_STRING([--with-cp20866],[Russian KOI8]),
  AC_DEFINE([EMBED_CP20866_SUPPORT],[1],[Define to 1 if you want Russian KOI8 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20871],
  AC_HELP_STRING([--with-cp20871],[codepage 20871]),
  AC_DEFINE([EMBED_CP20871_SUPPORT],[1],[Define to 1 if you want codepage 20871 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20880],
  AC_HELP_STRING([--with-cp20880],[codepage 20880]),
  AC_DEFINE([EMBED_CP20880_SUPPORT],[1],[Define to 1 if you want codepage 20880 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20905],
  AC_HELP_STRING([--with-cp20905],[codepage 20905]),
  AC_DEFINE([EMBED_CP20905_SUPPORT],[1],[Define to 1 if you want codepage 20905 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20924],
  AC_HELP_STRING([--with-cp20924],[codepage 20924]),
  AC_DEFINE([EMBED_CP20924_SUPPORT],[1],[Define to 1 if you want codepage 20924 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20932],
  AC_HELP_STRING([--with-cp20932],[EUC-JP]),
  AC_DEFINE([EMBED_CP20932_SUPPORT],[1],[Define to 1 if you want EUC-JP support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20936],
  AC_HELP_STRING([--with-cp20936],[codepage 20936]),
  AC_DEFINE([EMBED_CP20936_SUPPORT],[1],[Define to 1 if you want codepage 20936 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp20949],
  AC_HELP_STRING([--with-cp20949],[codepage 20949]),
  AC_DEFINE([EMBED_CP20949_SUPPORT],[1],[Define to 1 if you want codepage 20949 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp21025],
  AC_HELP_STRING([--with-cp21025],[codepage 21025]),
  AC_DEFINE([EMBED_CP21025_SUPPORT],[1],[Define to 1 if you want codepage 21025 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp21027],
  AC_HELP_STRING([--with-cp21027],[codepage 21027]),
  AC_DEFINE([EMBED_CP21027_SUPPORT],[1],[Define to 1 if you want codepage 21027 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp21866],
  AC_HELP_STRING([--with-cp21866],[Ukrainian KOI8]),
  AC_DEFINE([EMBED_CP21866_SUPPORT],[1],[Define to 1 if you want Ukrainian KOI8 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp28591],
  AC_HELP_STRING([--with-cp28591],[ISO 8859-1 Latin 1]),
  AC_DEFINE([EMBED_CP28591_SUPPORT],[1],[Define to 1 if you want ISO 8859-1 Latin 1 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp28592],
  AC_HELP_STRING([--with-cp28592],[ISO 8859-2 Latin 2 'East European']),
  AC_DEFINE([EMBED_CP28592_SUPPORT],[1],[Define to 1 if you want ISO 8859-2 Latin 2 'East European' support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp28593],
  AC_HELP_STRING([--with-cp28593],[ISO 8859-3 Latin 3 'South European']),
  AC_DEFINE([EMBED_CP28593_SUPPORT],[1],[Define to 1 if you want ISO 8859-3 Latin 3 'South European' support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp28594],
  AC_HELP_STRING([--with-cp28594],[ISO 8859-4 Latin 4 'Baltic old']),
  AC_DEFINE([EMBED_CP28594_SUPPORT],[1],[Define to 1 if you want ISO 8859-4 Latin 4 'Baltic old' support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp28595],
  AC_HELP_STRING([--with-cp28595],[ISO 8859-5 Cyrillic]),
  AC_DEFINE([EMBED_CP28595_SUPPORT],[1],[Define to 1 if you want ISO 8859-5 Cyrillic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp28596],
  AC_HELP_STRING([--with-cp28596],[ISO 8859-6 Arabic]),
  AC_DEFINE([EMBED_CP28596_SUPPORT],[1],[Define to 1 if you want ISO 8859-6 Arabic support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp28597],
  AC_HELP_STRING([--with-cp28597],[ISO 8859-7 Greek]),
  AC_DEFINE([EMBED_CP28597_SUPPORT],[1],[Define to 1 if you want ISO 8859-7 Greek support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp28598],
  AC_HELP_STRING([--with-cp28598],[ISO 8859-8 Hebrew]),
  AC_DEFINE([EMBED_CP28598_SUPPORT],[1],[Define to 1 if you want ISO 8859-8 Hebrew support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp28599],
  AC_HELP_STRING([--with-cp28599],[ISO 8859-9 Latin 5 'Turkish']),
  AC_DEFINE([EMBED_CP28599_SUPPORT],[1],[Define to 1 if you want ISO 8859-9 Latin 5 'Turkish' support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp28605],
  AC_HELP_STRING([--with-cp28605],[ISO 8859-15 Latin 9 'Euro']),
  AC_DEFINE([EMBED_CP28605_SUPPORT],[1],[Define to 1 if you want ISO 8859-15 Latin 9 'Euro' support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp38598],
  AC_HELP_STRING([--with-cp38598],[codepage 38598]),
  AC_DEFINE([EMBED_CP38598_SUPPORT],[1],[Define to 1 if you want codepage 38598 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp50220],
  AC_HELP_STRING([--with-cp50220],[codepage 50220]),
  AC_DEFINE([EMBED_CP50220_SUPPORT],[1],[Define to 1 if you want codepage 50220 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp50221],
  AC_HELP_STRING([--with-cp50221],[codepage 50221]),
  AC_DEFINE([EMBED_CP50221_SUPPORT],[1],[Define to 1 if you want codepage 50221 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp50222],
  AC_HELP_STRING([--with-cp50222],[codepage 50222]),
  AC_DEFINE([EMBED_CP50222_SUPPORT],[1],[Define to 1 if you want codepage 50222 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp50225],
  AC_HELP_STRING([--with-cp50225],[codepage 50225]),
  AC_DEFINE([EMBED_CP50225_SUPPORT],[1],[Define to 1 if you want codepage 50225 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp50227],
  AC_HELP_STRING([--with-cp50227],[codepage 50227]),
  AC_DEFINE([EMBED_CP50227_SUPPORT],[1],[Define to 1 if you want codepage 50227 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp50229],
  AC_HELP_STRING([--with-cp50229],[codepage 50229]),
  AC_DEFINE([EMBED_CP50229_SUPPORT],[1],[Define to 1 if you want codepage 50229 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp51949],
  AC_HELP_STRING([--with-cp51949],[codepage 51949]),
  AC_DEFINE([EMBED_CP51949_SUPPORT],[1],[Define to 1 if you want codepage 51949 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp52936],
  AC_HELP_STRING([--with-cp52936],[codepage 52936]),
  AC_DEFINE([EMBED_CP52936_SUPPORT],[1],[Define to 1 if you want codepage 52936 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp57002],
  AC_HELP_STRING([--with-cp57002],[codepage 57002]),
  AC_DEFINE([EMBED_CP57002_SUPPORT],[1],[Define to 1 if you want codepage 57002 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp57003],
  AC_HELP_STRING([--with-cp57003],[codepage 57003]),
  AC_DEFINE([EMBED_CP57003_SUPPORT],[1],[Define to 1 if you want codepage 57003 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp57004],
  AC_HELP_STRING([--with-cp57004],[codepage 57004]),
  AC_DEFINE([EMBED_CP57004_SUPPORT],[1],[Define to 1 if you want codepage 57004 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp57005],
  AC_HELP_STRING([--with-cp57005],[codepage 57005]),
  AC_DEFINE([EMBED_CP57005_SUPPORT],[1],[Define to 1 if you want codepage 57005 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp57006],
  AC_HELP_STRING([--with-cp57006],[codepage 57006]),
  AC_DEFINE([EMBED_CP57006_SUPPORT],[1],[Define to 1 if you want codepage 57006 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp57007],
  AC_HELP_STRING([--with-cp57007],[codepage 57007]),
  AC_DEFINE([EMBED_CP57007_SUPPORT],[1],[Define to 1 if you want codepage 57007 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp57008],
  AC_HELP_STRING([--with-cp57008],[codepage 57008]),
  AC_DEFINE([EMBED_CP57008_SUPPORT],[1],[Define to 1 if you want codepage 57008 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp57009],
  AC_HELP_STRING([--with-cp57009],[codepage 57009]),
  AC_DEFINE([EMBED_CP57009_SUPPORT],[1],[Define to 1 if you want codepage 57009 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp57010],
  AC_HELP_STRING([--with-cp57010],[codepage 57010]),
  AC_DEFINE([EMBED_CP57010_SUPPORT],[1],[Define to 1 if you want codepage 57010 support.]),
  [eac=no]
)

AC_ARG_WITH(
  [cp57011],
  AC_HELP_STRING([--with-cp57011],[codepage 57011]),
  AC_DEFINE([EMBED_CP57011_SUPPORT],[1],[Define to 1 if you want codepage 57011 support.]),
  [eac=no]
)

if test "$eac" = yes; then
  AC_DEFINE([EMBED_ALL_CP],[1],[Define to 1 if you want Define to 1 if you want all supported codepages function.])
fi
