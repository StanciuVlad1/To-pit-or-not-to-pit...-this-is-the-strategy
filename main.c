#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

//includere functiei predefinite care trebuia folosita
void get_operations(void **operations);
//functie care determina index-ul sensorului pe care se aplica o alta functie
int get_index(char s[]);
//functie care citeste sensorii din fisierul de input
void read_sensors(FILE *f, sensor *sensors, int n);
//functie care sorteaza senzorii in functie de importanta (PMU > TIRE)
void sort_sensors(sensor *sensors, int n);
//functia care rezolva problema propriu-zisa
void rezolvare(sensor *sensors, int *n, FILE *f);
//functia de print necesara problemei
void print(sensor *sensors, int index);
//functia de exit necesara problemei
void Exit(sensor *sensors, int n, int *ok, FILE *f);
//functie care sterge un anumit senzor din vectorul de senzori
void delete_sensor(sensor ***sensors, int *n, int index);
//functia de clear necesara problemei
void clear(sensor **sensors, int *n);
//functia de analyze necesara problemei
void analyze(sensor *sensors, int index);
//functia elibereaza memoria alocata dinamic
void freemem(sensor *sensors, int n);
int main(int argc, char const *argv[])
{ //se deschide fisierul de intrare
	FILE *f = fopen(argv[1], "rb");
	if (f == NULL)
	{
		printf("Eroarea la creearea fisierului!\n");
		return 1;
	}
	//declaram un int, "n" care reprezinta numarul de senzori din fisier
	int n;
	//citim din fisier acest numar
	fread(&n, sizeof(n), 1, f);
	//declaram un vector de senzori
	sensor *sensors;
	//alocam dinamic vectorul de senzori
	sensors = (sensor *)malloc(sizeof(sensor) * n);
	//citim senzorii din fisier prin functia definita anterior
	read_sensors(f, sensors, n);
	//sortam vectorul de senzori prin functia definita anterior
	sort_sensors(sensors, n);
	//apelam functia de rezolvare a problemei propriu-zisa
	rezolvare(sensors, &n, f);
	return 0;
}
int get_index(char s[])
{ //declaram nr, care reprezinta numarul nostru ce se afla in comanda
	/*i este un iterator care ne va ajuta sa ne plimbam prin vectorul de tip 
	char*/
	//negativ ne va spune daca in "s" se va intalni semnul "-"
	int nr = 0, i, negativ = 0;
	//parcurgem comanda data de la tastatura
	for (i = 0; i < strlen(s); i++)
	{ //verificam daca caracterul de pe pozitia curenta este cifra
		if (s[i] >= '0' && s[i] <= '9')
			//in caz afirmativ este adaugata la numarul nostru
			nr = nr * 10 + (s[i] - '0');
		//daca intalnim semnul "-", negativ va deveni 1
		if (s[i] == '-')
			negativ = 1;
	}
	//in cazul in care numarul nostru este negativ, il inmultim cu (-1)
	if (negativ)
		nr = -nr;
	//returnam numarul obtinut
	return nr;
}

void read_sensors(FILE *f, sensor *sensors, int n)
{ //i este un iterator
	//tip va determina tipul sensorului
	int i, tip;
	//parcurgem fisierul de input
	for (i = 0; i < n; i++)
	{ //citim din fisier tipul senzorului
		fread(&tip, sizeof(tip), 1, f);
		//il atribuim in structura senzorului
		sensors[i].sensor_type = tip;
		//verificam daca este de tip TIRE sau PMU si facem citirile respective
		if (tip == TIRE)
		{ //declaram sensor_data de tipe tire_sensor * si il alocam dinamic
			sensors[i].sensor_data = (tire_sensor *)malloc(sizeof(tire_sensor));
			//citim vectorul sensor_data al senzorului i
			fread(sensors[i].sensor_data, sizeof(tire_sensor), 1, f);
		}
		else
		{ //declaram sensor_data de tip pmu * si il alocam dinamic
			sensors[i].sensor_data = (power_management_unit *)malloc(sizeof(power_management_unit));
			//citim vectorul sensor_data al senzorului i
			fread(sensors[i].sensor_data, sizeof(power_management_unit), 1, f);
		}
		//citim numarul de operatii pentru senzorul i
		fread(&sensors[i].nr_operations, sizeof(int), 1, f);
		//alocam dinamic vectorul operations_idxs de tip int
		sensors[i].operations_idxs = (int *)malloc(sizeof(int) * sensors[i].nr_operations);
		//citim din fisier vectorul operations_idxs al senzorului i
		fread(sensors[i].operations_idxs, sizeof(int) * sensors[i].nr_operations, 1, f);
	}
}

void sort_sensors(sensor *sensors, int n)
{ //declaram un vector auxiliar de tip pmu
	sensor sensors_pmu[n];
	//declaram un vector auxiliar de tip tire
	sensor sensors_tire[n];
	//retinem in 2 cele 2 variabile numarul de senzori corespunzatori
	int nr_pmu = 0, nr_tire = 0;
	//declaram un iterator
	int i;
	//parcurgem vectorul de senzori
	for (i = 0; i < n; i++)
		/*adaugam snezorii in vectorii auxiliari corespunzatori fiecarui tip,
		asa cum acestia apar in citirea initiala*/
		if (sensors[i].sensor_type == 0)
			sensors_tire[nr_tire++] = sensors[i];
		else
			sensors_pmu[nr_pmu++] = sensors[i];
	//punem 0 in nr_tire pentru a parcurge vectorul auxiliar tire de la inceput
	nr_tire = 0;
	//parcurgem din nou vectorul de senzori
	for (i = 0; i < n; i++)
		/*atata timp cat i < pmu, punem pe pozitia i, toti senzorii de tipul
		pmu in ordinea citirii lor la inceputul vectorului de senzori*/
		if (i < nr_pmu)
			sensors[i] = sensors_pmu[i];
		else
			/*s-au terminat sensorii de tip pmu,asa ca vom pune toti senzorii
			de tip tire la finalul vectorului de senzori*/
			sensors[i] = sensors_tire[nr_tire++];
}
void rezolvare(sensor *sensors, int *n, FILE *f)
{ /*folosimt acest while pentru a crea o bucla infinita pana la intalnirea
	comenzii de exit */
	int ok = 1;
	while (ok)
	{ /*decaram un char comanda de 13 caractere- reprezentand cea mai lunga
		comanda existenta in fiserele de input*/
		char comanda[13];
		//citim de la tastaturar comanda
		fgets(comanda, 13, stdin);
		/*deoarece comanda fgets pune un \n inainte de \0, punem in loc de\n
		 \0*/
		comanda[strlen(comanda) - 1] = '\0';
		/*declaram un index si apelam functia de get_index pentru a stii pe ce
		senzor vom aplica functii*/
		int index;
		index = get_index(comanda);
		/*daca indexul este mai mare decat numarul de senzori sau este negativ
		afisam un mesaj corespunzator*/
		if (index >= *n || index < 0)
			printf("Index not in range!\n");
		else
		{ //daca coamnda este de tip print, apelam functia de print
			if (strstr(comanda, "print"))
				print(sensors, index);
			//daca coamnda este de tip exit, apelam functia de exit
			if (strstr(comanda, "exit"))
				Exit(sensors, *n, &ok, f);
			//daca comanda este de tip clear, apelam functia de clear
			if (strstr(comanda, "clear"))
				clear(&sensors, n);
			//daca comanda este de tip analyze, apelam functia de analyze
			if (strstr(comanda, "analyze"))
				analyze(sensors, index);
		}
	}
}
void print(sensor *sensors, int index)
{ //afisam tipul senzorului
	printf("%s\n", sensors[index].sensor_type == TIRE ? "Tire Sensor" : "Power Management Unit");
	//facem afisarile corespunzatoare fiecarui tip, dupa ce il determinam
	if (sensors[index].sensor_type == TIRE)
	{ //declaram un vector auxiliar de tip tire_sensor
		tire_sensor *tire = sensors[index].sensor_data;
		printf("Pressure: %.2f\n", tire->pressure);
		printf("Temperature: %.2f\n", tire->temperature);
		printf("Wear Level: %d%%\n", tire->wear_level);
		//in cazul in care score-ul este 0, inseamna ca nu a fost calculat
		if (tire->performace_score == 0)
			printf("Performance Score: Not Calculated\n");
		else
			//daca score-ul a fost calculat, il afisam
			printf("Performance Score: %d\n", tire->performace_score);
	}
	else
	{ //declaram un vector auxiliar de tip pmu
		power_management_unit *pmu = sensors[index].sensor_data;
		printf("Voltage: %.2f\n", pmu->voltage);
		printf("Current: %.2f\n", pmu->current);
		printf("Power Consumption: %.2f\n", pmu->power_consumption);
		printf("Energy Regen: %d%%\n", pmu->energy_regen);
		printf("Energy Storage: %d%%\n", pmu->energy_storage);
	}
}
void Exit(sensor *sensors, int n, int *ok, FILE *f)
{ //facem ok-ul 0 pentru a iesi din bucla infinita
	*ok = 0;
	//eliberam memoria utilizata de-a lungul rezolvarii problemei
	freemem(sensors, n);
	//inchidem fisierul de input
	fclose(f);
}
void delete_sensor(sensor ***sensors, int *n, int index)
{ //declaram un senzor auxiliar
	sensor sensor_to_delete;
	//in senzorul auxiliar, retinem senzorul care trebuie sters
	sensor_to_delete = (**sensors)[index];
	//declaram un iterator
	int i;
	//mutam cu o pozitie la stanga toti senzorii
	for (i = index; i < (*n) - 1; i++)
		(**sensors)[i] = (**sensors)[i + 1];
	//eliberam memoria ocupata de senzorul care trebuie sters
	free(sensor_to_delete.operations_idxs);
	free(sensor_to_delete.sensor_data);
	//modificam numarul de senzori
	(*n)--;
}
void clear(sensor **sensors, int *n)
{ //declaram un iterator
	int i;
	//parcurgem vectorul de senzori
	for (i = 0; i < *n; i++)
	{ //declaram un ok care va determina daca senzorul trebuie sters sau nu
		int ok = 1;
		//stabilim tipul senzorului si vericiam fiecare caracteristica in parte
		if ((*sensors)[i].sensor_type == TIRE)
		{
			//declaram un vector auxiliar de tip tire_sensor
			tire_sensor *tire = (*sensors)[i].sensor_data;
			//in cazul in care o caracteristica nu este corecta, ok va deveni 0
			if (tire->pressure < 19 || tire->pressure > 28)
				ok = 0;
			if (tire->temperature < 0 || tire->temperature > 120)
				ok = 0;
			if (tire->wear_level < 0 || tire->wear_level > 100)
				ok = 0;
		}
		else
		{ //declaram un vector auxiliar de tip pmu
			power_management_unit *pmu = (*sensors)[i].sensor_data;
			//in cazul in care o caracteristica nu este corecta, ok va deveni 0
			if (pmu->voltage < 10 || pmu->voltage > 20)
				ok = 0;
			if (pmu->current < -100 || pmu->current > 1000)
				ok = 0;
			if (pmu->power_consumption < 0 || pmu->power_consumption > 1000)
				ok = 0;
			if (pmu->energy_regen < 0 || pmu->energy_regen > 100)
				ok = 0;
			if (pmu->energy_storage < 0 || pmu->energy_storage > 100)
				ok = 0;
		}
		if (!ok)
		{ //daca ok este 0, atunci senzorul va fi sters
			delete_sensor(&sensors, n, i);
			//decrementam i-ul pentru a nu pierde niciun senzor
			i--;
		}
	}
	//realocam vectorul de senzori cu noua valoare a lui n
	sensors = realloc(sensors, (*n) * sizeof(sensor));
}

void analyze(sensor *sensors, int index)
{ //declaram un vector de pointeri la functii de tip void
	void (*operations[8])(void *);
	//aplicam functia predefinita pe vectorul declarat
	get_operations((void *)operations);
	//declaram un iteratoe
	int i;
	//parcurgem vectorul de operatii al senzorului primit ca parametru
	for (i = 0; i < sensors[index].nr_operations; i++)
	{ //declaram un int auxiliar pentru a apela functia necesara
		int index_operation_to_do = sensors[index].operations_idxs[i];
		//accesam functia la care pointeaza pointerul de la indexul respectiv
		(*operations[index_operation_to_do])(sensors[index].sensor_data);
	}
}

void freemem(sensor *sensors, int n)
{ //declaram un iterator
	int i;
	//parcurgem vectorul de senzori
	for (i = 0; i < n; i++)
	{ //dezalocam memoria alocata dinamic din cadrul fiecarui senzor
		free(sensors[i].operations_idxs);
		free(sensors[i].sensor_data);
	}
	//dezalocam memoria propriu-zisa a vectorului de senzori
	free(sensors);
}