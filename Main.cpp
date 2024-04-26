
#include "flv/FlvSrv.h"
#include <iostream>
#include <thread>

void doSrvFlvServe(size_t port) {
  std::cout << "Hello SrvFlvServe " << port << "\n";
  asio::io_context ioContext;
  FlvServer server(ioContext, port);
  ioContext.run();
}

#include "TcpPoolServer.h"
void doSrvPool(size_t port, size_t thNum) {
  std::cout << "Hello SrvTcpServe " << port << "\n";
  auto endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
  TcpPoolServer server(endpoint, thNum);
  server.run();
}

void usage() {
  std::cerr << "Usage: msnet -h\n";
  std::cerr << "Usage: msnet -c conf/msnet.xml -flv port<default:10600>\n";
  exit(-1);
}

#include "NginxStat.h"
void doTimerNginxStat() {
  doNginxStatAccess(GetXmlConfig(), "POST", "/stat", "");
}

int main(int argc, char const* argv[]) {
  const char* conf = "conf/msnet.xml";
  int flvPort = 10600;
  // if (argc < 3) {
  //     usage();
  // }

  // for (int argind = 1; argind < argc; argind++) {
  //     const char* argp = argv[argind];
  //     if (*argp != '-') {
  //         break;
  //     } else if (strcmp(argp, "-h") == 0) {
  //         usage();
  //         break;
  //     } else if (strcmp(argp, "-c") == 0) {
  //         if (argind >= argc) {
  //             usage();
  //         }
  //         conf = argv[++argind];
  //     } else if (strcmp(argp, "-flv") == 0) {
  //         if (argind >= argc) {
  //             usage();
  //         }
  //         flvPort = atoi(argv[++argind]);
  //     }
  // }
  try {
    std::thread th(doSrvFlvServe, flvPort);
    // th.join();
    // CommTimer doTimerNginx;
    if (!XmlConfigInit(conf)) {
      std::cerr << conf << "error\n";
      exit(-1);
    }
    // doTimerNginx.SetInterval(doTimerNginxStat, 50 * 1000);
    doSrvPool(GetXmlConfig()->server.port, 4);
  } catch (std::exception* e) {
  }
  // doTimerNginx.Stop();
  return 0;
}
