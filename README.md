# Map-Reduce-perfect-powers-using-PThreads
#Copyright Stan Andreea 333CA

void writeFunction = > functia se ocupa cu scrierea in fisierul de output a rezultatului
                    trimis ca parametru.

int *nThRoot = > functia se ocupa cu aflarea numerelor ce sunt puteri perfecte pentru
                o anumita putere data ca parametru si intoarce un vector format din
                toate aceste numere.
                Dupa ce-mi setez startul si endul pentru fiecare numar din vector ce
                trebuie verificat(initial startul = 1 si endul = cu numarul curent pentru
                care fac verificarea), verific, cat timp start-ul este mai mic decat
                numarul meu, micsorand la fiecare pas intervalul in care caut
                puterea perfecta, daca puterea se afla in jumatatea din stanga/dreapta
                calculata fata de mijlocul intervalului(start, end).
                Pentru a evita folosirea functiei pow si aparitia unui overflow, am facut
                inmultiri repetate(ridicari la putere) intr-un for si m-am oprit cand aceste
                inmultiri ajungeau sa dea rezultate mai mari decat numarul meu initial.
                In momentul in care gasesc puterea egala cu numarul meu initial, il memorez
                in vectorul rezultat si actualizez contorul pentru lungimea vectorului returnat.

void processFunction => Functia se ocupa cu procesarea fisierului dat ca parametru in structura
                thread-ului(maperului) pentru care efectuez procesarea. Dupa ce citesc din fisier
                si-mi memorez intr-un vector toate elementele pentru care voi efectua procesarea,
                incep sa calculez pentru puterile de la 2 pana la numarul de reduceri + 1. Initial
                imi salvez intr-un set nou elementele ce erau in hashtable-ul maperului curent pentru
                cheia reprezentata de puterea pentru care vreau sa verific, apoi apelez functia
                nth rooth ce calculeaza daca numarul dat e putere perfecta sau nu.
                Daca gasesc elementul ca fiind putere perfecta, in cazul in care nu mai am niciun
                element la cheia potrivita, doar creez o pereche cheie-valoare pe care o inserez,
                daca mai era inainte ceva, creez initial setul inserand si noile valori (la cel
                format inainte de apelarea functiei nth root) si doar spun hashtable-ului sa
                pointeze la adresa setului.

void *reducer => Functia este apelata la crearea threadurilor(reducerilor) si realizeaza contorizarea
                numarului total de numere puteri perfecte pentru o anume putere. Astfel, pentru
                fiecare reducer am asignat o putere (reducer #1 => puterea 2 etc), am parcurs fiecare
                hashtable al fiecarui mapper si am creat un nou set cu toate valorile potrivite pentru
                acea putere(spre exemplu, pentru puterea 2 am parcurs hashtable-urile la cheia 2 si am 
                extras toate valorile de acolo). Dupa asta, cu ajutorul unui mutex am creat numele
                fisierului de output corespunzator reducerului.

void *mapper => Functia este apelata la crearea threadurilor(maperilor) si realizeaza asignarea de
                fisiere pentru fiecare thread pentru a fi prelucrate. Dupa ce se revine din functia
                de procesare, cu ajutorul unui mutex pointez cu lista in care se vor afla
                toate hashtable urile pentru toti mapperi la adresa hashtable-ului nou format.

int main => In aceasta functie se afla toata logica pentru crearea si stergerea de threaduri, precum
            si asignarea anumitor campuri din structurile de mapperi si reduceri.
