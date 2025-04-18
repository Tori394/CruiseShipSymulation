Instrukcja Makefile:

•	„make” – kompiluje programy

•	„make testx” – uruchamia symulacje z danymi do odpowiedniego testu

•	„make user” – uruchamia symulacje z danymi do wprowadzenia przez użytkownika

•	„make clean” – czyszczenie


Streszczenie PL/ENG:

Przy nabrzeżu stoi statek o pojemności N pasażerów. Statek z lądem jest połączony mostkiem o pojemności K (K<N). Na statek próbują dostać się pasażerowie, z tym, że na statek nie może ich wejść więcej niż N, a wchodząc na statek na mostku nie może być ich równocześnie więcej niż K. Statek co określoną ilość czasu T1 (np.: jedną godzinę) wypływa w rejs. W momencie odpływania kapitan statku musi dopilnować aby na mostku nie było żadnego wchodzącego pasażera. Jednocześnie musi dopilnować by liczba pasażerów na statku nie przekroczyła N. Dodatkowo statek może odpłynąć przed czasem T1 w momencie otrzymania polecenia (sygnał1) od kapitana portu. Rejs trwa określoną ilość czasu równą T2. Po dotarciu do portu pasażerowie opuszczają statek. Po opuszczeniu statku przez ostatniego pasażera, kolejni pasażerowie próbują dostać się na pokład (mostek jest na tyle wąski, że w danym momencie ruch może odbywać się tylko w jedną stronę). Statek może wykonać maksymalnie R rejsów w danym dniu lub przerwać ich wykonywanie po otrzymaniu polecenia (sygnał2) od kapitana portu (jeżeli to polecenie nastąpi podczas załadunku, statek nie wypływa w rejs, a pasażerowie opuszczają statek. Jeżeli polecenie dotrze do kapitana w trakcie rejsu statek kończy bieżący rejs normalnie).

By the dock stands a ship with a capacity of N passengers. The ship is connected to the land by a bridge with a capacity of K (K < N). Passengers are trying to board the ship, but no more than N passengers can board, and at any given time, no more than K passengers can be on the bridge. The ship sets sail after a certain amount of time, T1 (e.g., one hour). At the time of departure, the captain must ensure that no passengers are on the bridge. He must also ensure that the number of passengers on the ship does not exceed N. Additionally, the ship may depart before the time T1 upon receiving an order (signal1) from the port captain. The voyage lasts a specific amount of time, T2. Upon reaching the port, the passengers disembark. After the last passenger leaves the ship, other passengers attempt to board (since the bridge is narrow, movement can only happen in one direction at a time). The ship can make a maximum of R voyages in one day or stop its voyages after receiving an order (signal2) from the port captain. If this order occurs during the boarding process, the ship does not set sail, and the passengers disembark. If the order reaches the captain during the voyage, the ship completes the current voyage normally.


Pełen raport:

![Dokumentacja PDF](https://github.com/Tori394/CruiseShipSymulation/blob/main/Raport.pdf)
