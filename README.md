Instrukcja Makefile:

•	„make” – kompiluje programy

•	„make testx” – uruchamia symulacje z danymi do odpowiedniego testu

•	„make user” – uruchamia symulacje z danymi do wprowadzenia przez użytkownika

•	„make clean” – czyszczenie


Opis projektu:

Przy nabrzeżu stoi statek o pojemności N pasażerów. Statek z lądem jest połączony mostkiem o pojemności K (K<N). Na statek próbują dostać się pasażerowie, z tym, że na statek nie może ich wejść więcej niż N, a wchodząc na statek na mostku nie może być ich równocześnie więcej niż K. Statek co określoną ilość czasu T1 (np.: jedną godzinę) wypływa w rejs. W momencie odpływania kapitan statku musi dopilnować aby na mostku nie było żadnego wchodzącego pasażera. Jednocześnie musi dopilnować by liczba pasażerów na statku nie przekroczyła N. Dodatkowo statek może odpłynąć przed czasem T1 w momencie otrzymania polecenia (sygnał1) od kapitana portu. Rejs trwa określoną ilość czasu równą T2. Po dotarciu do portu pasażerowie opuszczają statek. Po opuszczeniu statku przez ostatniego pasażera, kolejni pasażerowie próbują dostać się na pokład (mostek jest na tyle wąski, że w danym momencie ruch może odbywać się tylko w jedną stronę). Statek może wykonać maksymalnie R rejsów w danym dniu lub przerwać ich wykonywanie po otrzymaniu polecenia (sygnał2) od kapitana portu (jeżeli to polecenie nastąpi podczas załadunku, statek nie wypływa w rejs, a pasażerowie opuszczają statek. Jeżeli polecenie dotrze do kapitana w trakcie rejsu statek kończy bieżący rejs normalnie).
