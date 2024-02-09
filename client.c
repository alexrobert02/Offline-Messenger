
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#define PORT "2908"
#define ADRESS "127.0.0.1"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0;
  char buf[10];

  /* exista toate argumentele in linia de comanda? */
  /*if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }
  */
  /* stabilim portul */
  port = atoi (PORT);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(ADRESS);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  /* citirea mesajului */
  char sir[300], mesaj[50];
  int flag_logare, marime, receptez;
  int buffer;
  int flag_trimitere_mesaje = 0;
  printf ("[client]Comanda: ");
  fflush (stdout);
  while (gets(sir), !feof(stdin))
  {  
      flag_trimitere_mesaje = 0;
      if ((buffer = write(sd, sir, strlen(sir))) == -1)
          perror("Eroare la write() spre server.\n");
      else
      {
          //while(1){
          read(sd, &receptez, sizeof(int));
          marime = read(sd, mesaj, receptez);
          //read(sd, &flag_trimitere_mesaje, sizeof(int));
          mesaj[marime] = '\0';
          if (strcmp(mesaj, "Clientul a fost inchis cu succes.")==0) 
          {
              printf("\n%s\n", mesaj);
              printf("\n");
              close(sd);
              return 0;
          }
          else 
          {
              printf("\n%s\n", mesaj);
              printf("\n");
              fflush (stdout);
              //close(sd);
          }  
          //if(flag_trimitere_mesaje == 1)
            //break; 
          //}
      }
  printf("[client]Comanda: "); 
  }
  //nr=atoi(buf);
  //scanf("%d",&nr);
  
  //printf("[client] Am citit: %s\n",buf);

  /* trimiterea mesajului la server */
  //if (write (sd,&nr,sizeof(int)) <= 0)
    {
      //perror ("[client]Eroare la write() spre server.\n");
      //return errno;
    }

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
  //if (read (sd, &nr,sizeof(int)) < 0)
    {
      //perror ("[client]Eroare la read() de la server.\n");
      //return errno;
    }
  /* afisam mesajul primit */
  //printf ("[client]Mesajul primit este: %d\n", nr);

  /* inchidem conexiunea, am terminat */
  //close (sd);
}

