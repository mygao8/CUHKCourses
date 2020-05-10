
#include <stdio.h>
#include <string.h>
#include <netinet/in.h> // "struct sockaddr_in"
#include <sys/socket.h> // "socket"

int main(int argc, char *argv[]) {

  // create a IPv4/UDP socket
  int fd = socket(AF_INET, SOCK_DGRAM, 0);

  // initialize the address
  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(12345);
  address.sin_addr.s_addr = htonl(INADDR_ANY); 

  // Bind the socket to the address
  bind(fd, (struct sockaddr *)&address, sizeof(struct sockaddr));

  printf("Server serve at port 12345\n");
  while (1) {
    // Receive the message (we don't care about the address of the sender)
    int tmpnum;
    int len = recvfrom(fd, (char*)&tmpnum, sizeof(tmpnum), 0, NULL, NULL) ;
    int num = ntohl(tmpnum);
    printf("received %d\n", num);
  }


  return 0;
}
