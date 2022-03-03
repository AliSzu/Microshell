Projekt zaliczeniowy na Systemy Operacyjne w semestrze zimowym 2020

# <b>Specyfikacja Wymagań</b>
Zadanie polega na wykorzystaniu języka C do napisania prostego programu powłoki działającego pod Linuxem – Microshell. Program ten powinien przyjmować na wejściu polecenia, a następnie wykonywać działania zgodne z ich treścią.

### Funkcjonalności programu
  - wyświetlanie znaku zachęty w postaci `[{path}] $`, gdzie `{path}` jest ścieżką do bieżącego katalogu roboczego
  - obsługiwanie polecenie `cd`, działające analogicznie jak `cd` znane nam z powłoki bash
  - obsługiwanie polecenie `exit`, kończące działanie programu powłoki
  - obsługiwanie polecenie `help`, wyświetlające na ekranie informacje o autorze programu i oferowanych przez niego funkcjonalnościach
  - przyjmowanie polecenia odwołujące się przez nazwę do programów znajdujących się w katalogach opisanych wartością zmiennej środowiskowej `PATH` oraz umożliwiać wywołanie tych skryptów i programów z argumentami (czyt. `fork()` + `exec*()`)
  - wypisywanie komunikat błędu, gdy niemożliwe jest poprawne zinterpretowanie polecenia
  - tworzenie katalogów za pomocą polecenia `mkdir`, obsługiwane są również wariacje z podkatalogami przy pomocy flagi `-p`
  - usuwanie plików przy pomocy polecenia `rm` oraz z flagą `-r` do usuwania folderów
  - kopiowanie plików przy pomocy polecenia `cp` oraz z flagą `-r` do kopiowania folderów
  - program oferuje przeglądanie historii ostatnio użytych poleceń poprzez naciśniecie strzałki w górę
  - program oferuje autouzupełnianie dostępnych komend poprzez przycisk `tab`
