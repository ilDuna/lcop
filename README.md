# LCOP
*Lidar COntrol Protocol*

Implementazione delle funzioni di base (serializzazione / deserializzazione) per lo scambio di messaggi tramite il protocollo LCOP, progettato per il controllo da remoto di sensori LIDAR.  
Permette di creare e gestire una coda FIFO di messaggi lcop.  
Contiene anche il codice da eseguire sul server (*server.c*) e sul sensore LIDAR (*lidar.c*).

Richiede la libreria **rocket** ([link][rocket]) per stabilire un canale di comunicazione persistente e network-fault tolerant.

[rocket]: <https://github.com/ilDuna/rocket>
