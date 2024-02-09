
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h> 

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

void sendServerClient(struct thData tdL, char mesaj[100]){
  int marime=strlen(mesaj);
  printf("[thread]- %d - S-au scris in SOCKETPAIR %d bytes\n", tdL.idThread, marime);
  write(tdL.cl, &marime, sizeof(int));
  write(tdL.cl, mesaj, strlen(mesaj));
  //write(tdL.cl, &flag_trimitere_mesaje, sizeof(int));
}



static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

static int callbacktest(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s", argv[i] ? argv[i] : "NULL");
   }
   printf(" ");
   return 0;
}



int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;


  sqlite3 *db;
  int rc;
  char *sql;
  char *zErrMsg = 0;

  rc = sqlite3_open("offline_messenger.db", &db);

  sql = "CREATE TABLE IF NOT EXISTS USER(" \
        "ID INT PRIMARY KEY  NOT NULL," \
        "NAME           TEXT NOT NULL," \
        "IS_ACTIVE      BOOL NOT NULL);";

  rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    sql = "CREATE TABLE IF NOT EXISTS JUNCTION(" \
    "USER_ID         INT NOT NULL," \
    "WITH_USER_ID    INT NOT NULL," \
    "CONVERSATION_ID INT NOT NULL," \
    "PRIMARY KEY (USER_ID, WITH_USER_ID));";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    sql = "CREATE TABLE IF NOT EXISTS CONVERSATION(" \
          "INDEX_MESSAGE         INT NOT NULL,"
          "CONVERSATION_ID        INT NOT NULL," \
          "FROM_ID                INT NOT NULL," \
          "MESSAGE_CONTENT        TEXT NOT NULL," \
          "READ_FLAG              BOOL NOT NULL," \
          "CREATED_ON DATE PRIMARY KEY NOT NULL);";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};

char* extractuser(char aux[])
{
    //char aux[100];
    //strcpy(aux,sir);
    char *userbd = strtok(aux, " ");
    userbd = strtok(NULL, " ");
    aux[0] = '\0';
    strcat(aux, "'");
    strcat(aux, userbd);
    strcat(aux, "'");
    return aux;
}
char* getstringid(char stringid[], int user_id)
{
    //char stringid[100];
    stringid[0] = '\0';
    int factor = 1;
    int cifra;
    int result = 0;
    char character;
    while(user_id > 0){
        cifra = user_id % 10;
        user_id = user_id / 10;
        result = result * 10 + cifra;
    }                                        
    while(result > 0){
        cifra = result % 10;
        //printf("cifra: %d\n", cifra);
        character = cifra + '0';
        //printf("character: %c\n", character);
        strncat(stringid, &character, 1);                                        
        result = result / 10;
    }
    return stringid;
}


void raspunde(void *arg)
{
    int nr, i=0;
	  struct thData tdL; 
	  tdL= *((struct thData*)arg);

    char sir[300], mesaj[100], user[100], usernamelist[100];
    char* comanda;
    int buffer, marime, flag_user;
    int flag_logare = 0;
    int fd_login;

    char* userfromfunction;
    sqlite3_stmt* stmt;
    sqlite3 *db;
    int rc;
    char *sql;
    char *zErrMsg = 0;
      
    char username[100];
    char sql_command[1000];
    char aux[100];
    int flag_exist;
    int nr_rows;
    char aux2[200];
    char aux3[100];
    char string_with_user1[100];

    char stringid[100];
    char aux4[100];


     rc = sqlite3_open("offline_messenger.db", &db);

	    do 
        {
            bzero(sir,300);
            bzero(mesaj, 100);
            bzero(user,100);
            bzero(aux,100);
            bzero(aux2,200);
            bzero(aux3,100);
            bzero(aux4,100);
            if (-1 == (buffer = read(tdL.cl, sir, 300)))
                    perror("Eroare la citirea din FIFO.");
            else 
            {
                
                    strcpy(aux,sir);
                    strcpy(aux2, sir);
                    strcpy(aux3, sir);
                    strcpy(aux4, sir);
                    comanda = strtok(sir," ");
                    if(strcmp(comanda, "login")==0 && flag_logare == 1)
                    {
                        strcpy(mesaj, "Sunteti deja logat la un cont.");
                        sendServerClient(tdL, mesaj);
                    }
                    else 
                        if(strcmp(comanda, "login") == 0 && flag_logare == 0)
                        {
                            comanda = strtok(NULL, " ");
                            strcpy(string_with_user1, comanda);
                            //printf("%s\n", string_with_user1);
                            int pid_logare, pipe_logare[2];
                            if(-1 == pipe(pipe_logare) )
                            {
                                perror("Eroare la crearea canalului anonim");  exit(1);
                            }
                            if(-1 == ( pid_logare = fork() ))
                            {
                                perror("Eroare la crearea unui proces fiu");  exit(2);
                            }
                            if(pid_logare) // proces parinte pentru login
                            {
                                write(pipe_logare[1],comanda,strlen(comanda));
                                close(pipe_logare[1]);                     
                                wait(NULL);
                                read(pipe_logare[0], &flag_user, 4);
                                
                                if(flag_user == 1) // verificam logarea
                                {
                                    flag_logare = 1;
                                    strcpy(mesaj, "Logarea a avut loc cu succes.");
                                    sendServerClient(tdL, mesaj);
                                    
                                    userfromfunction = extractuser(aux);
                                    strcpy(username,userfromfunction);
                                    strcpy(sql_command, "UPDATE USER SET IS_ACTIVE = 1 WHERE NAME = ");
                                    strcat(sql_command, userfromfunction);
                                    sql = sql_command;
                                    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                                }
                                else
                                {
                                    strcpy(mesaj, "Logarea a esuat.");
                                    sendServerClient(tdL, mesaj);
                                }
                            }
                            else // proces copil pentru login
                            {
                                if(-1 == (fd_login = read(pipe_logare[0], user, 100)))
                                {
                                    perror("Eroare citire din pipe.\n");
                                    exit(1);
                                }

                                user[fd_login] = '\0';    
                                close(pipe_logare[0]);                    

                                
                                userfromfunction = extractuser(aux);
                                strcpy(username,userfromfunction);
                                strcpy(sql_command, "SELECT * FROM USER WHERE NAME = ");
                                strcat(sql_command, userfromfunction);
                                //printf("asdasd: %s\n",sql_command);
                                sqlite3_prepare_v2(db, sql_command , -1, &stmt, 0);
                                sqlite3_step(stmt);
                                flag_exist = sqlite3_column_int(stmt, 0);
                                //printf("%d\n",ajutor);
                                if(flag_exist == 0)
                                    flag_user = 0;
                                else
                                    flag_user = 1;

                                write(pipe_logare[1], &flag_user, 4);
                                //fseek(in_file, 0, SEEK_SET );
                                strcpy(usernamelist, "");
                                strcpy(user, "");
                                strcpy(sir, "");
                                flag_user = 0;
                                exit(0);
                            } 
                        }
                        else 
                            if(strcmp(comanda, "register")== 0 && flag_logare == 0)
                            {
                                userfromfunction = extractuser(aux);
                                strcpy(username,userfromfunction);
                                strcpy(sql_command, "SELECT * FROM USER WHERE NAME = ");
                                strcat(sql_command, username);
                                sqlite3_prepare_v2(db, sql_command, -1, &stmt, 0);
                                sqlite3_step(stmt);
                                flag_exist = sqlite3_column_int(stmt, 0);
                                if(flag_exist == 0)
                                {
                                    strcpy(mesaj, "V-ati inregistrat cu succes.");
                                    sendServerClient(tdL, mesaj);
                                    
                                    strcpy(sql_command, "INSERT INTO USER (ID, NAME, IS_ACTIVE) VALUES (");
                                    char sql_query[1000] = "SELECT COUNT(*) FROM USER;";
                                    sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                    sqlite3_step(stmt);
                                    nr_rows = sqlite3_column_int(stmt, 0);
                                    char c = nr_rows + 1 + '0';
                                    strncat(sql_command, &c, 1);
                                    strcat(sql_command, ", ");
                                    strcat(sql_command, username);
                                    strcat(sql_command, ", 0);");
                                    sql = sql_command;
                                    printf("%s\n", sql_command);
                                    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                                }
                                else
                                {
                                    strcpy(mesaj, "Utilizatorul este inregistrat deja.");
                                    sendServerClient(tdL, mesaj);
                                }
                            }
                            else
                                if(strcmp(comanda, "register")== 0 && flag_logare == 1)
                                {
                                    strcpy(mesaj, "Trebuie sa va delogati intai.");
                                    sendServerClient(tdL, mesaj);
                                }
                                else
                                    if(strcmp(comanda, "msg") == 0 && flag_logare == 1)
                                    {
                                        userfromfunction = extractuser(aux);
                                        strcpy(mesaj, "Ati trimis mesajul lui ");
                                        strcat(mesaj, userfromfunction);
                                        sendServerClient(tdL, mesaj);
                                        
                                        char sql_query[1000];
                                        strcpy(sql_query,"SELECT ID FROM USER WHERE NAME = ");
                                        char aux_pentru_user1[100] = "'";
                                        strcat (aux_pentru_user1, string_with_user1);
                                        strcat (aux_pentru_user1, "'");
                                        strcat(sql_query, aux_pentru_user1);
                                        printf("%s\n", sql_query);
                                        rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                        rc = sqlite3_step(stmt);
                                        int user_id1 = sqlite3_column_int(stmt, 0);

                                        strcpy(sql_query,"SELECT ID FROM USER WHERE NAME = ");
                                        strcat(sql_query, userfromfunction);
                                        rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                        rc = sqlite3_step(stmt);
                                        int user_id2 = sqlite3_column_int(stmt, 0);

                                        char string_with_user1_id[20];
                                        string_with_user1_id[0] = '\0';
                                        char string_with_user2_id[20];
                                        string_with_user2_id[0] = '\0';

                                        strcpy(sql_query, "SELECT CONVERSATION_ID FROM JUNCTION WHERE (USER_ID = ");
                                        strcat(sql_query, getstringid(stringid, user_id1));
                                        strcat(sql_query, " AND WITH_USER_ID = ");
                                        strcat(sql_query, getstringid(stringid, user_id2));
                                        strcat(sql_query, ");");
                                        printf("%s\n", sql_query);
                                        rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                        rc = sqlite3_step(stmt);
                                        int conv_id = sqlite3_column_int(stmt, 0);
                                        printf("Conv_id: %d\n", conv_id);
                                        if (conv_id == 0)
                                        {
                                            strcpy(sql_query, "INSERT INTO JUNCTION (USER_ID, WITH_USER_ID, CONVERSATION_ID) VALUES (");
                                            strcat(sql_query, getstringid(stringid, user_id1));
                                            strcat(sql_query, ", ");
                                            strcat(sql_query, getstringid(stringid, user_id2));
                                            strcat(sql_query, ", ");

                                            char sql_interogare[1000] = "SELECT CONVERSATION_ID FROM JUNCTION WHERE (USER_ID = ";
                                            strcat(sql_interogare, getstringid(stringid, user_id2));
                                            strcat(sql_interogare, " AND WITH_USER_ID = ");
                                            strcat(sql_interogare, getstringid(stringid, user_id1));
                                            strcat(sql_interogare, ");");
                                            rc = sqlite3_prepare_v2(db, sql_interogare, -1, &stmt, 0);
                                            rc = sqlite3_step(stmt);
                                            int xconv_id = sqlite3_column_int(stmt, 0);
                                            if(xconv_id == 0) 
                                            {
                                                char sql_query2[1000] = "SELECT COUNT(DISTINCT CONVERSATION_ID) FROM JUNCTION ORDER BY CONVERSATION_ID ASC;";
                                                sqlite3_prepare_v2(db, sql_query2, -1, &stmt, 0);
                                                sqlite3_step(stmt);
                                                xconv_id = sqlite3_column_int(stmt, 0);
                                                strcat(sql_query, getstringid(stringid, ++xconv_id));
                                                strcat(sql_query, ");");
                                            }
                                            else
                                            {
                                                strcat(sql_query, getstringid(stringid, xconv_id));
                                                strcat(sql_query, ");");
                                            }
                                            sql = sql_query;
                                            rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                                            conv_id = xconv_id;
                                        }
                                        char *send_mesaj = strtok(aux3, ":");
                                        send_mesaj = strtok(NULL, ":");

                                        strcpy(sql_command, "INSERT INTO CONVERSATION (INDEX_MESSAGE, CONVERSATION_ID, FROM_ID, MESSAGE_CONTENT, READ_FLAG, CREATED_ON) VALUES (");
                                        strcpy(sql_query, "SELECT COUNT(*) FROM CONVERSATION WHERE CONVERSATION_ID = ");
                                        strcat(sql_query, getstringid(stringid, conv_id));
                                        printf("%s\n", sql_query);
                                        strcat(sql_query, ";");
                                        sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                        sqlite3_step(stmt);
                                        nr_rows = sqlite3_column_int(stmt, 0);
                                        rc = sqlite3_exec(db, sql_query, callback, 0, &zErrMsg);
                                        strcat(sql_command, getstringid(stringid, ++nr_rows));
                                        strcat(sql_command, ", ");
                                        strcat(sql_command, getstringid(stringid, conv_id));
                                        strcat(sql_command, ", ");
                                        strcat(sql_command, getstringid(stringid, user_id1));
                                        strcat(sql_command, ", ");
                                        char mesaj_din_comanda[50] = "'";
                                        strcat(mesaj_din_comanda, send_mesaj);
                                        strcat(mesaj_din_comanda, "'");
                                        strcat(sql_command, mesaj_din_comanda);
                                        strcat(sql_command, ", 0, ");
                                        strcat(sql_command, "CURRENT_TIMESTAMP);");
                                        sql = sql_command;
                                        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                                    } 
                                    else
                                        if(strcmp(aux4, "inbox") == 0 && flag_logare == 1)
                                        {   
                                            char sql_query[1000];
                                            strcpy(sql_query,"SELECT ID FROM USER WHERE NAME = ");
                                            char aux_pentru_user1[100] = "'";
                                            strcat (aux_pentru_user1, string_with_user1);
                                            strcat (aux_pentru_user1, "'");
                                            strcat(sql_query, aux_pentru_user1);
                                            rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                            rc = sqlite3_step(stmt);
                                            int user_id1 = sqlite3_column_int(stmt, 0);
                                            
                                            strcpy(sql_command, "SELECT c.INDEX_MESSAGE, n.NAME, c.MESSAGE_CONTENT FROM JUNCTION j JOIN CONVERSATION c ON c.CONVERSATION_ID = j.CONVERSATION_ID JOIN USER n ON n.id = j.USER_ID AND c.READ_FLAG = 0 WHERE j.WITH_USER_ID = ");
                                            strcat(sql_command, getstringid(stringid, user_id1));
                                            strcat(sql_command, " AND c.FROM_ID IS NOT ");
                                            strcat(sql_command, getstringid(stringid, user_id1));
                                            strcat(sql_command, " ORDER BY c.CREATED_ON DESC;");
                                            sql = sql_command;
                                            rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

                                            typedef struct sqlite3_str sqlite3_str;
                                            sqlite3_str *bufferstmttest = sqlite3_str_new(db);
                                            strcpy(sql_query, "SELECT n.NAME FROM USER n JOIN JUNCTION j ON n.ID = j.USER_ID JOIN CONVERSATION c ON j.CONVERSATION_ID = c.CONVERSATION_ID WHERE c.FROM_ID = n.ID AND c.READ_FLAG = 0;");
                                            sql = sql_query;
                                            rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                                            sqlite3_prepare_v2(db, sql_command, -1, &stmt, 0);
                                            while (sqlite3_step(stmt) == SQLITE_ROW)
                                            {
                                                sqlite3_str_appendf(bufferstmttest, "%s.from %s: %s\n",
                                                                    (const char *)sqlite3_column_text(stmt, 0),
                                                                    (const char *)sqlite3_column_text(stmt, 1),
                                                                    (const char *)sqlite3_column_text(stmt, 2)
                                                                    );
                                            }
                                            char *display = sqlite3_str_finish(bufferstmttest);
                                            strcpy(mesaj, display);
                                            sendServerClient(tdL, mesaj);

                                            strcpy(sql_query, "UPDATE CONVERSATION SET READ_FLAG = 1 WHERE FROM_ID IS NOT ");
                                            strcat(sql_query, getstringid(stringid, user_id1));

                                            strcat(sql_query, ";");

                                            sql = sql_query;
                                            rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                                        }
                                        else
                                            if(strcmp(comanda, "history") == 0 && flag_logare == 1)
                                            {
                                                strcpy(sql_command, "SELECT c.INDEX_MESSAGE, n.NAME, c.MESSAGE_CONTENT FROM JUNCTION j JOIN CONVERSATION c ON c.CONVERSATION_ID = j.CONVERSATION_ID JOIN USER n ON n.ID = c.FROM_ID WHERE j.WITH_USER_ID = ");
                                            
                                                userfromfunction = extractuser(aux);
                                                char sql_query[1000];
                                                strcpy(sql_query,"SELECT ID FROM USER WHERE NAME = ");
                                                char aux_pentru_user1[100] = "'";
                                                strcat (aux_pentru_user1, string_with_user1);
                                                strcat (aux_pentru_user1, "'");
                                                strcat(sql_query, aux_pentru_user1);
                                                rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                                rc = sqlite3_step(stmt);
                                                int user_id1 = sqlite3_column_int(stmt, 0);
                                                strcat(sql_command, getstringid(stringid, user_id1));

                                                strcpy(sql_query,"SELECT ID FROM USER WHERE NAME = ");
                                                strcat(sql_query, userfromfunction);
                                                rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                                rc = sqlite3_step(stmt);
                                                int user_id2 = sqlite3_column_int(stmt, 0);
                                                
                                                strcat(sql_command, " AND j.USER_ID = ");
                                                strcat(sql_command, getstringid(stringid, user_id2));
                                                strcat(sql_command, ";");
                                                sql = sql_command;
                                                rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

                                                typedef struct sqlite3_str sqlite3_str;
                                                sqlite3_str *bufferstmttest = sqlite3_str_new(db);
                                                sqlite3_prepare_v2(db, sql_command, -1, &stmt, 0);
                                                while (sqlite3_step(stmt) == SQLITE_ROW)
                                                {
                                                sqlite3_str_appendf(bufferstmttest, "%s.%s: %s\n",
                                                                    (const char *)sqlite3_column_text(stmt, 0),
                                                                    (const char *)sqlite3_column_text(stmt, 1),
                                                                    (const char *)sqlite3_column_text(stmt, 2)
                                                                    );
                                                }
                                                char *display = sqlite3_str_finish(bufferstmttest);
                                                strcpy(mesaj, display);
                                                sendServerClient(tdL, mesaj);
                                            }
                                            else
                                                if(strcmp(comanda, "reply") == 0 && flag_logare == 1)
                                                {
                                                    userfromfunction = extractuser(aux);
                                                    strcpy(mesaj, "Ati trimis mesajul lui ");
                                                    strcat(mesaj, userfromfunction);
                                                    sendServerClient(tdL, mesaj);
                                                    
                                                    char sql_query[1000];
                                                    strcpy(sql_query,"SELECT ID FROM USER WHERE NAME = ");
                                                    char aux_pentru_user1[100] = "'";
                                                    //printf("USER1: %s\n",string_with_user1);
                                                    strcat (aux_pentru_user1, string_with_user1);
                                                    strcat (aux_pentru_user1, "'");
                                                    strcat(sql_query, aux_pentru_user1);
                                                    //printf("%s\n", sql_query);
                                                    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                                    rc = sqlite3_step(stmt);
                                                    int user_id1 = sqlite3_column_int(stmt, 0);

                                                    strcpy(sql_query,"SELECT ID FROM USER WHERE NAME = ");
                                                    strcat(sql_query, userfromfunction);
                                                    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                                    rc = sqlite3_step(stmt);
                                                    int user_id2 = sqlite3_column_int(stmt, 0);
                                                    //printf("%s\n", sql_query);

                                                    //printf("user1_id:%d\n", user_id1);
                                                    //printf("user2_id:%d\n", user_id2);

                                                    char string_with_user1_id[20];
                                                    string_with_user1_id[0] = '\0';
                                                    char string_with_user2_id[20];
                                                    string_with_user2_id[0] = '\0';


                                                    //strcpy(sql_command, "INSERT INTO JUNCTION (USER_ID, WITH_USER_ID, CONVERSATION_ID, LAST_READ_ID) VALUES (");
                                                    //strcat(sql_command, string_with_user1_id);
                                                    //strcat(sql_command, ", ");
                                                    //strcat(sql_command, string_with_user2_id);
                                                    //printf("%s\n", sql_command);
                                                    

                                                    strcpy(sql_query, "SELECT CONVERSATION_ID FROM JUNCTION WHERE (USER_ID = ");
                                                    strcat(sql_query, getstringid(stringid, user_id1));
                                                    strcat(sql_query, " AND WITH_USER_ID = ");
                                                    strcat(sql_query, getstringid(stringid, user_id2));
                                                    strcat(sql_query, ");");
                                                    printf("%s\n", sql_query);
                                                    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                                    rc = sqlite3_step(stmt);
                                                    int conv_id = sqlite3_column_int(stmt, 0);
                                                    //printf("Conv_id: %d\n", conv_id);
                                                    if (conv_id == 0)
                                                    {
                                                        strcpy(sql_query, "INSERT INTO JUNCTION (USER_ID, WITH_USER_ID, CONVERSATION_ID) VALUES (");
                                                        strcat(sql_query, getstringid(stringid, user_id1));
                                                        strcat(sql_query, ", ");
                                                        strcat(sql_query, getstringid(stringid, user_id2));
                                                        strcat(sql_query, ", ");

                                                        char sql_interogare[1000] = "SELECT CONVERSATION_ID FROM JUNCTION WHERE (USER_ID = ";
                                                        strcat(sql_interogare, getstringid(stringid, user_id2));
                                                        strcat(sql_interogare, " AND WITH_USER_ID = ");
                                                        strcat(sql_interogare, getstringid(stringid, user_id1));
                                                        strcat(sql_interogare, ");");
                                                        rc = sqlite3_prepare_v2(db, sql_interogare, -1, &stmt, 0);
                                                        rc = sqlite3_step(stmt);
                                                        int xconv_id = sqlite3_column_int(stmt, 0);
                                                        //printf("%d\n",xconv_id);
                                                        if(xconv_id == 0) 
                                                        {
                                                            char sql_query2[1000] = "SELECT COUNT(DISTINCT CONVERSATION_ID) FROM JUNCTION ORDER BY CONVERSATION_ID ASC;";
                                                            sqlite3_prepare_v2(db, sql_query2, -1, &stmt, 0);
                                                            sqlite3_step(stmt);
                                                            xconv_id = sqlite3_column_int(stmt, 0);
                                                            //printf("xconv_id: %d\n", xconv_id);
                                                            strcat(sql_query, getstringid(stringid, ++xconv_id));
                                                            strcat(sql_query, ");");
                                                        }
                                                        else
                                                        {
                                                            strcat(sql_query, getstringid(stringid, xconv_id));
                                                            strcat(sql_query, ");");
                                                        }
                                                        sql = sql_query;
                                                        //rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                                                        conv_id = xconv_id;
                                                    }
                                                    //printf("%s\n", aux3);
                                                    char *send_mesaj = strtok(aux3, ":");
                                                    send_mesaj = strtok(NULL, " ");
                                                    //printf("%s\n", send_mesaj);
                                                    char *index = strtok(aux2, " ");
                                                    index = strtok(NULL, " ");
                                                    index = strtok(NULL, " ");
                                                    //printf("%s\n", bla);


                                                    strcpy(sql_command, "INSERT INTO CONVERSATION (INDEX_MESSAGE, CONVERSATION_ID, FROM_ID, MESSAGE_CONTENT, READ_FLAG, CREATED_ON) VALUES (");
                                                    strcpy(sql_query, "SELECT COUNT(*) FROM CONVERSATION WHERE CONVERSATION_ID = ");
                                                    strcat(sql_query, getstringid(stringid, conv_id));
                                                    printf("%s\n", sql_query);
                                                    strcat(sql_query, ";");
                                                    sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                                    sqlite3_step(stmt);
                                                    nr_rows = sqlite3_column_int(stmt, 0);
                                                    //printf("Numar de randuri: %d\n", nr_rows);
                                                    rc = sqlite3_exec(db, sql_query, callback, 0, &zErrMsg);
                                                    strcat(sql_command, getstringid(stringid, ++nr_rows));
                                                    strcat(sql_command, ", ");
                                                    strcat(sql_command, getstringid(stringid, conv_id));
                                                    strcat(sql_command, ", ");
                                                    strcat(sql_command, getstringid(stringid, user_id1));
                                                    strcat(sql_command, ", ");
                                                    char mesaj_din_comanda[50] = "'";
                                                    strcat(mesaj_din_comanda, send_mesaj);
                                                    strcat(mesaj_din_comanda, " - replied from (");
                                                    strcat(mesaj_din_comanda, index);
                                                    strcat(mesaj_din_comanda, ".");

                                                    strcpy(sql_query, "SELECT MESSAGE_CONTENT FROM CONVERSATION WHERE INDEX_MESSAGE = ");
                                                    strcat(sql_query, index);
                                                    strcat(sql_query, ";");
                                                    sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
                                                    sqlite3_step(stmt);
                                                    char mesaj_text[100];
                                                    strcpy(mesaj_text, sqlite3_column_text(stmt, 0));
                                                    rc = sqlite3_exec(db, sql_query, callback, 0, &zErrMsg);

                                                    strcat(mesaj_din_comanda, mesaj_text);
                                                    strcat(mesaj_din_comanda, ")'");


                                                    strcat(sql_command, mesaj_din_comanda);
                                                    strcat(sql_command, ", 0, ");
                                                    strcat(sql_command, "CURRENT_TIMESTAMP);");
                                                    printf("%s\n", sql_command);
                                                    sql = sql_command;
                                                    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg); 




                                                }
                                                else
                                                    if(strcmp(aux4, "logout") == 0 && flag_logare == 0)
                                                    {
                                                        printf("%s\n", aux4);
                                                        strcpy(mesaj, "Nu sunteti logat.");
                                                        sendServerClient(tdL, mesaj);
                                                    }
                                                    else
                                                        if(strcmp(aux4, "logout") == 0 && flag_logare == 1)
                                                        {
                                                            flag_logare = 0;
                                                            strcpy(mesaj, "V-ati delogat cu succes.");
                                                            sendServerClient(tdL, mesaj);
                                                            char sql_command[100] = "UPDATE USER SET IS_ACTIVE = 0 WHERE NAME = ";
                                                            strcat(sql_command, username);
                                                            sql = sql_command;
                                                            rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                                                        }
                                                        else
                                                            if(strcmp(aux, "list online") == 0)
                                                            {
                                                                strcpy(mesaj, "Lista de utilizatori online: ");

                                                                typedef struct sqlite3_str sqlite3_str;
                                                                sqlite3_str *bufferstmt = sqlite3_str_new(db);
                                                                strcpy(sql_command, "SELECT NAME FROM USER WHERE IS_ACTIVE = 1;");
                                                                sqlite3_prepare_v2(db, sql_command, -1, &stmt, 0);
                                                                while (sqlite3_step(stmt) == SQLITE_ROW)
                                                                {
                                                                    sqlite3_str_appendf(bufferstmt, "%s, ",
                                                                                        (const char *)sqlite3_column_text(stmt, 0)
                                                                                        );
                                                                }
                                                                char *display = sqlite3_str_finish(bufferstmt);
                                                                display[strlen(display)-2] = '\0';
                                                                char copie[100];
                                                                strcpy(copie, display);
                                                                strcat(mesaj, copie);
                                                                sendServerClient(tdL, mesaj);
                                                            }
                                                            else    
                                                                if(strcmp(aux, "quit") == 0) 
                                                                {
                                                                    buffer = 0;
                                                                    strcpy(mesaj, "Clientul a fost inchis cu succes.");
                                                                    sendServerClient(tdL, mesaj);
                                                                    char sql_command[100] = "UPDATE USER SET IS_ACTIVE = 0 WHERE NAME = ";
                                                                    strcat(sql_command, username);
                                                                    sql = sql_command;
                                                                    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                                                                    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                                                                }
                                                                else 
                                                                {
                                                                    printf("%s\n", aux4);
                                                                    strcpy(mesaj, "Comanda nu este valida.");
                                                                    sendServerClient(tdL, mesaj);
                                                                } 
                    strcpy(mesaj, "");
                
            }
        } while (buffer > 0);
  sqlite3_close(db);
}

