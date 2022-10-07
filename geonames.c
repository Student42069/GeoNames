#define USAGE "\
Usage: %s  <NUMBER OF CITIES>\n\
\n\
Display a table of the n most populous cities on stdout. The number of cities must not exceed 5000.\n\
\n\
If no argument is provided, the program prints this help and exit.\n\
\n\
Program parameters :\n\
  NUMBER of cities         Number de cities to print (1..500)\n\
  "
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FORMAT_TITRE    "%4s   %-20.20s   %-20.20s   %s\n"
#define FORMAT_COLONNES "%4d   %-20.20s   %-20.20s   %10d\n"
// #define FORMAT_COLONNES "%4d   %-20.20s   %-20.20s   %10ld\n"

/**
 * ----------------
 * Types de données
 * ----------------
 */
struct Pays {
    char nom[50]; // Le nom du pays
    char code[3]; // Le code de deux lettres identifiant le pays
};

struct city{
    char asciiname[205];     
    char country_code[10];
    unsigned int population;
    char pays[50]; // Le pays dans laquelle la ville existe
};

struct ReadCity{
    int geonameid;  
    char name[205];
    char asciiname[205];     
    char alternatenames[10005];
    double latitude;
    double longitude;     
    char feature_class[5]; 
    char feature_code[15];
    char country_code[10];
    char cc2[205];
    char admin1_code[25];
    char admin2_code[85];
    char admin3_code[25];
    char admin4_code[25];
    unsigned int population;
    int elevation;
    int dem; 
    char timezone[45];
    char modification_date[15];
};

/**
 * -----------------------
 * Codes d'erreur
 * -----------------------
 */

enum error {
    OK                      = 0,
    ERREUR_NB_ARGS          = 1,
    ERREUR_NB_VILLES        = 2,
    ERREUR_ARGS_TYPE	    = 3
};

unsigned int count_lines(char *file_name);
unsigned int count_lines_country();
void load_cities(struct city *cities);
FILE* open_file(char *file_name);
void close_file(FILE *fptr);
void fill_fields_city(struct city *one_city, char *buffer);
void load_countries(struct Pays *countries);
void fill_fields_country(struct Pays *one_country, char *buffer);
unsigned int remove_hashtags(FILE *fptr, int count);
void associate_countries(struct city *cities, unsigned int city_count, 
                            struct Pays *countries, unsigned int country_count);
int cmpfunc (const void * a, const void * b);
int handle_parameters(int argc, char *argv[]);
int is__not_numeric(char arg[]);

int main(int argc, char *argv[]) {
    int number_cities = handle_parameters(argc, argv);

    if (number_cities == 0) {
        return 0;
    }

    unsigned int country_count = count_lines_country();
    struct Pays countries[country_count];
    load_countries(countries);

    unsigned int city_count = count_lines("cities15000.txt");
    struct city cities[city_count];
    load_cities(cities);

    associate_countries(cities, city_count, countries, country_count);
    qsort(cities, city_count, sizeof(struct city), cmpfunc);

    // Debug only
    int i;
    printf (FORMAT_TITRE, "Rang", "Nom", "Pays", "Population");
    for (i = 0; i < number_cities; ++i) {
        // if (strcmp(cities[i].asciiname, "Araguari") == 0){
            printf (FORMAT_COLONNES, i + 1, cities[i].asciiname, cities[i].pays, cities[i].population);
        // }
        // printf (FORMAT_COLONNES, i + 1, cities[i].asciiname, cities[i].pays, cities[i].population);
    }

    // char name[201];
    // fgets(name,200,stdin);
    // printf("%s", name);

    // printf("%s\n", argv[0]);
    // printf("%s\n", argv[1]);
    // printf("%s\n", argv[2]);

    return 0;
}

unsigned int count_lines(char *file_name) {
    FILE *fptr = open_file(file_name);
    unsigned int count = 0;
    char c;

    for (c = getc(fptr); c != EOF; c = getc(fptr))
        if (c == '\n') // Increment count if this character is newline
            count = count + 1;

    close_file(fptr);
    return count;
}

unsigned int count_lines_country() {
    FILE *fptr = open_file("countryInfo.txt");
    unsigned int count = 0;
    char c;

    for (c = getc(fptr); c != EOF; c = getc(fptr))
        if (c == '\n') // Increment count if this character is newline
            count = count + 1;
    
    count = remove_hashtags(fptr, count);
    close_file(fptr);
    return count;
}

unsigned int remove_hashtags(FILE *fptr, int count) {
    rewind(fptr);
    char buffer[600 + 1];
    while (fgets(buffer, 600, fptr) != NULL)
        if (buffer[0] == '#') 
            count = count - 1;
    return count;
}

void load_cities(struct city *cities) {
    FILE *fptr = open_file("cities15000.txt");
    char buffer[sizeof(struct ReadCity) + 1];
    int city_count = 0;

    while (fgets(buffer, sizeof(struct ReadCity), fptr) != NULL && strlen(buffer) > 10) {
        fill_fields_city(&cities[city_count], buffer);
        city_count = city_count + 1;
    }

    close_file(fptr);
}

void close_file(FILE *fptr) {
    if (fclose(fptr) == EOF) {
        printf("Erreur dans la fermeture du fichier.\n");
        exit(2);
    }
}

FILE* open_file(char *file_name) {
    FILE *fptr = fopen(file_name, "r");
    
    if (fptr == NULL) {
        printf("Erreur %d\n", 2);
        exit(1);
    }
    return fptr;
}

void fill_fields_city(struct city *one_city, char *buffer) {
    int field_count = 1;
    char *field;
    char field14[50];

    while ((field = strsep(&buffer, "\t")) != NULL) {
        if (field_count == 3) {
            strcpy(one_city -> asciiname, field);
        } else if (field_count == 9) {
            strcpy(one_city -> country_code, field);
        } else if (field_count == 14) {
            strcpy(field14, field);
        } else if (field_count == 15) {
            if (atoi(field) < 15000) {
                one_city -> population = (unsigned int) atoi(field14);
            } else {
                one_city -> population = (unsigned int) atoi(field);
            }
        }
        field_count = field_count + 1;
    }
}

void load_countries(struct Pays *countries) {
    FILE *fptr = open_file("countryInfo.txt");
    char buffer[600 + 1];
    int country_count = 0;

    while (fgets(buffer, 600, fptr) != NULL) {
        if (buffer[0] != '#') {
            fill_fields_country(&countries[country_count], buffer);
            country_count = country_count + 1;
        }
    }

    close_file(fptr);
}

void fill_fields_country(struct Pays *one_country, char *buffer) {
    int field_count = 1;
    char *field;

    while ((field = strsep(&buffer, "\t")) != NULL) {
        if (field_count == 1) {
            strcpy(one_country -> code, field);
        } else if (field_count == 5) {
            strcpy(one_country -> nom, field);
        }
        field_count = field_count + 1;
    }
}

void associate_countries(struct city *cities, unsigned int city_count, struct Pays *countries, unsigned int country_count) {
    int city_counter;
    int country_counter;

    for (city_counter = 0; city_counter < city_count; city_counter++) {
        for (country_counter = 0; country_counter < country_count; country_counter++) {
            if (strcmp(cities[city_counter].country_code, countries[country_counter].code) == 0) {
                strcpy(cities[city_counter].pays, countries[country_counter].nom);
                break;
            }
        }
    }
}

int cmpfunc (const void * a, const void * b) {
    int l = ((struct city *)a)->population;
    int r = ((struct city *)b)->population; 
    return (r - l);
}

int handle_parameters(int argc, char *argv[]) {
    if (argc != 2) {
        printf("%s\n", "nombre arguments invalide");
        printf("%s\n", USAGE);
        return 0;
    }

    if (is__not_numeric(argv[1])) {
        printf("%s\n", "type argument invalide");
        return 0;
    }

    if ((atoi(argv[1]) < 1) || (atoi(argv[1]) > 5000)) {
        printf("%s\n", "nombre de ville invalide");
        return 0;
    }

    return atoi(argv[1]);
}

int is__not_numeric(char arg[]) {
    int i;
    char minus = '-';
    char first = arg[0];
    if ('-' == arg[0]) 
        return 0;
    for (i = 0; i < strlen(arg); i++)
        if(arg[i] < '0' || arg[i] > '9')
            return 1;
    return 0;
}