# Zadania - Zestaw 5
## Potoki nazwane i nienazwane
### Zadanie 1
Należy napisać interpreter poleceń obsługujący operator pipe - "|". Interpreter czyta kolejne linie z wejścia standardowego. Każda linia ma format:

`prog1 arg1 ... argn1 | prog2 arg1 ... argn2 | ... | progN arg1 ... argnN`

Dla takiej linii interpreter powinien uruchomić wszystkie N poleceń w osobnych procesach, zapewniając przy użyciu potoków nienazwanych oraz funkcji dup2, by wyjście standardowe procesu k było przekierowane do wejścia standardowego procesu (k+1). Można założyć ograniczenie górne na ilość obsługiwanych argumentów (co najmniej 3) oraz ilość połączonych komend w pojedynczym poleceniu (co najmniej 20). Interpreter powinien umożliwiać uruchamianie programów znajdujących się w katalogu na liście w zmiennej PATH bez podawania pełnej ścieżki. Po uruchomieniu ciągu programów składających się na pojeczyne polecenie (linijkę) interpreter powinien oczekiwać na zakończenie wszystkich tych programów.

Uwaga: należy użyć pipe/fork/exec, nie popen

### Zadanie 2
W ramach tego zadania stworzony zostanie wizualizator zbioru Mandelbrota. Zbiór ten składa się z liczb zespolonych c takich, że ciąg {zn} zdefiniowany poprzez:

    z0 = 0, zn+1 = fc(zn),      gdzie fc(z) = z2 

ma moduł ograniczony przez 2, tj. |zn| <= 2 dla każdego n. W praktyce wizualizacja polega na tym, że dla zadanego punktu c sprawdzamy, czy po pewnej ustalonej ilości iteracji K ten warunek jest spełniony i kolorujemy punkt c na podstawie tego, po ilu iteracjach warunek przestaje być prawdą - nazwijmy tę liczbę iters( c ). Jeśli np. K = 100 i dla danego c dopiero w 30 iteracji dostajemy zn takie, że |zn| > 2, to iters( c ) = 30. Jeśli po K iteracjach warunek pozostaje spełniony, to przyjmujemy iters( c ) = K.

Wizualizację będziemy wykonywać dla punktów w prostokątnym obszarze D = { c: -2 < Re( c ) < 1, -1 < Im( c ) < 1 }

Należy napisać dwa programy - master oraz slave - które będą komunikować się poprzez potok nazwany (kolejkę FIFO), do której ścieżkę będą dostawać jako argument wywołania. Do potoku pisać będzie wiele procesów wykonujących program slave, a czytał będzie z niej jeden proces master.

**Slave**
Slave przyjmuje trzy argumenty - ścieżkę do potoku nazwanego i liczby całkowite N oraz K. Wykonuje następujące akcje:
* otwiera potok nazwany
* generuje N losowych punktów w obszarze D, dla każdego punktu c oblicza wartość iters(c) oraz zapisuje do potoku jedną linijkę zawierającą część rzeczywistą oraz urojoną punktu c oraz obliczoną wartość iters(c)
* zamyka potok nazwany

**Master**
Master przyjmuje dwa argumenty - ścieżkę do potoku nazwanego i liczbę całkowitą R. Tworzy on potok o podanej nazwie. Master przechowuje zainicjalizowaną zerami tablicę dwuwymiarową T o rozmiarze R x R reprezentującą obraz obszaru D - każdy element tablicy to jeden "piksel" i odpowiada w naturalny sposób pewnemu kawałkowi obszaru D.

Master czyta z potoku nazwanego kolejne linijki aż do momentu gdy zostanie on zamknięty przez wszystkie procesy do niego piszące. Każda linijka to informacja o punkcie c = x + iy oraz wartości iters(c). Dla każdej oczytanej linijki master zapisuje w tablicy T wartość iters(c) w miejscu odpowiadającym pikselowi reprezentującemu kawałek D w którym leży punkt c.

Po zakończeniu czytania master zapisuje zawartość tablicy T do pliku o nazwie data tak, że w każdej linijce znajdują się oddzielone białymi znakami wartości indeksów i, j oraz wartość T[i][j], w kolejności takiej jak w poniższym przykładzie:
```shell
0 0 10
0 1 35
0 2 11
1 0 5
1 1 77
```
itd. 

Następnie master uruchamia program Gnuplot przy użyciu funkcji popen, i przekazuje na jego wejście standardowe następujące polecenia:
```shell
set view map
set xrange [0:X]
set yrange [0:X]
plot 'data' with image
```
gdzie za X wstawiona ma być wartość argumentu R.