/*
 * Adonis project / software utilities:
 *  Http put/get mini lib
 *  written by L. Demailly
 *  (c) 1996 Observatoire de Paris - Meudon - France
 *
 * $Id: http_lib.c,v 2.3 1996/04/18 08:00:55 dl Exp dl $ 
 *
 * Description : Use http protocol, connects to server to echange data
 *
 * $Log: http_lib.c,v $
 * Revision 2.3  1996/04/18  08:00:55  dl
 * outputing header on stderr. bug fix on get (no header). OS9 compat defs.
 *
 * Revision 2.2  1996/04/17  14:58:12  dl
 * using Date: rcs keyword
 *
 * Revision 2.1  1996/04/17  14:54:12  dl
 * rcs keyword/version fix
 *
 * Revision 2.0  1996/04/17  14:52:05  dl
 * Major rewrite, using utility functions, commented for c2man
 * added http_get
 *
 * Revision 1.7  1996/04/17  08:12:49  dl
 * uh! missing n++ in read_line, was always returning failure
 *
 * Revision 1.6  1996/04/16  15:31:39  dl
 * new read_line function, showing infos with ifdef VERBOSE
 *
 * Revision 1.5  1996/04/16  15:00:47  dl
 * OS9 compatibilty define. \n -> \012 to be sure to have a LF
 *
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

static char *rcsid="$Id: http_lib.c,v 2.3 1996/04/18 08:00:55 dl Exp dl $";

#define VERBOSE

/* http_lib - Http data exchanges mini library.
 */


#ifdef OSK
/* OS/9 includes */
#include <modes.h>
#include <types.h>
#include <machine/reg.h>
#include <INET/socket.h>
#include <INET/in.h>
#include <INET/netdb.h>
#include <INET/pwd.h>
extern char *malloc();
#else
/* unix */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

static int http_read_line (int fd,char *buffer, int max) ;
static int http_read_buffer (int fd,char *buffer, int max) ;

#endif /* OS9/Unix */

#include <stdio.h>

#include "http_lib.h"

#define SERVER_DEFAULT "adonis"

/* pointer to a mallocated string containing server name or NULL */
char *http_server=NULL ;
/* server port number */
int  http_port=5757;

/*
 * read a line from file descriptor
 * returns the number of bytes read. negative if a read error occured
 * before the end of line or the max.
 * cariage returns (CR) are ignored.
 */
static int http_read_line (fd,buffer,max) 
     int fd; /* file descriptor to read from */
     char *buffer; /* placeholder for data */
     int max; /* max number of bytes to read */
{ /* not efficient on long lines (multiple unbuffered 1 char reads) */
  int n=0;
  while (n<max) {
    if (read(fd,buffer,1)!=1) {
      n= -n;
      break;
    }
    n++;
    if (*buffer=='\015') continue; /* ignore CR */
    if (*buffer=='\012') break;    /* LF is the separator */
    buffer++;
  }
  *buffer=0;
  return n;
}


/*
 * read data from file descriptor
 * retries reading until the number of bytes requested is read.
 * returns the number of bytes read. negative if a read error (EOF) occured
 * before the requested length.
 */
static int http_read_buffer (fd,buffer,length) 
     int fd;  /* file descriptor to read from */
     char *buffer; /* placeholder for data */
     int length; /* number of bytes to read */
{
  int n,r;
  for (n=0; n<length; n+=r) {
    r=read(fd,buffer,length-n);
    if (r<=0) return -n;
    buffer+=r;
  }
  return n;
}


typedef enum 
{
  CLOSE,  /* Close the socket after the query (for put) */
  KEEP_OPEN /* Keep it open */
} querymode;

#ifndef OSK

static http_retcode http_query(char *command, 
			       char *additional_header, querymode mode, 
			       char* data, int length, int *pfd);
#endif

/* beware that filename+type+rest of header must not exceed MAXBUF */
/* so we limit filename to 256 and type to 64 chars in put & get */
#define MAXBUF 512

/*
 * Pseudo general http query
 *
 * send a command and additional headers to the http server
 */
static http_retcode http_query(command, additional_header, mode,
			      data, length, pfd) 
     char *command;		/* command to send  */
     char *additional_header;	/* additional header */
     querymode mode; 		/* type of query */
     char *data;  /* Data to send after header. If NULL, not data is sent */
     int length;  /* size of data */
     int *pfd;    /* pointer to variable where to set file descriptor value */
{
  int     s;
  struct  hostent *hp;
  struct  sockaddr_in     server;
  char header[MAXBUF];
  int  hlg;
  http_retcode ret;
  
  if (pfd) *pfd=-1;

  /* get host info by name :*/
  if ((hp = gethostbyname( http_server ? http_server : SERVER_DEFAULT ))) {
    memset((char *) &server,0, sizeof(server));
    memmove((char *) &server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
    server.sin_port = (unsigned short) htons(http_port);
  } else
    return ERRHOST;

  /* create socket */
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return ERRSOCK;
  setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, 0, 0);

  /* connect to server */
  if (connect(s, &server, sizeof(server)) < 0) 
    ret=ERRCONN;
  else {
    if (pfd) *pfd=s;
    
    /* create header */
    sprintf(header,
 "%s HTTP/1.0\012User-Agent: adlib/2 ($Date: 1996/04/18 08:00:55 $)\012%s\012",
	    command,
	    additional_header
	    );
    hlg=strlen(header);

    /* send header */
    if (write(s,header,hlg)!=hlg)
      ret= ERRWRHD;

    /* send data */
    else if (length && data && (write(s,data,length)!=length) ) 
      ret= ERRWRDT;

    else {
      /* read result & check */
      ret=http_read_line(s,header,MAXBUF-1);
#ifdef VERBOSE
      fputs(header,stderr);
      putc('\n',stderr);
#endif	
      if (ret<=0) 
	ret=ERRRDHD;
      else if (sscanf(header,"HTTP/1.0 %03d",(int*)&ret)!=1) 
	  ret=ERRPAHD;
      else if (mode==KEEP_OPEN)
	return ret;
    }
  }
  /* close socket */
  close(s);
  return ret;
}


/*
 * Put data on the server
 *
 * This function sends data to the http data server.
 * The data will be stored under the ressource name filename.
 * returns a negative error code or a positive code from the server
 *
 * limitations: filename is truncated to first 256 characters 
 *              and type to 64.
 */
http_retcode http_put(filename, data, length, overwrite, type) 
     char *filename;  /* name of the ressource to create */
     char *data;      /* pointer to the data to send   */
     int length;      /* length of the data to send  */
     int overwrite;   /* flag to request to overwrite the ressource if it
			 was already existing */
     char *type;      /* type of the data, if NULL default type is used */
{
  char command[MAXBUF];
  char header[MAXBUF];
  sprintf(command,"PUT /%.256s",filename); /* limit filename to 256 chars */
  sprintf(header,"Content-length: %d\012Content-type: %.64s\012%s",
	    length,
	    type ? type : "binary/octet-stream",
	    overwrite ? "Control: overwrite=1\012" : ""
	    );
  return http_query(command,header,CLOSE, data, length, NULL);
}


/*
 * Get data from the server
 *
 * This function gets data from the http data server.
 * The data is read from the ressource named filename.
 * Address of new new allocated memory block is filled in pdata
 * whose length is returned via plength.
 * 
 * returns a negative error code or a positive code from the server
 * 
 *
 * limitations: filename is truncated to first 256 characters
 */
http_retcode http_get(filename, pdata, plength, typebuf) 
     char *filename; /* name of the ressource to read */
     char **pdata; /* address of a pointer variable which will be set
		      to point toward allocated memory containing read data.*/
     int  *plength;/* address of integer variable which will be set to
		      length of the read data */
     char *typebuf; /* allocated buffer where the read data type is returned.
		    If NULL, the type is not returned */
     
{
  http_retcode ret;
  
  char command[MAXBUF];
  char header[MAXBUF];
  char *pc;
  int  fd;
  int  n,length=-1;

  if (!pdata) return ERRNULL; else *pdata=NULL;
  if (plength) *plength=0;
  if (typebuf) *typebuf='\0';

  sprintf(command,"GET /%.256s",filename); /* limit filename to 256 chars */
  ret=http_query(command,"",KEEP_OPEN, NULL, NULL, &fd);
  if (ret==200) {
    while (1) {
      n=http_read_line(fd,header,MAXBUF-1);
#ifdef VERBOSE
      fputs(header,stderr);
      putc('\n',stderr);
#endif	
      if (n<=0) {
	close(fd);
	return ERRRDHD;
      }
      /* empty line ? (=> end of header) */
      if ( n>0 && (*header)=='\0') break;
      /* try to parse some keywords : */
      /* convert to lower case 'till a : is found or end of string */
      for (pc=header; (*pc!=':' && *pc) ; pc++) *pc=tolower(*pc);
      sscanf(header,"content-length: %d",&length);
      if (typebuf) sscanf(header,"content-type: %s",typebuf);
    }
    if (length<=0) {
      close(fd);
      return ERRNOLG;
    }
    if (plength) *plength=length;
    if (!(*pdata=malloc(length))) {
      close(fd);
      return ERRMEM;
    }
    n=http_read_buffer(fd,*pdata,length);
    close(fd);
    if (n!=length) ret=ERRRDDT;
  } else if (ret>=0) close(fd);
  return ret;
}


/*
 * Request the header
 *
 * This function outputs the header of thehttp data server.
 * The header is from the ressource named filename.
 * The length and type of data is eventually returned (like for http_get(3))
 *
 * returns a negative error code or a positive code from the server
 * 
 * limitations: filename is truncated to first 256 characters
 */
http_retcode http_head(filename, plength, typebuf) 
     char *filename; /* name of the ressource to read */
     int  *plength;/* address of integer variable which will be set to
		      length of the data */
     char *typebuf; /* allocated buffer where the data type is returned.
		    If NULL, the type is not returned */
{
/* mostly copied from http_get : */
  http_retcode ret;
  
  char command[MAXBUF];
  char header[MAXBUF];
  char *pc;
  int  fd;
  int  n,length=-1;

  if (plength) *plength=0;
  if (typebuf) *typebuf='\0';

  sprintf(command,"HEAD /%.256s",filename); /* limit filename to 256 chars */
  ret=http_query(command,"",KEEP_OPEN, NULL, NULL, &fd);
  if (ret==200) {
    while (1) {
      n=http_read_line(fd,header,MAXBUF-1);
#ifdef VERBOSE
      fputs(header,stderr);
      putc('\n',stderr);
#endif	
      if (n<=0) {
	close(fd);
	return ERRRDHD;
      }
      /* empty line ? (=> end of header) */
      if ( n>0 && (*header)=='\0') break;
      /* try to parse some keywords : */
      /* convert to lower case 'till a : is found or end of string */
      for (pc=header; (*pc!=':' && *pc) ; pc++) *pc=tolower(*pc);
      sscanf(header,"content-length: %d",&length);
      if (typebuf) sscanf(header,"content-type: %s",typebuf);
    }
    if (plength) *plength=length;
    close(fd);
  } else if (ret>=0) close(fd);
  return ret;
}



/*
 * Delete data on the server
 *
 * This function request a DELETE on the http data server.
 *
 * returns a negative error code or a positive code from the server
 *
 * limitations: filename is truncated to first 256 characters 
 */

http_retcode http_delete(filename) 
     char *filename;  /* name of the ressource to create */
{
  char command[MAXBUF];
  sprintf(command,"DELETE /%.256s",filename); /* limit filename to 256 chars */
  return http_query(command,"",CLOSE, NULL, 0, NULL);
}



/* parses an url : setting the http_server and http_port global variables
 * and returning the filename to pass to http_get/put/...
 * returns a negative error code or 0 if sucessfully parsed.
 */
http_retcode http_parse_url(url,pfilename)
    /* writeable copy of an url */
     char *url;  
    /* address of a pointer that will be filled with allocated filename
     * the pointer must be equal to NULL before calling or it will be 
     * automatically freed (free(3))
     */
     char **pfilename; 
{
  char *pc,c;
  
  http_port=80;
  if (http_server) {
    free(http_server);
    http_server=NULL;
  }
  if (*pfilename) {
    free(*pfilename);
    *pfilename=NULL;
  }
  
  if (strncasecmp("http://",url,7)) {
#ifdef VERBOSE
    fprintf(stderr,"invalid url (must start with 'http://')\n");
#endif
    return ERRURLH;
  }
  url+=7;
  for (pc=url,c=*pc; (c && c!=':' && c!='/');) c=*pc++;
  *(pc-1)=0;
  if (c==':') {
    if (sscanf(pc,"%d",&http_port)!=1) {
#ifdef VERBOSE
      fprintf(stderr,"invalid port in url\n");
#endif
      return ERRURLP;
    }
    for (pc++; (*pc && *pc!='/') ; pc++) ;
    if (*pc) pc++;
  }

  http_server=strdup(url);
  *pfilename= strdup ( c ? pc : "") ;

#ifdef VERBOSE
  fprintf(stderr,"host=(%s), port=%d, filename=(%s)\n",
	    http_server,http_port,*pfilename);
#endif
  return OK0;
}

