#include <adicpp/adicpp.h>

class TestClient : public ksock::ClientFiber {
  public:
          ~TestClient();
          TestClient();
  protected:
    void  main();
  private:
};

TestClient::~TestClient()
{
}

TestClient::TestClient()
{
}

void TestClient::main()
{
  fprintf(stderr, "%s, %d\n", __PRETTY_FUNCTION__, __LINE__);
  throw ksys::Exception(44, __PRETTY_FUNCTION__);
}

class ClientShell : public ksock::Client {
  public:
          ~ClientShell();
          ClientShell();

    void  main();
  protected:
  private:
};

ClientShell::~ClientShell()
{
}

ClientShell::ClientShell()
{
}

void ClientShell::main()
{
  for( intptr_t i = 10; i > 0; i-- ){
    ksys::AutoPtr< TestClient> client(newObject<TestClient>);
    attachFiber(*client.ptr());
    client.ptr(NULL);
  }
}

int main(int argc, char ** argv)
{
  int errcode = -1;
  adicpp::initialize();
  try{
    ClientShell shell;
    shell.main();
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code();
  }
  catch( ... ){
  }
  adicpp::cleanup();
  return errcode;
}
