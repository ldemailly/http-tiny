/*
 * Adonis project / software utilities:
 *  Http put/get mini lib
 *  written by L. Demailly
 *  (c) 1996 Observatoire de Paris - Meudon - France
 *
 * $Id:$
 *
 */

 /* declarations */

extern char *http_server;

extern int http_port;

typedef enum 
{
  /* Client side errors */
  ERRHOST=-1, /* No such host */
  ERRSOCK=-2, /* Can't create socket */
  ERRCONN=-3, /* Can't connect to host */
  ERRWRHD=-4, /* Write error on socket while writing header */
  ERRWRDT=-5, /* Write error on socket while writing data */
  ERRRDHD=-6, /* Read error on socket while reading result */
  ERRPAHD=-7, /* Invalid answer from data server */
  ERRNULL=-8, /* Null data pointer */
  ERRNOLG=-9, /* No/Bad length in header */
  ERRMEM=-10, /* Can't allocate memory */
  ERRRDDT=-11,/* Read error while reading data */

  /* Return code by the server */
  ERR400=400, /* Invalid query */
  ERR403=403, /* Forbidden */
  ERR408=408, /* Request timeout */
  ERR500=500, /* Server error */
  ERR501=501, /* Not implemented */
  ERR503=503, /* Service overloaded */

  /* Succesful results */
  OK201=201, /* Ressource succesfully created */
  OK200=200  /* Ressource succesfully read */

} return_code;

/* prototypes */

#ifndef OSK
return_code http_put(char *filename, char *data, int length, 
	     int overwrite, char *type) ;
return_code http_get(char *filename, char **pdata,int *plength, char *typebuf);

#endif
