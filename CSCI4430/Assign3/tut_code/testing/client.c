

#include <string.h>
#include <netinet/in.h> // "struct sockaddr_in"
#include <sys/socket.h>

int main(int argc, char *argv[]) {

  char* ip = argv[1];

  // create an IPv4/UDP socket
  int fd = socket(AF_INET, SOCK_DGRAM, 0);

  // initialize the address of server
  struct sockaddr_in destination;
  memset(&destination, 0, sizeof(struct sockaddr_in));
  destination.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &(destination.sin_addr));
  destination.sin_port = htons(12345);

  int num=0;
  while (1) {
    int tmpnum=htonl(num);
    int len = sizeof(tmpnum);
    printf("send %d\n", num);
    sendto(fd, (char*)&tmpnum, len, 0, (struct sockaddr *)&destination, sizeof(destination));
    num++;
    sleep(1);
  }

  return 0;
 
}
