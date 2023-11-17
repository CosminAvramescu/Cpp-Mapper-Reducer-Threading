![topology](https://i.imgur.com/nMQQvSt.png)

---Aspecte generale 
	M-am folosit de 3 clase pentru a avea codul mai bine organizat (Mapper, Reducer, Barrier).
Cea mai importanta parte a temei e reprezentata de functiile thread_function_mapper si 
thread_function_reducer. In get_args extrag parametri rularii. In print creez fisierele de output
si scriu in ele rezultatul cu fputs(). La finalul thread_function_mapper apelez wait pe bariera,
iar la inceputul thread_function_reducer apelez din nou wait pe bariera. Aceasta bariera este
initializata cu M+R. Practic, este nevoie de M+R thread-uri sa ajunga la bariera pentru a se putea
trece mai departe. Prin asezarea barierei la inceputul thread_function_reducer ma asigur ca nu se
vor prelucra datele de Reduceri decat dupa ce vor termina Mapperii sa citeasca.

---Main
	Creez vectorul de mapperi si vectorul de reduceri si instantiez clasa Barrier. Creez 
threadurile si le atribui thread_function in functie de tipul lor (mapper sau reducer). Ele vor
intra in thread_function si vor executa operatiile. La final, revin toate in threadul main datorita
metodei join.

---Barrier
	Clasa Barrier contine, printre altele, o bariera, un mutex, vectorul
de mapperi si vectorul de reduceri. Le initializez pe toate in constructor. Instanta mea de Barrier
pe care am creat-o in main o dau ca parametru functiilor thread_function pentru a avea acces la
bariera (astfel am acces la aceeasi instanta de barrier si in thread_function_mapper si in
thread_function_reducer).

---Mapper
	Fiecare mapper are un vector de liste(pentru fiecare exponent). Toate mapperele au
acces la variabilele statice n(nr de fisiere ce trebuie citite) si vectorul cu numele acestor
fisiere. In metoda statica init setez n si vectorul cu numele fisierelor. Metoda de parseFiles
este apelata din thread_function_mapper, primeste un nume de fisier si parcurge linie cu linie.
Se verifica numerele citite in isPerfectPower (daca se gaseste prin binarysearch-BS un exponent
se adauga la lista[exponent] numarul citit).

---Reducer
	Fiecare reducer are o lista (unde se pun toate valorile corespunzatoare unui exponent 
din toti mapperii) si un set (unde se pun toate valorile din lista anterior creata). Setul fiind
o multime, nu vor exista duplicate, asa ca pentru a numara valorile unice, doar returnez size()
de la set in metoda countUniqueValues().

---thread_function_mapper()
	Argumentul functiei este obiectul de tip Barrier. Cu ajutorul unui mutex incrementez index
pentru a stii id-ul thread-ului la care ma aflu. Am avut nevoie de un mutex pentru aceasta operatie pentru
a nu suprascrie indexul 2 thread-uri in acelasi timp. Tot in acest mutex incrementez lastReadedFile
(am avut nevoie de el pentru a imparti fisierele de intrare mapperelor in cazul in care erau mai
putine mappere decat fisiere de intrare). Cat timp lastReadedFile este mai mic decat nr total
de fisiere, mai parcurg un fisier. Daca nr de mapperi este egal cu nr de fisiere, ies din while
(pentru ca fiecare mapper va avea un fisier assignat), altfel, daca inca mai sunt fisiere de citit,
incrementez lastReadedFile cu un nou mutex si dau continue pentru a trece la urmatoarea iteratie in
while. 

---thread_function_reducer()
	In continuare incrementez indexul thread-ului curent in mutex, apoi initializez lista din
reducer, combin datele din lista anterior initializata intr-un set si printez rezultatul apeland
metoda countUniqueValues() in metoda print().