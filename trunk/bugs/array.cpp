#include <windows.h>
#include <stdio.h>

#include "ibase.h"

#define DbName "localhost:c:\\macroscope.fdb"

char DPB[] = {
  isc_dpb_version1,
  isc_dpb_user_name, 6, 's', 'y', 's', 'd', 'b', 'a',
  isc_dpb_password, 9, 'm', 'a', 's', 't', 'e', 'r', 'k', 'e', 'y',
  isc_dpb_lc_ctype, 11, 'U', 'N', 'I', 'C', 'O', 'D', 'E', '_', 'F', 'S', 'S'
};

char TPB[] = {
  isc_tpb_version3,
  isc_tpb_read_committed,
  isc_tpb_rec_version,
  isc_tpb_nowait
};

void printStatus(ISC_STATUS_ARRAY Status)
{
  if( Status[1] != 0 ){
    isc_print_status(Status);
    exit(Status[1]);
  }
}

void printSqlError(ISC_STATUS_ARRAY Status)
{
  if( Status[1] != 0 ){
    isc_print_sqlerror(isc_sqlcode(Status),Status);
    exit(Status[1]);
  }
}

class ThreadTester {
  private:
    static DWORD WINAPI ThreadFunc(LPVOID AThread);
    HANDLE Handle_;
    DWORD Id_;
    DWORD ExitCode_;
    ISC_STATUS_ARRAY Status_;
    isc_db_handle DbHandle_;
    isc_tr_handle TrHandle_;
    isc_stmt_handle StHandle_;

    void printStatus(ISC_STATUS_ARRAY Status,long Line = 0);
    void printSqlError(ISC_STATUS_ARRAY Status);
    void execute();
  public:
    ThreadTester();
    ~ThreadTester();

    bool Active() const;
};

void ThreadTester::printStatus(ISC_STATUS_ARRAY Status,long Line)
{
  if( Status[1] != 0 ){
    isc_print_status(Status);
    if( Line > 0 ) printf("In line %d\n",Line);
    isc_dsql_free_statement(Status_,&StHandle_,DSQL_drop);
    isc_rollback_transaction(Status_,&TrHandle_);
    isc_detach_database(Status_,&DbHandle_);
    ExitCode_ = Status_[1];
    ExitThread(Status[1]);
  }
}

void ThreadTester::printSqlError(ISC_STATUS_ARRAY Status)
{
  if( Status[1] != 0 ){
    isc_print_sqlerror(isc_sqlcode(Status),Status);
    isc_dsql_free_statement(Status_,&StHandle_,DSQL_drop);
    isc_rollback_transaction(Status_,&TrHandle_);
    isc_detach_database(Status_,&DbHandle_);
    ExitThread(Status[1]);
  }
}

bool ThreadTester::Active() const
{
  DWORD AExitCode;
  return GetExitCodeThread(Handle_,&AExitCode) != 0 && AExitCode == STILL_ACTIVE;
}  

DWORD WINAPI ThreadTester::ThreadFunc(LPVOID AThread)
{
  reinterpret_cast<ThreadTester *>(AThread)->execute();
  return reinterpret_cast<ThreadTester *>(AThread)->ExitCode_;
}

ThreadTester::ThreadTester() :
  Handle_(INVALID_HANDLE_VALUE),
  ExitCode_(0), DbHandle_(0), TrHandle_(0), StHandle_(0)
{
  Handle_ = CreateThread(NULL,0,ThreadFunc,this,CREATE_SUSPENDED,&Id_);
  if( Handle_ == INVALID_HANDLE_VALUE ) exit(GetLastError());
  ResumeThread(Handle_);
  ExitCode_ = STILL_ACTIVE;
}

ThreadTester::~ThreadTester()
{
  SetThreadPriority(Handle_,THREAD_PRIORITY_HIGHEST);
  ResumeThread(Handle_);
  while( Active() ) if( SwitchToThread() == 0 ) Sleep(1);
}

void ThreadTester::execute()
{
  isc_attach_database(Status_,0,DbName,&DbHandle_,sizeof(DPB),DPB);
  printStatus(Status_,__LINE__);

  isc_start_transaction(Status_,&TrHandle_,1,&DbHandle_,sizeof(TPB),TPB);
  printStatus(Status_,__LINE__);

  isc_dsql_allocate_statement(Status_,&DbHandle_,&StHandle_);
  printStatus(Status_,__LINE__);

  static char InsertString[] = 
    "INSERT INTO table1 (dep_id,dep_array) VALUES (GEN_ID(TABLE1_GEN,1),?)"
  ;

  isc_dsql_prepare(Status_,&TrHandle_,&StHandle_,0,InsertString,3,NULL);
  printStatus(Status_,__LINE__);

  ISC_QUAD ArrayId;
  ISC_ARRAY_DESC ArrayDesc;
  short Flag0 = 0;

  char BufSqlda[XSQLDA_LENGTH(1)];
  XSQLDA & Sqlda = *(XSQLDA *) BufSqlda;

  Sqlda.sqln = 1;
  Sqlda.version = SQLDA_VERSION1;
  
  isc_dsql_describe_bind(Status_,&StHandle_,3,&Sqlda);
  printStatus(Status_,__LINE__);

  isc_array_lookup_bounds(Status_,
    &DbHandle_,&TrHandle_,
    Sqlda.sqlvar[0].relname,Sqlda.sqlvar[0].sqlname,
    &ArrayDesc
  );
  printStatus(Status_,__LINE__);

  Sqlda.sqlvar[0].sqldata = (char *) &ArrayId;
  Sqlda.sqlvar[0].sqltype = SQL_ARRAY + 1;
  Sqlda.sqlvar[0].sqlind  = &Flag0;

  for( long i = 1000; i > 0; i-- ){
    ArrayId.isc_quad_low = 0;
    ArrayId.isc_quad_high = 0;

    __int64 Data[6] = {
      10000000000,
      10000000001,
      10000000010,
      10000000011,
      10000000020,
      10000000021
    };

    ISC_LONG SizeOfData = sizeof(Data);
    isc_array_put_slice(Status_,
      &DbHandle_,&TrHandle_,
      &ArrayId,&ArrayDesc,
      Data,&SizeOfData
    );
    printStatus(Status_,__LINE__);

    isc_dsql_execute(Status_,&TrHandle_,&StHandle_,3,&Sqlda);
    printStatus(Status_,__LINE__);
  }

  isc_dsql_free_statement(Status_,&StHandle_,DSQL_drop);
  printStatus(Status_,__LINE__);

  isc_commit_transaction(Status_,&TrHandle_);
  printStatus(Status_,__LINE__);

  isc_detach_database(Status_,&DbHandle_);
  printStatus(Status_,__LINE__);

  ExitCode_ = 0;
}

class Tester {
  private:
    ISC_STATUS_ARRAY Status_;
    isc_db_handle DbHandle_;
    isc_tr_handle TrHandle_;
  public:
    Tester();
    ~Tester();
};

Tester::Tester() : DbHandle_(0), TrHandle_(0)
{
  static char CreateDbString[] = 
    "CREATE DATABASE '" DbName "'"
    " USER 'sysdba' PASSWORD 'masterkey' PAGE_SIZE=1024"
    " DEFAULT CHARACTER SET UNICODE_FSS"
  ;

  if( isc_dsql_execute_immediate(Status_,&DbHandle_,&TrHandle_,0,CreateDbString,3,NULL) != 0 )
    if( isc_sqlcode(Status_) != -902 ) printSqlError(Status_);

  isc_detach_database(Status_,&DbHandle_);

  isc_attach_database(Status_,0,DbName,&DbHandle_,sizeof(DPB),DPB);
  printStatus(Status_);

  isc_start_transaction(Status_,&TrHandle_,1,&DbHandle_,sizeof(TPB),TPB);
  printStatus(Status_);

  static char CreateTableString[] = 
    "RECREATE TABLE table1 ("
    " dep_id        INTEGER NOT NULL PRIMARY KEY,"
    " dep_array     DECIMAL(18,0) [0:2,0:1]"
    ")"
  ;

  isc_dsql_execute_immediate(Status_,&DbHandle_,&TrHandle_,0,CreateTableString,3,NULL);
  printStatus(Status_);

  static char DropGenString[] = "DROP GENERATOR TABLE1_GEN";

  if( isc_dsql_execute_immediate(Status_,&DbHandle_,&TrHandle_,0,DropGenString,3,NULL) != 0 )
    if( Status_[1] != isc_no_meta_update ) printStatus(Status_);

  static char CreateGenString[] = "CREATE GENERATOR TABLE1_GEN";

  isc_dsql_execute_immediate(Status_,&DbHandle_,&TrHandle_,0,CreateGenString,3,NULL);
  printStatus(Status_);

  isc_commit_transaction(Status_,&TrHandle_);
  printStatus(Status_);

  isc_start_transaction(Status_,&TrHandle_,1,&DbHandle_,sizeof(TPB),TPB);
  printStatus(Status_);

  for( long i = 10; i > 0; i-- ){
    printf("cycle = %ld\n",i);
    ThreadTester Tester[10];
  }

  isc_commit_transaction(Status_,&TrHandle_);
  printStatus(Status_);
}

Tester::~Tester()
{
  isc_detach_database(Status_,&DbHandle_);
}

int main(int /*argc*/,char ** /*argv*/)
{
  Tester Tester;
  return 0;
}
