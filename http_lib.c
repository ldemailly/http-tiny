/*
 * Adonis project / software utilities:
 *  Http put mini lib
 *  written by L. Demailly
 *  (c) 1996 Observatoire de Paris - Meudon - France
 *
 * $Id: http_put.c,v 1.4 1996/04/16 14:32:32 dl Exp dl $ 
 *
 * Description : Use http protocol, connects to server to send a packet
 *               
 *
 * $Log: http_put.c,v $
 * Revision 1.4  1996/04/16  14:32:32  dl
 * big error in memmove (mixed src dest 'cause of bcopy)
 * cleanup - allow sliced reads
 *
 * Revision 1.3  1996/04/16  12:13:44  dl
 * added overwrite optional parameter
 *
 * Revision 1.2  1996/04/16  10:24:19  dl
 * server name and port as global variables instead of defines (changeable)
 * rewrote more compact
 *
 * Revision 1.1  1996/04/16  09:11:05  dl
 * Initial revision
 *
 *
 */

static char *rcsid="$Id: http_put.c,v 1.4 1996/04/16 14:32:32 dl Exp dl $";


#ifdef OSK
/* OS/9 includes */
#include <modes.h>
#include <types.h>
#include <machine/reg.h>
#include <INET/socket.h>
#include <INET/in.h>
#include <INET/netdb.h>
#include <INET/pwd.h>
#else
/* unix */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>


/* prototype */
int http_put(char *filename, char *data, int length, 
	     int overwrite, char *type) ;

#endif /* OS9/Unix */

#include <stdio.h>

char *http_server="adonis";
int  http_port=5757;


int http_put(filename, data, length, overwrite, type) 
     char *filename,*data,*type;
     int length,overwrite;
{
  int     s;
  struct  hostent *hp;
  struct  sockaddr_in     server;
  char header[512];
  int  hlg,ret,remaining,off;
  
  /* get host info by name :*/
  if ((hp = gethostbyname(http_server))) {
    memset((char *) &server,0, sizeof(server));
    memmove((char *) &server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
    server.sin_port = (unsigned short) htons(http_port);
  } else
    return -1;

  /* create socket */
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -2;
  setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, 0, 0);

  /* connect to server */
  if (connect(s, &server, sizeof(server)) < 0) 
    ret=-3;
  else {

    /* create header */
    sprintf(header,
	"PUT /%s HTTP/1.0\012Content-length: %d\012Content-type: %s\012%s\012",
	    filename,
	    length,
	    type ? type : "binary/octet-stream",
	    overwrite ? "Control: overwrite=1\012" : ""
	    );
    hlg=strlen(header);

    /* send header */
    if (write(s,header,hlg)!=hlg)
      ret= -4;

    /* send data */
    else if (write(s,data,length)!=length) 
      ret= -5;

    else {
      /* read result & check */
      for (remaining=12, off=0; 
	   remaining && (ret=read(s,header+off,remaining)) != 0 ;
	   remaining -= ret, off+= ret
	   ) /* nothing more to do (everything in the for) */
#ifdef DEBUG
	printf("read %d bytes\n",ret);
#endif	
        ;
      header[off]=0;
#ifdef DEBUG
      printf("read='%s'\n",header);
#endif	
      if (!ret) 
	ret=-6;
      else if (sscanf(header,"HTTP/1.0 %03d",&ret)!=1) 
	  ret=-7;
    }
  }
  /* close socket */
  close(s);
  return ret;
}

