/*
 * Adonis project / software utilities:
 *  Http put mini lib
 *  written by L. Demailly
 *  (c) 1996 Observatoire de Paris - Meudon - France
 *
 * $Id$ 
 *
 * Description : Use http protocol, connects to server to send a packet
 *               
 *
 * $Log$
 *
 */

static char *rcsid="$Id$";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define SERVER "hplyot"
#define PORT    5757

/* prototype */
int http_put(char *filename, char *data, int length, char *type) ;

int http_put(filename, data, length, type) 
     char *filename,*data,*type;
     int length;
{
  int     s;
  struct  hostent *hp;
  struct  sockaddr_in     server;
  char header[512];
  int hlg,ret;
  
/* get host info by name :*/
  if ((hp = gethostbyname(SERVER))) {
    memset((char *) &server,0, sizeof(server));
    memcpy(hp->h_addr, (char *) &server.sin_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
  } else {
    return (-1);
  }
  server.sin_port = (unsigned short) htons(PORT);
/* create socket */
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) return (-2);
  setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
/* connect to server */
  if (connect(s, &server, sizeof(server)) < 0) {
    close(s);
    return (-3);
  }
/* create header */
  if (!type) type="binary/octet-stream";
  sprintf(header,"PUT /%s HTTP/1.0\nContent-length: %d\nContent-type: %s\n\n",
	  filename,length,type);
  hlg=strlen(header);
/* send header */
  if (write(s,header,hlg)!=hlg) {
    close(s);
    return (-4);
  }
/* send data */
  if (write(s,data,length)!=length) {
    close(s);
    return (-5);
  }
/* check return */
  if (read(s,header,12)!=12) {
    close(s);
    return (-6);
  }
/* close socket */
  close(s);
  header[13]=0;
  if (sscanf(header,"HTTP/1.0 %03d",&ret)!=1) {
    return (-7);
  }
  return ret;
}

      
  



