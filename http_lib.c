/*
 * Adonis project / software utilities:
 *  Http put mini lib
 *  written by L. Demailly
 *  (c) 1996 Observatoire de Paris - Meudon - France
 *
 * $Id: http_put.c,v 1.2 1996/04/16 10:24:19 dl Exp dl $ 
 *
 * Description : Use http protocol, connects to server to send a packet
 *               
 *
 * $Log: http_put.c,v $
 * Revision 1.2  1996/04/16  10:24:19  dl
 * server name and port as global variables instead of defines (changeable)
 * rewrote more compact
 *
 * Revision 1.1  1996/04/16  09:11:05  dl
 * Initial revision
 *
 *
 */

static char *rcsid="$Id: http_put.c,v 1.2 1996/04/16 10:24:19 dl Exp dl $";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char *http_server="hplyot";
int  http_port=5757;


/* prototype */
int http_put(char *filename, char *data, int length, 
	     int overwrite, char *type) ;

int http_put(filename, data, length, overwrite, type) 
     char *filename,*data,*type;
     int length,overwrite;
{
  int     s;
  struct  hostent *hp;
  struct  sockaddr_in     server;
  char header[512];
  int hlg,ret;
  
/* get host info by name :*/
  if ((hp = gethostbyname(http_server))) {
    memset((char *) &server,0, sizeof(server));
    memcpy(hp->h_addr, (char *) &server.sin_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
  } else {
    return (-1);
  }
  server.sin_port = (unsigned short) htons(http_port);
/* create socket */
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) return (-2);
  setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, 0, 0);
/* connect to server */
  if (connect(s, &server, sizeof(server)) < 0) {close(s); return (-3);}
/* create header */
  sprintf(header,"PUT /%s HTTP/1.0\nContent-length: %d\nContent-type: %s\n%s\n",
	  filename,length,type?type:"binary/octet-stream",
	  overwrite?"Control: overwrite=1\n":"");
  hlg=strlen(header);
/* send header */
  if (write(s,header,hlg)!=hlg) {close(s); return (-4);}
/* send data */
  if (write(s,data,length)!=length) {close(s); return (-5);}
/* check return */
  if (read(s,header,12)!=12) {close(s); return (-6);}
/* close socket */
  close(s);
  header[13]=0;
  if (sscanf(header,"HTTP/1.0 %03d",&ret)!=1) {return (-7);}
  return ret;
}

      
  



