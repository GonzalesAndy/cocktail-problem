#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <semaphore.h>
#include <atomic>
#include <signal.h> // Utilisation de SIGINT

using namespace std;

// Bool représentant la disponibilité des ingrédiants
bool hasPineapple(false);
bool hasGrenadine(false);
bool hasRum(false);

// Définition des sémaphores
sem_t boraBoraBartender, pinaColadaBartender, bacardiBartender;
sem_t pineappleJuice, grenadineJuice, rum;
sem_t beverageSupplierSem;
mutex table;

// Entier atomiques afin de suivre le nombre de cocktails servis
atomic<unsigned> boraBoraCocktailCount(0);
atomic<unsigned> pinaColadaCocktailCount(0);
atomic<unsigned> bacardiCount(0);

void beverageSupplier() {
    while (true) {
        sem_wait(&beverageSupplierSem);
        int choice = rand() % 3; // Choisit les deux ingrédiants à fournir
        if (choice == 0) { // Réveille les thread correspondant aux ingrédiants choisis
            sem_post(&pineappleJuice);
            sem_post(&grenadineJuice);
            cout << "Beverage Supplier places pineapple juice and grenadine juice on the table.\n";
        } else if (choice == 1) {
            sem_post(&grenadineJuice);
            sem_post(&rum);
            cout << "Beverage Supplier places grenadine juice and rum on the table.\n";
        } else {
            sem_post(&pineappleJuice);
            sem_post(&rum);
            cout << "Beverage Supplier places pineapple juice and rum on the table.\n";
        }
    }
}

void ingredientDeliverer(sem_t& ingredient, sem_t& firstBartender,sem_t& secondBartender, bool& firstIngredientBool, bool& secondIngredientBool, bool& thirdIngredientBool) {
    while(true) {
        sem_wait(&ingredient);
        lock_guard<mutex> lock(table);
        // Regarde si un autre ingrédiant est disponible, si oui réveille le barmen qui a la bonne combinaison
        if (firstIngredientBool) {
            firstIngredientBool = false;
            sem_post(&firstBartender);
        } else if (secondIngredientBool) {
            secondIngredientBool = false;
            sem_post(&secondBartender);
        } else {
            thirdIngredientBool = true;
        }
    }
}

void bartender(sem_t &bartender, string name, atomic<unsigned> &count) {
    while (true) {
        sem_wait(&bartender);
        cout << name << " is making a cocktail" << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
        sem_post(&beverageSupplierSem); // Demande au fournisseur d'envoyer de nouveaux ingrédiants
        cout << name << " is serving the cocktail" << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
        ++count; // Ajoute au compteur de cocktail servi
    }
}


int main() {
    // Initialisation des sémaphores
    sem_init(&boraBoraBartender, 0, 0);
    sem_init(&pinaColadaBartender, 0, 0);
    sem_init(&bacardiBartender, 0, 0);
    sem_init(&pineappleJuice, 0, 0);
    sem_init(&grenadineJuice, 0, 0);
    sem_init(&rum, 0, 0);
    sem_init(&beverageSupplierSem, 0, 1);


    // Lancement des différents thread
    thread b1(bartender, ref(boraBoraBartender), "Bora Bora Bartender", ref(boraBoraCocktailCount));
    thread b2(bartender, ref(pinaColadaBartender), "Pina Colada Bartender", ref(pinaColadaCocktailCount));
    thread b3(bartender, ref(bacardiBartender), "Bicardi Bartender", ref(bacardiCount));
    thread bs(beverageSupplier);

    thread id1(ingredientDeliverer, ref(rum), ref(pinaColadaBartender), ref(bacardiBartender), ref(hasPineapple), ref(hasGrenadine), ref(hasRum));
    thread id2(ingredientDeliverer, ref(grenadineJuice), ref(boraBoraBartender), ref(bacardiBartender), ref(hasPineapple), ref(hasRum),ref(hasGrenadine));
    thread id3(ingredientDeliverer, ref(pineappleJuice), ref(boraBoraBartender), ref(pinaColadaBartender), ref(hasGrenadine), ref(hasRum), ref(hasPineapple));

    // Affiche le nombre de cocktails servis de chaque type, quand le programme est interrompu (CTRL+C)
    signal(SIGINT, [](int) {
        cout << endl << endl << "Bora Bora Served: " << boraBoraCocktailCount << " times" << endl;
        cout << "Pina Colada Served: " << pinaColadaCocktailCount << " times" << endl;
        cout << "Bicardi Served: " << bacardiCount << " times" << endl;
        exit(0);
    });

    // Attente de la fin des thread
    b1.join();
    b2.join();
    b3.join();

    bs.join();

    id1.join();
    id2.join();
    id3.join();

    return 0;
}
