#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include <stdio.h>
#include "pico/stdio_usb.h"
#include "tusb.h"

#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define TCP_PORT 80          // 
#define DEBUG_printf printf
#define BUF_SIZE 2048
#define POLL_TIME_S 5
//#define PICO_CYW43_ARCH_POLL 1

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool complete;
    uint8_t buffer_sent[BUF_SIZE];
    uint8_t buffer_recv[BUF_SIZE];
    int sent_len;
    int recv_len;
    int run_count;
} TCP_SERVER_T;

// initial messages
char message1[80]="Entrez une commande (help pour la liste).        ";
char * listecommande="ledon : allumer la led. <br> ledoff : eteindre la led. <br>";
/*****************************************************/
/*    initialisation structure parametre             */
/*****************************************************/
static TCP_SERVER_T* tcp_server_init(void) {
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return NULL;
    }
    return state;
}

/*****************************************************/
/*    server close                                   */
/*****************************************************/
static err_t tcp_server_close(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != NULL) {
        tcp_arg(state->client_pcb, NULL);
        tcp_poll(state->client_pcb, NULL, 0);
        tcp_sent(state->client_pcb, NULL);
        tcp_recv(state->client_pcb, NULL);
        tcp_err(state->client_pcb, NULL);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = NULL;
    }
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
    return err;
}
/*****************************************************/
/*    error test                                     */
/*****************************************************/
static err_t tcp_server_result(void *arg, int status) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (status == 0) {
        DEBUG_printf("No error\n");
    } else {
        DEBUG_printf("Error : %d\n", status);
    }
    state->complete = true;
    return ERR_OK;
}
/*****************************************************/
/*    error test                                     */
/*****************************************************/
static void tcp_server_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err_fn %d\n", err);
        tcp_server_result(arg, err);
    }
}
/*****************************************************/
/*    send data to client                            */
/*****************************************************/
err_t tcp_server_send_data(void *arg, struct tcp_pcb *tpcb)
{
    // html page description
    char * ligne1= " <!DOCTYPE html> <html> <body>Bienvenue sur le Pico W. <br>   <form action=\"./commande\">";
    char * ligne2= " <br> Commande : <br><input type=\"Commande\" id=\"commande\" name=\"com\" /> <br>  </form><br>";
    char * ligne3="<br><form action=\"./stop\"><input type=\"submit\" value=\"STOP\" /></form></body></html>\r\n\r\n\0";
    // html header
    char * entete="HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n\0";
    int tailleent=strlen(entete);
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    // init buffer with spaces
    for(int i=0; i< BUF_SIZE; i++) {
        state->buffer_sent[i] = ' ';
    }
    //copy header 
       for(int i=0; i< tailleent; i++) {
        state->buffer_sent[i] = entete[i];
    }
    DEBUG_printf("Writing %ld bytes to client\n", tailleent);
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    err_t err = tcp_write(tpcb, state->buffer_sent, BUF_SIZE, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to write data %d\n", err);
        return tcp_server_result(arg, -1);
    }
    
    // init buffer with spaces
    for(int i=0; i< BUF_SIZE; i++) {
        state->buffer_sent[i] = ' ';
    }
    // copy html page  description
    int tailletot=0;
    int taille1=strlen(ligne1);
    for(int i=0; i< taille1; i++) {
        state->buffer_sent[i] = ligne1[i];
    }
    tailletot=tailletot+taille1;
    int taille2=strlen(ligne2);
    for(int i=0; i< taille2; i++) {
        state->buffer_sent[tailletot+i] = ligne2[i];
    }
    tailletot=tailletot+taille2;
    int taillemess=strlen(message1);
  
    // add variable message
    for(int i=0; i< taillemess; i++) {
        state->buffer_sent[tailletot+i] = message1[i];
    }
    tailletot=tailletot+taillemess;
    int taille3=strlen(ligne3);
    for(int i=0; i< taille3; i++) {
        state->buffer_sent[tailletot+i] = ligne3[i];
    }
    tailletot=tailletot+taille3;
    DEBUG_printf("Taille message= %ld bytes \n", tailletot);
    //DEBUG_printf("message = %s\n",state->buffer_sent);
    state->sent_len = 0;
    DEBUG_printf("Writing %ld bytes to client\n", BUF_SIZE);
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
     err = tcp_write(tpcb, state->buffer_sent, BUF_SIZE, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        DEBUG_printf("Failed to write data %d\n", err);
        return tcp_server_result(arg, -1);
    }
    return ERR_OK;
}
/*****************************************************/
/*    callback function send                         */
/*****************************************************/
static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    DEBUG_printf("tcp_server_sent %u \n", len);
    return ERR_OK;
}
/*****************************************************/
/*    command analyser                               */
/*****************************************************/
void analyseCommande(void *arg, struct tcp_pcb *tpcb,char *commande) {
    err_t  erreur;
    char extcom[40];
     DEBUG_printf("Debut >>>%s<<<\n",commande);
     if (commande[4]=='/' && commande[5]!=' ') {
        int i=5,j=0;
        while (commande[i]!=' ') {
           extcom[j]=commande[i];
           i++;
           j++;
        }
        extcom[j]=0;
        if (strcmp(extcom,"stop?")==0){
           strcpy(message1,"Appui bouton OK");
        }
        else {
            i=18,j=0;
            while (commande[i]!=' ') {
               extcom[j]=commande[i];
               i++;
               j++;
            }
            extcom[j]=0;
           if (strcmp(extcom,"help")==0){
              strcpy(message1,listecommande);
           }
           else
           if (strcmp(extcom,"ledon")==0) {
              printf("Led On\n");
              cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
           }
           else
           if (strcmp(extcom,"ledoff")==0) {
              cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
           }
          // add here other command
           else {
               strcpy(message1,"Commande inconnue (help pour la liste).");
           }
        }
     }
     return;
}
/*****************************************************/
/*    callback function reception                    */
/*****************************************************/
err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    DEBUG_printf("***************Appel recv *************\n");
     if (p!=NULL) {  
       DEBUG_printf("Packet taille= %d \n",p->tot_len);
       if (p->tot_len > 0) {
           char debut[80];
  
         memcpy(debut,p->payload,16);
         debut[16] =0;
         if (strcmp(debut,"GET /favicon.ico") !=0 ) {    // modify if request post
             memcpy(debut,p->payload,78);
             debut[79] =0;
             analyseCommande(arg, tpcb,debut);
         }
       }
       pbuf_free(p);
     }
     // write html page
     err_t  erreur =tcp_server_send_data(arg, tpcb);
 
    DEBUG_printf("********Fin Appel recv *************\n");
    return ERR_OK;
} 
/*****************************************************/
/*    callback function poll                    */
/*****************************************************/ 
static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb) {
    //TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    //DEBUG_printf("tcp_server_poll_fn \n");
    return ERR_OK;
}
  
/*****************************************************/
/*    callback function server accept                 */
/*****************************************************/
static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("Failure in accept %d %d\n",err,client_pcb); 
        tcp_server_result(arg, err);
        return ERR_VAL;
    }
    DEBUG_printf("Client connected\n");

    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);
    
   // err_t erreur =tcp_server_send_data(arg, state->client_pcb);
   // sleep_ms(1000);
    
    return ERR_OK; 
}
/*****************************************************/
/*     function open server                          */
/*****************************************************/
static bool tcp_server_open(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    DEBUG_printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err) {
        DEBUG_printf("failed to bind to port %u\n", TCP_PORT);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}
/*****************************************************/
/*    principal function                             */
/*****************************************************/
int main() {
    stdio_init_all();
    // comment this instruction for work not usb connection
    while (!tud_cdc_connected())  sleep_ms(100);  // boucle attente connexion usb pour message
    // init wifi driver
    if (cyw43_arch_init()) {          // initalize cyw43_driver code and initializes the lwIP stack 
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
    };

   
    // init structure paraméter
    TCP_SERVER_T *state = tcp_server_init();
    if (!state) {
        return 1;
    }
    // server open
    if (!tcp_server_open(state)) {
        tcp_server_result(state, -1);
        return 1;
    }
    // loop never end 
    while (true) {
      cyw43_arch_poll();
      sleep_ms(100);
    }
    
    free(state);
    
    cyw43_arch_deinit();
    return 0;
}
