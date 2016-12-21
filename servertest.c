#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<sqlite3.h>
#include<sys/wait.h>
#include<time.h>

#define QUEUE_SIZE 16

int counter = 0;
char pending_msg[100][100];
char timestamp[100][100];
char email_id[100][100];
char images[100][10241];
char imagearray[10241];
char image_paths[100][80];
char path[80];
char pending_subject[100][100];
int count_pending_msg = 0;

//callback is used for sqlite3 functionality
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
  int i;
  for(i = 0; i < argc-4; i++)
  {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    strcpy(pending_msg[count_pending_msg], argv[i]);
    strcpy(timestamp[count_pending_msg],argv[i+1]);
    strcpy(email_id[count_pending_msg], argv[i+2]);
    strcpy(pending_subject[count_pending_msg], argv[i+3]);
    strcpy(image_paths[count_pending_msg], argv[i+4]);
    count_pending_msg++;
  }
  printf("\n");
  return 0;
}
//**************************************************************************
//**************************************************************************
//Function to send image
int send_image(int socket, char image_path[]){

   FILE *picture;
   int size, read_size, stat, packet_index;
   char send_buffer[10240], read_buffer[256];
   packet_index = 1;

   picture = fopen(image_path, "r");
   printf("Getting Picture Size\n");   

   if(picture == NULL) {
        printf("Error Opening Image File"); } 

   fseek(picture, 0, SEEK_END);
   size = ftell(picture);
   fseek(picture, 0, SEEK_SET);
   printf("Total Picture size: %d\n",size);

   //Send Picture Size
   printf("Sending Picture Size\n");
   write(socket, (void *)&size, sizeof(int));

   //Send Picture as Byte Array
   printf("Sending Picture as Byte Array\n");

   do { //Read while we get errors that are due to signals.
      stat = read(socket, &read_buffer , 255);
      printf("Bytes read: %i\n",stat);
   } while (stat < 0);

   printf("Received data in socket\n");
   printf("Socket data: %s\n", read_buffer);

   while(!feof(picture)) {
   //while(packet_index = 1){
      //Read from the file into our send buffer
      read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

      //Send data through our socket 
      do{
        stat = write(socket, send_buffer, read_size);  
      }while (stat < 0);

      printf("Packet Number: %d\n",packet_index);
      printf("Packet Size Sent: %d\n",read_size);     
      printf(" \n");
      printf(" \n");


      packet_index++;  

      //Zero out our send buffer
      bzero(send_buffer, sizeof(send_buffer));
     }
     return 0;
  }
//**********************************************************
int receive_image(int socket)
{ // Start function 

int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index = 1,stat;

char verify = '1';
FILE *image;

//Find the size of the image
do{
stat = read(socket, &size, sizeof(int));
}while(stat<0);

printf("Packet received.\n");
printf("Packet size: %i\n",stat);
printf("Image size: %i\n",size);
printf(" \n");

char buffer[] = "Got it";

//Send our verification signal
do{
stat = write(socket, &buffer, sizeof(int));
}while(stat<0);

printf("Reply sent\n");
printf(" \n");
strcpy(path, "/home/gk/Documents/Test/ServerImages/img");
char path1[10];
sprintf(path1, "%d", ++counter);
strcat(path, path1);
strcat(path, ".png");
printf("%s\n", path);
image = fopen(path, "w");

if( image == NULL) {
printf("Error has occurred. Image file could not be opened\n");
return -1; }

//Loop while we have not received the entire file yet


int need_exit = 0;
struct timeval timeout = {10,0};

fd_set fds;
int buffer_fd, buffer_out;

while(recv_size < size) {
//while(packet_index < 2){

    FD_ZERO(&fds);
    FD_SET(socket,&fds);

    buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

    if (buffer_fd < 0)\
       printf("error: bad file descriptor set.\n");

    if (buffer_fd == 0)
       printf("error: buffer read timeout expired.\n");

    if (buffer_fd > 0)
    {
        do{
               read_size = read(socket,imagearray, 10241);
            }while(read_size <0);

            printf("Packet number received: %i\n",packet_index);
        printf("Packet size: %i\n",read_size);


        //Write the currently read data into our image file
         write_size = fwrite(imagearray,1,read_size, image);


         printf("Written image size: %i\n",write_size); 

             if(read_size !=write_size) {
                 printf("error in read write\n");    }


             //Increment the total number of bytes read
             recv_size += read_size;
             packet_index++;
             printf("Total received image size: %i\n",recv_size);
             printf(" \n");
             printf(" \n");
    }

}
  fclose(image);
  printf("Image successfully Received!\n");
  return 1;
  }

int main(int argc, char* argv[])
{
  int sockfd, newsockfd, clntaddrlen, portNo, idcount = 0, status;
  struct sockaddr_in servAddr, clntAddr;
  char msgbuffer[256], ipbuffer[256], id[20], sql_query[500], r_time_buffer[100], s_time_buffer[100],remail_id[100], semail_id[100], subject[50];
  char *conn_ip;
  char image_path[50];
  pid_t pid;

  //Port Number of the Server
  portNo = atoi(argv[1]);

  sqlite3 *db;
  int rc;
  char *Zerrmsg, *sql;

//opening Log database 
  rc = sqlite3_open("Log.db", &db);
  if(rc)
  {
    fprintf(stderr, "Database could not be opened Successfully: %s\n", sqlite3_errmsg(db));
    exit(1);
  }
  else
  {
    fprintf(stderr, "Database opened Successfully\n");
  }

  //socket creation at server side
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  clntaddrlen = sizeof(clntAddr);

  if(socket < 0)
    {
      printf("Socket Creation Failed\n");
      exit(1);
    }

  fprintf(stdout, "Socket Created\n");

  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(portNo);
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
      printf("Binding Failed\n");
      exit(1);
    }

  fprintf(stdout, "Binding Successful\n");

  if(listen(sockfd, QUEUE_SIZE) < 0)
    {
      printf("Listening Failed\n");
      exit(1);
    }

  fprintf(stdout, "I'm Listening\n");

  while(1)
    {
      newsockfd = accept(sockfd, (struct sockaddr*)&clntAddr, &clntaddrlen);

      if(newsockfd < 0)
       {
          printf("Accepting Failed\n");
           exit(1);
       }

       printf("Accepting Successful\n");

       //handling multiple clients
       pid = fork();
      if(pid == 0)
       {
            //converting connecting client's ip to string format
            conn_ip = inet_ntoa(clntAddr.sin_addr);
            
            //receiving the Connecting client's email id
            int e1=recv(newsockfd,semail_id,sizeof(semail_id), 0);
            semail_id[e1] = '\0';

            //query for updating the ip_address for the corresponding email_id

            strcpy(sql_query,"UPDATE email_ip SET IP = '");
            strcat(sql_query, conn_ip);
            strcat(sql_query, "' WHERE EMAIL_ID = '");
            strcat(sql_query, semail_id);
            strcat(sql_query, "';");

            rc = sqlite3_exec(db, sql_query, callback, 0, &Zerrmsg);
            if(rc != SQLITE_OK)
            {
              fprintf(stderr, "SQL Error: %s\n", Zerrmsg);
              sqlite3_free(Zerrmsg);
            }
            else
            {
              fprintf(stderr, "IP Address linked with Email Id updated Successfully\n");
            }


            //query for checking pending messages for pending client
            strcpy(sql_query,"SELECT MESSAGE, S_TIMESTAMP, SENDER_EMAIL, SUBJECT, IMAGEPATH FROM sender_receiver WHERE RECEIVER_EMAIL = ");
            strcat(sql_query, "'");
            strcat(sql_query, semail_id);
            strcat(sql_query, "' AND ");
            strcat(sql_query, "VALID = 0;");

            sql = sql_query;

            rc = sqlite3_exec(db, sql, callback, 0, &Zerrmsg);
            if(rc != SQLITE_OK)
            {
              fprintf(stderr, "SQL Error: %s\n", Zerrmsg);
              sqlite3_free(Zerrmsg);
            }
            else
            {
              fprintf(stderr, "Data Retrieved Successfully from SENDER_RECEIVER\n");
            }

            //sending the number of pending messages to the recipient
            send(newsockfd, (int*)&count_pending_msg, sizeof(int), 0);
            int i;
            printf("No. of pending messages: %d\n", count_pending_msg);

            //Updating valid bit for pending messages which will be sent
            for(i = 0; i < count_pending_msg; i++)
            {
              printf("%s\n", pending_msg[i]);
              strcpy(sql_query, "UPDATE sender_receiver SET VALID = 1 WHERE MESSAGE = '");
              strcat(sql_query, pending_msg[i]);
              strcat(sql_query, "';");
              sql = sql_query;
              rc = sqlite3_exec(db, sql, callback, 0, &Zerrmsg);
              if(rc != SQLITE_OK)
              {
                fprintf(stderr, "SQL Error: %s\n", Zerrmsg);
                sqlite3_free(Zerrmsg);
              }
             else
             {
                fprintf(stderr, "Data Updated Successfully in SENDER_RECEIVER\n");
              }
            }


            //sending the pending messages and the respective timestamp to the recipient
            for(i = 0; i < count_pending_msg; i++)
            {
              sleep(2);
              send_image(newsockfd, image_paths[i]);
              sleep(2);
              send(newsockfd, pending_msg[i], sizeof(pending_msg[i]), 0);
              sleep(2);
              send(newsockfd, email_id[i], sizeof(email_id[i]), 0);
              sleep(2);
              send(newsockfd, timestamp[i], sizeof(timestamp[i]), 0);
              sleep(2);
              send(newsockfd, pending_subject[i], sizeof(pending_subject[i]), 0);
              sleep(3);
            }


            //Receiving Image from client
            receive_image(newsockfd);
            sleep(2);

            //Receiving the Email_Id of the receiving client
            int n3 = recv(newsockfd, remail_id, sizeof(remail_id), 0);
            remail_id[n3] = '\0';
            send(newsockfd, remail_id, sizeof(remail_id), 0);

            //Receiving the Subject
            int n5 = recv(newsockfd, subject, sizeof(subject), 0);
            subject[n5] = '\0';
            send(newsockfd, subject, sizeof(subject), 0);

            //Receiving the Message from the sending client
            int n4 = recv(newsockfd, msgbuffer, sizeof(msgbuffer), 0);
            msgbuffer[n4] = '\0';
            send(newsockfd, msgbuffer, sizeof(msgbuffer), 0);


            //Receiving the Timestamp at which message was sent
            int n2 = recv(newsockfd, s_time_buffer, sizeof(s_time_buffer), 0);
            s_time_buffer[n2] = '\0';

            time_t curtime;
            time(&curtime);

            sprintf(r_time_buffer, "%s", ctime(&curtime));
            
            //query for inserting sender email,receiver email, and message
            strcpy(sql_query, "INSERT INTO sender_receiver(SENDER_EMAIL, S_TIMESTAMP, RECEIVER_EMAIL, R_TIMESTAMP, MESSAGE, SUBJECT, IMAGE, IMAGEPATH) VALUES(");
            strcat(sql_query, "'");
            strcat(sql_query, semail_id);
            strcat(sql_query, "'");
            strcat(sql_query, ", ");
            strcat(sql_query, "'");
            strcat(sql_query, s_time_buffer);
            strcat(sql_query, "'");
            strcat(sql_query, ", ");
            strcat(sql_query, "'");
            strcat(sql_query, remail_id);
            strcat(sql_query, "'");
            strcat(sql_query, ", ");
            strcat(sql_query, "'");
            strcat(sql_query, r_time_buffer);
            strcat(sql_query, "'");
            strcat(sql_query, ", ");
            strcat(sql_query, "'");
            strcat(sql_query, msgbuffer);
            strcat(sql_query, "'");
            strcat(sql_query, ", ");
            strcat(sql_query, "'");
            strcat(sql_query, subject);
            strcat(sql_query, "'");
            strcat(sql_query, ", ");
            strcat(sql_query, "'");
            strcat(sql_query, imagearray);
            strcat(sql_query, "', '");
            strcat(sql_query, path);
            strcat(sql_query, "'");
            strcat(sql_query, ");");
            sql = sql_query;

            //executing query
            rc = sqlite3_exec(db, sql, callback, 0,  &Zerrmsg);
            if(rc != SQLITE_OK)
            {
              fprintf(stderr, "SQL Error: %s\n", Zerrmsg);
              sqlite3_free(Zerrmsg);
            }
            else
            {
              fprintf(stderr, "Data inserted Successfully into RECEIVER\n");
            }
            close(newsockfd);
            exit(0);
       }
       else if(pid != 0)
       {
         wait(&status);
       }
    }
  sqlite3_close(db);
  close(sockfd);
  return 0;
}