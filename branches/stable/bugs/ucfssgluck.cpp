#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ibase.h"

using namespace std;

struct ISC_TEB {
    isc_db_handle * db_ptr;
    long            tpb_len;
    char *          tpb_ptr;
};

void printStatus(int code, ISC_STATUS_ARRAY status, int line)
{
  if( code != 0 ){
    isc_print_status(status);
    isc_print_sqlerror(isc_sqlcode(status), status);
    printf("Error in line %d\n", line);
  }
}

int main(int, char **)
{
  isc_db_handle     newdb           = 0;
  isc_tr_handle     trans           = 0;
  char              createSQLText[] = "CREATE DATABASE 'MACROSCOPE.FDB' USER 'sysdba' PASSWORD 'masterkey' PAGE_SIZE=1024 " "DEFAULT CHARACTER SET UNICODE_FSS"
  ;
  ISC_STATUS_ARRAY  status;

  printStatus(isc_dsql_execute_immediate(status, &newdb, &trans, 0, createSQLText, 3, NULL), status, __LINE__);
  printStatus(isc_detach_database(status, &newdb), status, __LINE__);

  char  dpb[34];
  dpb[0] = isc_dpb_version1;
  dpb[1] = isc_dpb_user_name;
  dpb[2] = (char) strlen("sysdba");
  strcpy(dpb + 3, "sysdba");
  dpb[9] = isc_dpb_password;
  dpb[10] = (char) strlen("masterkey");
  strcpy(dpb + 11, "masterkey");
  dpb[20] = isc_dpb_lc_ctype;
  dpb[21] = (char) strlen("UNICODE_FSS");
  strcpy(dpb + 22, "UNICODE_FSS");

  printStatus(isc_attach_database(status, strlen("MACROSCOPE.FDB"), "MACROSCOPE.FDB", &newdb, 33, dpb), status, __LINE__);

  char    tpb[] = {
    isc_tpb_version3, isc_tpb_read_committed, isc_tpb_rec_version, isc_tpb_nowait
  };
  ISC_TEB teb;
  teb.db_ptr = &newdb;
  teb.tpb_len = sizeof(tpb);
  teb.tpb_ptr = tpb;

  printStatus(isc_start_multiple(status, &trans, 1, &teb), status, __LINE__);

  isc_stmt_handle stmt  = 0;

  printStatus(isc_dsql_allocate_statement(status, &newdb, &stmt), status, __LINE__);

  char  createTableSQLText[]  = "create table table1 (" "  dep_id       INTEGER not null," "  dep_name     CHAR(20)," "  dep_shname   CHAR(20)," "  dep_address  CHAR(255)," "  primary key  (dep_id)" ");"
  ;

  printStatus(isc_dsql_prepare(status, &trans, &stmt, 0, createTableSQLText, 3, NULL), status, __LINE__);

  printStatus(isc_dsql_execute2(status, &trans, &stmt, 3, NULL, NULL), status, __LINE__);

  printStatus(isc_commit_transaction(status, &trans), status, __LINE__);

  printStatus(isc_start_multiple(status, &trans, 1, &teb), status, __LINE__);

  char  gluckedStmpText[] = "insert into table1 (" "  dep_id," "  dep_name," "  dep_shname," "  dep_address" ") values (" "-1," "'name1'," "'shname1'," "'address1\xD0\xB9\xD1\x86\xD1\x83\xD0\xBA\xD0\xB5\xD0\xBD'" // "йцукен" in UTF-8
  ");"
  ;

  printStatus(isc_dsql_prepare(status, &trans, &stmt, 0, gluckedStmpText, 3, NULL), status, __LINE__);

  printStatus(isc_dsql_execute2(status, &trans, &stmt, 3, NULL, NULL), status, __LINE__);

  printStatus(isc_commit_transaction(status, &trans), status, __LINE__);
  return 0;
}