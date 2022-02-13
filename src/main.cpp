#include <cstdio>
#include <iostream>
#include <unistd.h>
#include "Server.h"
#include "Util.h"

int main(int argc, char **argv) {
  int threadNumber = 4;
  int port = 8080;
  int opt;
  const char *str = "t:p:";

  while ((opt = getopt(argc, argv, str)) != -1) {
    switch (opt) {
      case 't': {
        threadNumber = atoi(optarg);
        break;
      }
      case 'p': {
        port = atoi(optarg);
        break;
      }
      default: break;
    }
  }

  handle_for_sigpipe();

  printf("---------ExeServer Config---------\nPort: %d\nThreadNumber:%d\n----------------------------------\n", port, threadNumber);

  ExeServer exeServer(port, "127.0.0.1");
  exeServer.run(threadNumber, 10000);
}
