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

typedef struct s_server {
  int my_socket, socket_2; //descripteurs de sockets
  uint socket_len; //entier non signé pour la taille d'une structure
  struct sockaddr_in serv_addr, cli_addr; //structure d'adresse qui contiendra les paramètres réseaux recepteur/expediteur
  fd_set fds; //ensbl de descripteurs en lecture à monitorer avec select
  int t[FD_SETSIZE]; //tableau de l'ensemble des descripteurs
  int taille;// = 0; //nombre de descripteurs dans le tableau
  int sockmax;// = 0;
  int		i, j, k;				//variables pour les boucles
  char	buf[1024];			//espace necessaire pour stocker un message reçu
  char	*msg_ac0;//[1024] = "Bienvenue sur ce serveur de messagerie instantanée";
  char	*msg_ac1;
  char  *msg_ac2;//[1024] = "Veuillez saisir un nouvel identifiant avant de continuer";
  char   choix[2];
  struct s_customer *customer; //pointeur vers la structure qui contiendra les informations clients
} t_server;

typedef struct s_customer {
  char  name[10];  //nom d'un client
  /*int   x;
  char  *t_name;*/
} t_customer;

void init_serveur(t_server *serv) {
  memset(serv->buf,'\0',1024);        // initialisation du buffer qui sera utilisé
  my_putstr("Paramétrage réseau en cours, veuillez patienter...\n");
  serv->socket_len = sizeof(struct sockaddr_in); //taille d'une structure sockaddr_in
  //Création de la socket réseau
  serv->my_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(serv->my_socket == -1) 
    {
      my_putstr("Error opening socket \n");
      exit(1);
    }
  //Paramétrage de la struc d'adresse du serveur -> serv_addr
  bzero(&serv->serv_addr, sizeof(serv->serv_addr));
  serv->serv_addr.sin_port = htons(12345);  // on set notre port d'ecoute
  serv->serv_addr.sin_family = AF_INET;  // on set le type de la socket
  serv->serv_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); // On spécifie l'écoute pour toutes les requêtes
  //Association de la socket et des paramètres réseaux du serveur
  if ((bind(serv->my_socket, (struct sockaddr *) &serv->serv_addr, sizeof(serv->serv_addr))) < 0) 
    {
      my_putstr("Error bind \n");
      close(serv->my_socket);
      exit(1);
    } // on bind notre socket au port et a l'interface, et on verifie bien le retour
  my_putstr("Paramétrage terminé, le serveur est en route \n");
}

void con_customer(t_server *serv) {
  serv->customer = malloc(sizeof(t_server));
  FD_ZERO(&serv->fds); //Il faut vider l'ensbl des descripteurs à chaque recommencement de la boucle
  FD_SET(serv->my_socket, &serv->fds); //Ici on ajoute le descripteur my_socket à l'ensbl fds
  for(serv->j=0; serv->j < serv->taille; serv->j++) 
    {
      if(serv->t[serv->j] != 0){
        FD_SET(serv->t[serv->j],&serv->fds);
      if(serv->sockmax < serv->t[serv->j])
        serv->sockmax = serv->t[serv->j]; } //Ici on prend le "numero" de socket maximal pour la fonction select
    }
  if(select(serv->sockmax+1, &serv->fds, NULL, NULL, NULL) == -1) 
    {
      my_putstr("Erreur lors de l'appel a select");
      exit (1);
    } //On utilise le select sur toutes les sockets y compris celle d'écoute
  if(FD_ISSET(serv->my_socket, &serv->fds)) 
    {
      serv->socket_2 = accept(serv->my_socket,(struct sockaddr *) &serv->cli_addr, &serv->socket_len);
      if (serv->socket_2 < 0)
        my_putstr("cannot accept\n");
      else
        my_putstr("Con %s\n", inet_ntoa(serv->cli_addr.sin_addr)); //Connexion avec le client
    } //Si la socket d'écoute est dans le fds -> Il a reçu quelque chose (connection d'un client)
    serv->taille++; //Qu'on ajoute au tableau des descripteurs
    serv->t[serv->taille -1] = serv->socket_2;
}

void cust_registration(t_server *cust) {
  memset(cust->customer->name,'\0', 10);
  //cust->customer->t_name = malloc(sizeof(char));
 // for(cust->k = 1; cust->k < cust->taille; cust->k++) { //On renvoie la chaine à tous les clients
    if(send(cust->socket_2, cust->msg_ac0, 1024, 0) == -1) 
      {
        my_putstr("Erreur lors de l'appel a send");
        exit(1);
      }
    if(send(cust->socket_2, cust->msg_ac1, 1024, 0) == -1) 
      {
        my_putstr("Erreur lors de l'appel a send");
        exit(1);
      }
    if(FD_ISSET(cust->my_socket, &cust->fds)) 
      { //Si une socket s'y trouve, alors un client a envoyer quelque chose
        if(recv(cust->t[cust->taille -1], &cust->customer->name, 10, 0) == -1) 
          {
            my_putstr("Erreur lors de la reception du message");
            exit(4);
          } //On stocke le message
        my_putstr("Ce client se nomme: %s", cust->customer->name);
       /* cust->customer->t_name[cust->customer->x] = cust->customer->name;
        cust->customer->x++; */
      }
    if(send(cust->socket_2, cust->msg_ac2, 1024, 0) == -1) 
      {
        my_putstr("Erreur lors de l'appel a send");
        exit(1);
      }
    memset(cust->choix,'\0', 2);
    if(FD_ISSET(cust->my_socket, &cust->fds)) 
      { //Si une socket s'y trouve, alors un client a envoyer quelque chose
        if(recv(cust->t[cust->taille -1], &cust->choix, 2, 0) == -1) 
          {
            my_putstr("Erreur lors de la reception du message");
            exit(4);
          } //On stocke le message
        my_putstr("%s a choisi l'option %s", cust->customer->name, cust->choix);
      }
} 
void gen_chat(t_server *serv) {
  memset(serv->buf,'\0',1024);
  for(serv->i = 1; serv->i < serv->taille; serv->i++) { //On parcours tous les autres descripteurs du tableau
    if(FD_ISSET(serv->t[serv->i], &serv->fds)) { //Si une socket s'y trouve, alors un client a envoyer quelque chose
      if(recv(serv->t[serv->i], &serv->buf, 1024, 0) == -1) {
        my_putstr("Erreur lors de la reception du message");
        exit(4);
      } //On stocke le message
      my_putstr(" %s\n", serv->buf);
      for(serv->k = 1; serv->k < serv->taille -1; serv->k++) { //On renvoie la chaine à tous les clients
        if(serv->k != serv->i) {
          if(send(serv->t[serv->k], &serv->buf, 1024, 0) == -1) {
            my_putstr("Erreur lors de l'appel a send");
            exit(1);
          }
        }
      }
    }
  }
}




int main(/*int argc, char *argv[]*/) {
  t_server serveur;
  serveur.taille = 0;
  serveur.sockmax = 0;
  //serveur.customer->x = 0;
  serveur.msg_ac0 = strdup("Bienvenue sur ce serveur de messagerie instantanée");
  serveur.msg_ac1 = strdup("Veuillez saisir un nouvel identifiant avant de continuer s'il vous plait:");
  serveur.msg_ac2 = strdup("Bien maintenant, vous souhaitez:\n 1)Acceder au chat général\n 2)Voir les autres membres connectés\n 3)Voir les channels de discussion\n 4)Quitter le chat\n (veuillez choisir une valeur entre 1 et 4)");
  init_serveur(&serveur);
  listen(serveur.my_socket, 5); //On met le serveur en écoute avec un buffer de 5
  my_putstr("Le serveur est en attente d'une connexion... \n");
  serveur.sockmax = 0;
  serveur.t[0] = serveur.my_socket;
  serveur.taille++;
  while (1) {
    con_customer(&serveur);
    cust_registration(&serveur);
    gen_chat(&serveur);
  }
}
