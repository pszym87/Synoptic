## Podsumowanie
Synoptic to projekt systemu wbudowanego z użyciem mikrokontrolera **STM32** NUCLEO64-F476RG, który komunikuje się z peryferiami: wyświetlaczem TFT (ST7735S), pamięcią EEPROM 24LC01, konwerterem UART-USB PL2303 i czujnikiem ciśnienia atmosferycznego LPS25HB.

Projekt obejmuje: fizyczne połączenie komponentów na płytce stykowej, **oprogramowanie w C niskopoziomowych bibliotek** dla urządzeń LPS26HB, 24LC01, ST7735S, oprogramowanie w C funkcjonalności wysokopoziomowej: 

- debuggowanie urządzenia po konsoli portu szeregowego
- tryb pomiarów w czasie rzeczywistym z prezentacją odczytów na wyświetlaczu
- zapis i odczyt historii pomiarów z timestampem
- ustawienie alarmu dla urządzenia, który powoduje auto-zapis pomiaru do historii

Mikrokontroler komunikuje się z urządzeniami za pomocą protokołów **I2C** (moduł pamięci EEPROM 24LC01 i czujnik ciśnienia LPS25HB współdzieli linie SDA i SCL), **SPI** (wyświetlacz) i **UART**.

**Schemat połączeń**
![schemat](https://user-images.githubusercontent.com/106841261/179467229-f1d3b543-00c1-4221-a203-384294190a52.png)

## Funkcjonalność wysokopoziomowa

Podprogram 1: ‘Live mode’

W trybie rzeczywistym sczytuje z czujnika LPS25HB temperaturę i ciśnienie atmosferyczne. Program wylicza wysokość w m.n.p.m. Dane prezentuje na wyświetlaczu graficznym w formie obrazka i tekstu.

Podprogram 2: ‘History mode’

Podprogram łączy się z pamięcią EEPROM odczytuje historie zapisanych pomiarów i wyświetla na wyświetlaczu. 

Pozostałe tryby pracy

Niezależnie od wybranego podprogramu system w trybie przerwaniowym uruchamia **alarm**, który powoduje zapis danych pomiarowych i czasu z **zegara RTC** do pamięci EEPROM. 

Stworzony prosty debugger i sterowanie urządzeniem odbywa się po konsoli wirtualnego portu szeregowego (np. Putty, Screen). 

Debugging

Funkcjonalność debuggingu sprowadza się do:

1. zrzucenia wartości flag przy uruchomieniu urządzenia:

- sprawdzenie **flagi watchdoga** w celu zidentyfikowania powodu restartu urządzenia
- sprawdzenie poprawności działania peryferiów (testowe odczyty z urządzeń peryferyjnych)
2. Wystawienia szeregu instrukcji do sterowania i debuggowania urządzenia i peryferiów.

| Instrukcja | Opis | Przykładowe użycie |
| --- | --- | --- |
| setdate dd/mm/yy | Ustawia datę dd/mm/yy zegara RTC | setdate 11/12/22 |
| settime hh:mm | Ustawia czas hh:mm zegara RTC | settime 10:30 |
| printtime | Drukuje na konsoli aktualnie ustawiony czas na zegarze RTC |  |
| printdate | Drukuje na konsoli aktualnie ustawioną datę na zegarze RTC |  |
| save | Zapisuje w kolejnym miejscu pamięci odczyt bieżącej daty, czasu i temperatury |  |
| printmem | Drukuje na konsoli historię zapisaną na EEPROM |  |
| erasemem | Czyści zawartość historii na EEPROM |  |
| memr address | Drukuje na konsoli zawartość komórki adresu address. Adresy podawane jako wartości w systemie 10. | memr 32 |
| setal hh:mm | Ustawia alarm na hh:mm | setal 11:00 |
| refhis | Wymusza pobranie z pamięci EEPROM do pamięci podręcznej mikrokontrolera zapisu historii |  |
| chprog | Przełącza pomiędzy programami: live mode i history mode |  |
| man | Drukuje na konsoli wszystkie instrukcje dostępne dla urządzenia |  |

## Zarządzanie pamięcią
<img width="494" alt="memory" src="https://user-images.githubusercontent.com/106841261/179468684-8faf6774-02f8-4b0f-a1a5-7d7348af77af.png">

Pamięć 24LC01 umożliwia zapis 128B informacji. Na zapis pomiarów, określanych wcześniej jako historia przeznaczono 35B począwszy od adresu 0x00.

Jeden rekord to 7B informacji, w którym po 1B przeznaczono kolejno na: dzień, miesiąc, rok, godzinę, minutę, część całkowitą temperatury, część dziesiętną temperatury. 

Pamięć umożliwia przechowywanie 5 rekordów. Jeśli wystąpi żądanie zapisu kolejnych rekordów, to poprzednie rekordy zostaną nadpisane wg reguły cyklicznej tj. po zapisaniu 0x22 wskaźnik zostanie przesunięty na początek pamięci w miejsce 0x00.

System wie gdzie zapisywać kolejny rekord przez wskaźnik pamięci, który umieszczono w komórce 0x7f. Znajduje się w niej adres kolejnej komórki do zapisu pierwszego byte’a rekordu.
![IMG_0042](https://user-images.githubusercontent.com/106841261/179476986-7a420d2d-3704-40a7-828b-27d68e177792.jpg)

## Doxygen
Dokumentacja doxygen pod linkiem https://pszym87.github.io/Synoptic/index.html

