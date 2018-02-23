#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

void  my_putchar(char c)
{
  write(1, &c, 1);
}

void    my_putstr(char *str)
{
  while (*str)
  {
    my_putchar(*str);
    str++;
  }
}

typedef struct s_customer {
  int my_socket; //descripteurs de sockets
  uint socket_len; //entier non signé pour la taille d'une structure
  struct sockaddr_in serv_addr; //structure d'adresse qui contiendra les paramètres réseaux recepteur/expediteur
  fd_set fds; //ensbl de descripteurs en lecture à monitorer avec select
  char  buf[1024];      //espace necessaire pour stocker un message reçu
  char  buf1[1024];      //espace necessaire pour stocker un message envoyé
  char  buf2[1024];
  char  name[10];
  char   choix[2];
} t_customer;

void init_customer(t_customer *cust) {
  memset(cust->buf,'\0',1024);        // initialisation du buffer qui sera utilisé
  memset(cust->buf1,'\0',1024);        // initialisation du buffer qui sera utilisé
  my_putstr("Paramétrage réseau en cours, veuillez patienter...\n");
  //Création de la socket réseau
  cust->my_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(cust->my_socket == -1) 
    {
      my_putstr("Error opening socket \n");
      exit(1);
    }
  //Paramétrage de la struc d'adresse du serveur -> serv_addr
  bzero(&cust->serv_addr, sizeof(cust->serv_addr));
  cust->serv_addr.sin_port = htons(12345);  // on set notre port d'ecoute
  cust->serv_addr.sin_family = AF_INET;  // on set le type de la socket
  cust->serv_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); // On spécifie l'écoute pour toutes les requêtes
  my_putstr("tentative de connection...\n");
  if(connect(cust->my_socket, (struct sockaddr *) &cust->serv_addr, sizeof(cust->serv_addr)) == -1) {
    my_putstr("Erreur de connection");
    exit(2);
  }
  my_putstr("connection etablie\n");
}

void cust_registration(t_customer *cust) {
  FD_ZERO(&cust->fds);
  FD_SET(cust->my_socket, &cust->fds);
  if(select(cust->my_socket+1, &cust->fds, NULL, NULL, NULL) == -1) {
    my_putstr("Erreur lors de l'appel a select");
    exit(1);
  }
  if(FD_ISSET(cust->my_socket, &cust->fds)) {
    if(recv(cust->my_socket, &cust->buf, 1024, 0) == -1) {
      my_putstr("erreur lors de la reception");
      exit(1);
    }
    my_putstr("%s\n", cust->buf);
  }
  memset(cust->buf,'\0',1024);
  if(FD_ISSET(cust->my_socket, &cust->fds)) {
    if(recv(cust->my_socket, &cust->buf, 1024, 0) == -1) {
      my_putstr("erreur lors de la reception");
      exit(1);
    }
    my_putstr("%s\n", cust->buf);
  }
  memset(cust->name,'\0',10);
  FD_SET(0, &cust->fds);
  if(FD_ISSET(0,&cust->fds)) { // si l'entree standard est dans fds, alors l'utilisateur en en train de rédiger un message a envoyer
    if(read(0,cust->name,10) == -1) {  // on lit donc ce qui arrive sur l'entrée standard
      my_putstr("Erreur lors de l'appel a read -> ");
      exit(1);
    }
    if(send(cust->my_socket,cust->name,10,0) == -1) {   // puis on l'envoie au serveur
      my_putstr("Erreur lors de l'appel a send");
      exit(1);
    }
  }
  if(FD_ISSET(cust->my_socket, &cust->fds)) {
    if(recv(cust->my_socket, &cust->buf, 1024, 0) == -1) {
      my_putstr("erreur lors de la reception");
      exit(1);
    }
    my_putstr("%s\n", cust->buf);
  }
  //FD_SET(0, &cust->fds);
  if(read(0,cust->choix,2) == -1) {  // on lit donc ce qui arrive sur l'entrée standard
    my_putstr("Erreur lors de l'appel a read -> ");
    exit(1);
  }
  if(send(cust->my_socket,cust->choix,2,0) == -1) {   // puis on l'envoie au serveur
    my_putstr("Erreur lors de l'appel a send");
    exit(1);
  }
}

void en_gen_chat(t_customer *cust) {
  my_putstr("debut chat \n");
  memset(cust->buf2,'\0',1024);        // initialisation du buffer qui sera utilisé
  memset(cust->buf1,'\0',1024);        
  /*if(select(cust->my_socket+1, &cust->fds, NULL, NULL, NULL) == -1) {
      my_putstr("erreur lors de l'appel à select");
      exit(1);
    }*/
  if(FD_ISSET(0,&cust->fds)) { // si l'entree standard est dans fds, alors l'utilisateur en en train de rédiger un message a envoyer
    if(read(0,cust->buf1,1024) == -1) {  // on lit donc ce qui arrive sur l'entrée standard
      my_putstr("Erreur lors de l'appel a read -> ");
      exit(1);
    }
    if(send(cust->my_socket,cust->buf1,1024,0) == -1) {   // puis on l'envoie au serveur
      my_putstr("Erreur lors de l'appel a send");
      exit(1);
    }
  }
  if(FD_ISSET(cust->my_socket, &cust->fds)) {//si la socket de communication est dans fds, alors le serveur nous a envo$
    if(recv(cust->my_socket, &cust->buf2, 1024, 0) == -1) {
      my_putstr("Erreur lors de la reception");
      exit(4);
    }
      my_putstr("la que se trouve :%s\n", cust->buf2);    // on l'affiche
  }
}

int main(/*int argc, char *argv[]*/) {
  t_customer customer;
  init_customer(&customer);
  cust_registration(&customer);
  while(1) {
    en_gen_chat(&customer);
  }
  close(my_socket);
}
