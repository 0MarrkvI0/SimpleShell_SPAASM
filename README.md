# Shell & Networking Simulator - SPAASM FIIT STU LS 2024/25

**Autor:** Martin Kvietok  
**Subject:** System Programming and Assemblers  
**Faculty:** Fakulta informatiky a informačných technológií, STU Bratislava  

---

## Popis projektu

Tento Unix-based program je jednoduchý **shell s podporou špeciálnych znakov**, ktorý zároveň umožňuje **simuláciu sieťovej komunikácie** medzi klientom a serverom. Shell umožňuje spúšťať príkazy, pracovať s rúrami a presmerovaniami, zatiaľ čo klient-server časť umožňuje posielanie správ, ktoré server prevádza na **veľké písmená** a vracia ich späť klientom.

---

## Funkcionalita

### Shell
- Podpora príkazov ako v klasickom Unix shelli
- Spracovanie špeciálnych znakov: `#`, `;`, `<`, `>`, `|`, `\`
- Prispôsobený prompt: používateľské meno, hostname a aktuálny čas
- Spustenie programov cez `fork()` a `execvp()`
- Presmerovanie vstupu/výstupu, pipy

### Klient–Server simulácia
- Server prijíma text, prevádza ho na **uppercase** a vracia klientovi
- Detekuje a ošetruje odpojenie klienta
- Klient sa pripája k serveru a odosiela vstup
- Pracuje s IP, portmi aj UNIX socketmi (`-p`, `-i`, `-u`)

---

## Spustenie

```bash
./shellnet -s -p 5000        # Spustenie servera na porte 5000
./shellnet -c -i 127.0.0.1   # Pripojenie klienta k serveru cez IP
./shellnet -h                # Zobrazí nápovedu
```

---

## Implementácia

- **Jazyk:** C (GCC)
- **Platforma:** Unix/Linux
- **Makefile:** Vlastný, na jednoduchú kompiláciu
- **Knižnice a volania:**
  - `fork()`, `execvp()`, `dup2()`, `wait()` – shell
  - `socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `read()`, `write()` – sieť

---

## Predpoklady

- Jeden typ spojenia (`-p`, `-u`, alebo `-i`) na jedno spustenie
- Shell aj sieťová vrstva fungujú súbežne
- Vstupné príkazy musia byť dostupné v systéme (`$PATH`)
- Klient/server bežia na tom istom alebo dostupnom systéme

---

## Algoritmy a techniky

- **Parser:** Vlastný, pre spracovanie špeciálnych znakov a príkazových argumentov
- **Sieťová slučka:** Čaká na vstup, prevedie text na veľké písmená, pošle späť
- **Modularita:** `shell.h`, `server_utils.h`, `client_utils.h` – každá časť má vlastný súbor
