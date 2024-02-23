Dans la documentation du picoW, on trouve les explications pour créer un serveur web à l’aide de micropython.

Mais pour le langage C il n’existe que des exemples de client et de serveur tcp/ip pour recevoir et envoyer un simple fichier de données aléatoires.


A partir de l’exemple tcp_server, j’ai réussi à écrire un serveur web assez simple qui affiche un formulaire html et qui récupère en retour les informations du formulaire.


Après des heures de travail, des dizaines de tests et de nombreuses recherches sur internet sur l’utilisation de la librairie lwip et des fonctions tcp, le programme fonctionne correctement.
Le principal problème venait de la fonction tcp_write dont je croyais qu’il fallait indiquer la taille réelle des données à écrire. En fait, il faut d’abord initialiser le buffer à écrire avec des espaces puis le remplir avec les données à écrire puis le passer à la fonction tcp_write en indiquant la longueur totale du buffer donc la valeur BUF_SIZE et non la longueur réelle.
Le programme reprend les principales fonctions de l’exemple pour toutes les fonctions de création de la connexion wifi, d’ouverture du serveur et de l’attente de la connexion du client.


La fonction d’écriture des données est adaptée pour envoyer une entête html puis les données du formulaire.

La fonction de lecture est adaptée pour éliminer les requêtes inutiles et pour décoder les commandes transmises. A chaque appel de cette fonction, le formulaire est de nouveau renvoyé complété d’un message contenant la réponse. La fonction d’analyse est bien sur à adapter en fonction des commandes désirées. Un exemple d’appui sur un bouton est fourni dans le programme.


La fonction principale ne contient que les appels pour ouvrir la connexion wifi et créer le serveur web puis une simple boucle qui appelle la fonction de pooling. C’est elle qui déclenchera au travers du moteurs tcp de lwip, les fonctions d’écriture et de lecture. Rien n’est prévu pour arrêter cette boucle, cela reste à adapter.

Remarque : pour les tests, et pour permettre d’afficher les messages de contrôle, il est appelé la procédure de gestion du port usb. Le serveur web ne démarrera donc que si une connexion de type telnet ou putty est ouverte. Pour un fonctionnement autonome, il est donc nécessaire de commenter cette instruction.
