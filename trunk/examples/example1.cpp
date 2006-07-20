/*-
 * Copyright 2005 Guram Dukashvili
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
//---------------------------------------------------------------------------
#include <adicpp/adicpp.h>
//------------------------------------------------------------------------------
using namespace ksys;
using namespace utf8;
using namespace adicpp;
//------------------------------------------------------------------------------
class ThreadTester : public Thread {
  protected:
    void  execute();
  public:
          ThreadTester();
};
//------------------------------------------------------------------------------
ThreadTester::ThreadTester()
{
}
//------------------------------------------------------------------------------
void ThreadTester::execute()
{
  try{
    AutoPtr< Randomizer>  rnd (newObject<Randomizer>);
    rnd->randomize();

    Config  config;
    config.parse().override();

    AutoPtr< Database>  database  (Database::newDatabase(&config));

    database->name("sample")->
    addParam("user_name", "root")->
    addParam("password", "");

    database->create();

    bool  cont;
    for( cont = true; cont; ){
      try{
        database->attach();
        cont = false;
      }
      catch( ExceptionSP & e ){
        e->writeStdError();
      }
    }

    AutoPtr< Statement> statement (database->newAttachedStatement());

    for( cont = true; cont; ){
      try{
        statement->execute("CREATE TABLE TABLE_EXAMPLE1 (" "fl_id           INTEGER NOT NULL AUTO_INCREMENT," "fl_threadid     INTEGER," "fl_char         CHAR(20)," "fl_varchar      VARCHAR(20)," "fl_timestamp    TIMESTAMP," "fl_bigint       DECIMAL(18,0)," "fl_blob         LONGBLOB," "PRIMARY KEY (fl_id)" ")");
        cont = false;
      }
      catch( ExceptionSP & e ){
        if( !e->searchCode(isc_no_meta_update, isc_random, ER_TABLE_EXISTS_ERROR, ER_DUP_KEYNAME) )
          throw;
        else
          cont = false;
      }
    }

    statement->text("insert into TABLE_EXAMPLE1 (" " fl_threadid," " fl_char," " fl_varchar," " fl_timestamp," " fl_bigint," " fl_blob" ") values (" " :fl_threadid," " :fl_char," " :fl_varchar," " :fl_timestamp," " :fl_bigint," " :fl_blob" ")");

    database->start();
    for( uintptr_t i = 0; i < 100; i++ ){
      String  s;
      for( uintptr_t j = rnd->random(20); j > 0; j-- ){
        uintptr_t c;
        while( (getC1Type(c = rnd->random2(0xFFFF)) & C1_CNTRL) != 0 );
        char  b[7];
        s += plane(b, ucs2utf8seq(b, c));
      }
      statement->prepare();
      statement->paramAsMutant("fl_char", s);
      statement->paramAsMutant("fl_varchar", s.upper());
      statement->paramAsMutant("fl_threadid", id());
      statement->paramAsMutant("fl_timestamp", time2Timeval(gettimeofday()));
      statement->paramAsMutant("fl_bigint", rnd->random2(15));
      MemoryStream  blob;
      blob.resize(rnd->random2(65535) + 1);
      statement->paramAsMutant("fl_blob", blob);
      statement->execute();
    }
    database->commit();

    exitCode_ = 0;
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    exitCode_ = e->code();
  }
}
//------------------------------------------------------------------------------
int main(int, char **)
{
  /*  {
      SP<SPTest> sp(newObject<SPTest>);
      sp->method();
    }*/
  /*  
  fbcpp::Database Database2;    
  Database2.Params().add("user_name","sysdba");    
  Database2.Params().add("password","masterkey");    
  Database2.Params().add("sql_dialect",3);    
  Database2.attach(DATABASE_NAME);    
  fbcpp::Transaction Transaction2;    
  Transaction2.attach(Database2);    
  Transaction2.ParamsByDatabase(Database2).
  add("version3").
  add("read_committed").
  add("no_rec_version").
  add("nowait");    
  Transaction2.start();    
  EventHandler EventHandler;    
  EventHandler.attach(Database2);    
  EventHandler.add("inet_stat_ins");    
  EventHandler.add("inet_stat_upd");    
  EventHandler.add("inet_stat_del");    
  EventHandler.queue();*/

  /*  Randomizer rnd;
    rnd.randomize();
    long cycle, i;
    longlong time = gettimeofday();
    for( cycle = 0; cycle < 100; cycle++ ){
      adicpp::initialize();
      printf("run test cycle %3d\n",cycle);
      try {
        Vector<ThreadTester> testers;
        testers.resize(40);
        for( i = testers.count() - 1; i >= 0; i-- ){
          testers[i].stackSize(8192);
          testers[i].resume();
        }
        testers.resize(0);
        Database database;
        database.name("sample").params().
          add("user_name","root").add("password","O5DdY.QC");
        database.create();
        database.attach();
        Transaction transaction;
        transaction.attach(database);
        DSQLStatement statement;
        statement.attach(database);
        try {
          statement.execute(
            "CREATE TABLE TABLE_EXAMPLE1 ("
            "fl_id           INTEGER NOT NULL AUTO_INCREMENT,"
            "fl_threadid     INTEGER,"
            "fl_char         CHAR(20),"
            "fl_varchar      VARCHAR(20),"
            "fl_timestamp    TIMESTAMP,"
            "fl_bigint       DECIMAL(18,0),"
            "fl_blob         LONGBLOB,"
            "PRIMARY KEY (fl_id)"
            ")"
          );
        }
        catch( EDSQLStExecute & e ){
          if( e.code() != ER_TABLE_EXISTS_ERROR ) throw;
        }
        statement.sqlText(
          "insert into TABLE_EXAMPLE1 ("
          " fl_threadid,"
          " fl_char,"
          " fl_varchar,"
          " fl_timestamp,"
          " fl_bigint,"
          " fl_blob"
          ") values ("
          " :fl_threadid,"
          " :fl_char,"
          " :fl_varchar,"
          " :fl_timestamp,"
          " :fl_bigint,"
          " :fl_blob"
          ")"
        );
        transaction.start();
        for( i = 0; i < 100; i++ ){
          String s;
          for( long j = rnd.random(20); j > 0; j-- ){
            long c;
            while( (getC1Type(c = rnd.random2(0xFFFF)) & C1_CNTRL) != 0 );
            char b[7];
            s += String::plane(b,ucs2utf8seq(b,c));
          }
          statement.prepare();
          statement.paramAsMutant("fl_char",s);
          statement.paramAsMutant("fl_varchar",s.upper());
          statement.paramAsMutant("fl_threadid",gettid());
          statement.paramAsMutant("fl_timestamp",time2Timeval(gettimeofday()));
          statement.paramAsMutant("fl_bigint",rnd.random2(15));
          MemoryStream blob;
          blob.resize(rnd.random2(65535) + 1);
          statement.paramAsMutant("fl_blob",blob);
          statement.execute();
        }
        transaction.commit();
        transaction.start();
        statement.execute("SELECT fl_bigint FROM TABLE_EXAMPLE1 GROUP BY fl_bigint");
        while( statement.values().fetch() ){
          statement.values().selectLast();
          printf(
            "fl_bigint = %s\n",
            String(statement.valueAsMutant("fl_bigint")).c_str()
          );
        }
        statement.execute("SELECT fl_threadid FROM TABLE_EXAMPLE1 GROUP BY fl_threadid");
        while( statement.values().fetch() ){
          statement.values().selectLast();
          printf(
            "fl_threadid = %s\n",
            String(statement.valueAsMutant("fl_threadid")).c_str()
          );
        }
        statement.execute("SELECT * FROM TABLE_EXAMPLE1");
        while( statement.values().fetch() ){
          statement.values().selectLast();
        }
        transaction.commit();
    
        database.drop();
      }
      catch( Exception & e ){
        stdErr.log(lmERROR,"%d %s\n",e.code(),e.what().getOEMString().ptr());
      }
      adicpp::cleanup();
    }
    time = gettimeofday() - time;
    printf(
      "test time: %d:%02d:%02d.%06d\n"
      "average test time: %d:%02d:%02d.%06d\n",
      int(time / 1000000 / 60 / 60),
      int(time / 1000000 / 60 % 60),
      int(time / 1000000 % 60),
      int(time % 1000000),
      int(time / cycle / 1000000 / 60 / 60),
      int(time / cycle / 1000000 / 60 % 60),
      int(time / cycle / 1000000 % 60),
      int(time / cycle % 1000000)
    );*/
  return 0;
}
//------------------------------------------------------------------------------
