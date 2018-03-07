# Zadania - zestaw 1
## Zarządzanie pamięcią, biblioteki, pomiar czasu 
### Zadanie 1. Ksiazka adresowa
Zaprojektuj i przygotuj zestaw funkcji (bibliotekę) do zarzadzania "ksiazka kontaktowa" przechowujaca kontakty zawierajace:  Imię, Nazwisko, Date urodzenia, email, telefon, adres w postaci

a) drzewa binarnego

b) listy dwukierunkowej

Biblioteka powinna umozliwiac: 

- tworzenie i usuwanie ksiazki kontaktowej 

- dodanie i usuniecie  kontaktu do/z ksiazki (bez utraty innych kontaktow i bez wyciekow pamieci) 

- wyszukiwanie elementu w ksiazce

- sortowanie/przebudowanie ksiazki wg wybranego pola (Nazwisko, Data urodzenia, email, telefon)

Przygotuj plik Makefile, zawierający polecenia kompilujące pliki źródłowe biblioteki oraz tworzące biblioteki w dwóch wersjach: statyczną i dzieloną.

### Zadanie 2. Program korzystający z biblioteki
Napisz program testujący działanie funkcji z biblioteki z zadania 1.

Program powinien stworzyc ksiazki adresowe skladajace sie z 1000 elementow a realizowane w postaci listy dwukierunkowej oraz drzewa binarnego. Dane możesz wygenerowac chocby na stronkach typu generatedata.com albo uzyc danych losowych.

W programie zmierz i wypisz czasy realizacji podstawowych operacji:

- stworzenie ksiazki adresowej,

- dodanie pojedynczego elementu (czasy mierz dla dodawania po kolei kazdego elementu)

- usuwanie kontaktu z ksiazki (zmierz czasy dla przypadku optymistycznego oraz pesymistycznego)

- wyszukanie elementu w ksiazce (zmierz czasy dla przypadku optymistycznego oraz pesymistycznego)

- przesortowanie/przebudowanie ksiazki

Mierzac czasy pokaz trzy wartosci: czas rzeczywisty, czas uzytkownika i czas systemowy.

### Zadanie 3. Testy i pomiary
a) Przygotuj plik Makefile, zawierający polecenia kompilujące program z zad 2 na trzy sposoby:
- z wykorzystaniem bibliotek statycznych,
- z wykorzystaniem bibliotek dzielonych (dynamiczne, ładowane przy uruchomieniu programu),
- z wykorzystaniem bibliotek ładowanych dynamicznie (dynamiczne, ładowane przez program),
oraz uruchamiający testy.

Wyniki pomiarow zbierz w pliku results.txt. Plik zalacz jako element rozwiazania.

b) Rozszerz plik Makefile z punktu 3a) dodajac mozliwosc skompilowania programu na roznych poziomach optymalizacji -O0...-Os. Przeprowadz ponownie pomiary kompilujac i uruchamiajac program na roznych poziomach iotymalizacji.

Wyniki pomiarow dodaj do pliku results.txt. Otrzymane wyniki krotko skomentuj.

### Zadanie 4. Analiza pliku core
Zmodyfikuj program z punktu 2 tak aby wygenerowany został zrzut pamieci (plik core). Korzystajac z gdb oraz zrzutu pamieci znajdz w programie miejsce i stan programu powodujace blad krytyczny i zrzut pamieci. Do oddawanego zestawu dodaj plik tekstowy gdb.txt a w nim: wskaz w jaki sposob zmodyfikowales program z punktu 2 i jaka operacja powoduje blad i zrzut pamieci (ew jakie kroki podczas uruchamiania programu należy wykonac aby blad i zrzut wystapily) a także opisz w jaki sposob posluzyles się/nalezy posluzyc sie gdb do analizy pliku core oraz wskaz/zalacz te elementy zrzuconego obrazu pamieci które potwierdzaja fakt oraz okolicznosci wystapienia (prefabrykowanego) bledu.

Uwaga. Jesli program z p.2 zrzuca cora – nie zostanie to zaliczone jako rozwiazanie punktu 4 :)