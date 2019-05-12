Dette er et C program som var en skoleoppgave jeg gjorde tredje semester på
UiO. For å kompilere brukes makefilen. For å få mest forståelse av hva som skjer
i programmet anbefales det å kompilere slik "make debug". Deretter kjøres
server programmet i en terminal f.eks slik: "./server alice_short.job 1111".
Disse .job filene er input filer som ses på som jobbene til server, mens 1111
her er et eksempel på en port. Klient kan nå koble til serveren, hvis server
kjøres lokalt så kan man kjøre klient slik: "./klient 127.0.0.1 1111". Klient
vil nå kunne hente ut jobber fra serveren og utføre de, og når alle jobber er
hentet eller klient er ferdig så vil "connection" brytes og begge programmene
terminerer. 
