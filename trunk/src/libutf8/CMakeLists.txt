ADD_LIBRARY(utf8 STATIC
	cp10000.cpp
	cp10001.cpp
	cp10002.cpp
	cp10003.cpp
	cp10004.cpp
	cp10005.cpp
	cp10006.cpp
	cp10007.cpp
	cp10008.cpp
	cp10010.cpp
	cp10017.cpp
	cp10021.cpp
	cp10029.cpp
	cp10079.cpp
	cp10081.cpp
	cp10082.cpp
	cp1026.cpp
	cp1047.cpp
	cp1140.cpp
	cp1141.cpp
	cp1142.cpp
	cp1143.cpp
	cp1144.cpp
	cp1145.cpp
	cp1146.cpp
	cp1147.cpp
	cp1148.cpp
	cp1149.cpp
	cp1250.cpp
	cp1251.cpp
	cp1252.cpp
	cp1253.cpp
	cp1254.cpp
	cp1255.cpp
	cp1256.cpp
	cp1257.cpp
	cp1258.cpp
	cp1361.cpp
	cp20000.cpp
	cp20001.cpp
	cp20002.cpp
	cp20003.cpp
	cp20004.cpp
	cp20005.cpp
	cp20105.cpp
	cp20106.cpp
	cp20107.cpp
	cp20108.cpp
	cp20127.cpp
	cp20261.cpp
	cp20269.cpp
	cp20273.cpp
	cp20277.cpp
	cp20278.cpp
	cp20280.cpp
	cp20284.cpp
	cp20285.cpp
	cp20290.cpp
	cp20297.cpp
	cp20420.cpp
	cp20423.cpp
	cp20424.cpp
	cp20833.cpp
	cp20838.cpp
	cp20866.cpp
	cp20871.cpp
	cp20880.cpp
	cp20905.cpp
	cp20924.cpp
	cp20932.cpp
	cp20936.cpp
	cp20949.cpp
	cp21025.cpp
	cp21027.cpp
	cp21866.cpp
	cp28591.cpp
	cp28592.cpp
	cp28593.cpp
	cp28594.cpp
	cp28595.cpp
	cp28596.cpp
	cp28597.cpp
	cp28598.cpp
	cp28599.cpp
	cp28605.cpp
	cp37.cpp
	cp38598.cpp
	cp437.cpp
	cp500.cpp
	cp50220.cpp
	cp50221.cpp
	cp50222.cpp
	cp50225.cpp
	cp50227.cpp
	cp50229.cpp
	cp51949.cpp
	cp52936.cpp
	cp57002.cpp
	cp57003.cpp
	cp57004.cpp
	cp57005.cpp
	cp57006.cpp
	cp57007.cpp
	cp57008.cpp
	cp57009.cpp
	cp57010.cpp
	cp57011.cpp
	cp54936.cpp
	cp28603.cpp
	cp708.cpp
	cp720.cpp
	cp737.cpp
	cp775.cpp
	cp850.cpp
	cp852.cpp
	cp855.cpp
	cp857.cpp
	cp858.cpp
	cp860.cpp
	cp861.cpp
	cp862.cpp
	cp863.cpp
	cp864.cpp
	cp865.cpp
	cp866.cpp
	cp869.cpp
	cp870.cpp
	cp874.cpp
	cp875.cpp
	cp932.cpp
	cp936.cpp
	cp949.cpp
	cp950.cpp
	utf8c1.cpp
	utf8c2.cpp
	utf8c3.cpp
	utf8cp.cpp
	utf8dtcp.cpp
	utf8func.cpp
	utf8lct.cpp
	utf8str.cpp
	utf8uct.cpp
)

SET (MAIN_SITE_LIST_IT 0)
LIST (LENGTH MAIN_SITE MAIN_SITE_LIST_LENGTH)
WHILE (MAIN_SITE_LIST_IT LESS MAIN_SITE_LIST_LENGTH)
  LIST (GET MAIN_SITE ${MAIN_SITE_LIST_IT} MAIN_SITE0)
  STRING (TOLOWER MAIN_SITE0 ${MAIN_SITE0})
  STRING (TOUPPER MAIN_SITE1 ${MAIN_SITE0})
  IF (SITE MATCHES "${MAIN_SITE0}" OR SITE MATCHES "${MAIN_SITE1}")
    ADD_DEFINITIONS(-DEMBED_CP1251_SUPPORT -DEMBED_CP866_SUPPORT -DEMBED_CP20866_SUPPORT)
    LIST (LENGTH MAIN_SITE MAIN_SITE_LIST_IT)
    SET (MAIN_SITE_ACCEPTED 1)
  ENDIF (SITE MATCHES "${MAIN_SITE0}" OR SITE MATCHES "${MAIN_SITE1}")
  MATH (EXPR MAIN_SITE_LIST_IT "${MAIN_SITE_LIST_IT} + 1")
ENDWHILE (MAIN_SITE_LIST_IT LESS MAIN_SITE_LIST_LENGTH)
		      
IF (NOT MAIN_SITE_ACCEPTED OR NOT PRIVATE_RELEASE)
  ADD_DEFINITIONS(-DEMBED_ALL_CP)
ENDIF (NOT MAIN_SITE_ACCEPTED OR NOT PRIVATE_RELEASE)
