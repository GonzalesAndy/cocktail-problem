#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <semaphore.h>
#include <atomic>
#include <signal.h>

using namespace std;

bool hasPineapple(false);
bool hasCoconutCream(false);
bool hasRum(false);

sem_t boraBoraBartender, pinaColadaBartender, bacardiBartender;
sem_t pineappleJuice, coconutCream, rum;
sem_t beverageSupplierSem;
mutex table;

atomic<unsigned> boraBoraCocktailCount(0);
atomic<unsigned> pinaColadaCocktailCount(0);
atomic<unsigned> bacardiCount(0);

void beverageSupplier() {
    while (true) {
        sem_wait(&beverageSupplierSem);
        int choice = rand() % 3;
        if (choice == 0) {
            sem_post(&pineappleJuice);
            sem_post(&coconutCream);
            cout << "Beverage Supplier places pineapple juice and coconut cream on the table.\n";
        } else if (choice == 1) {
            sem_post(&coconutCream);
            sem_post(&rum);
            cout << "Beverage Supplier places coconut cream and rum on the table.\n";
        } else {
            sem_post(&pineappleJuice);
            sem_post(&rum);
            cout << "Beverage Supplier places pineapple juice and rum on the table.\n";
        }
    }
}

void ingredientDelivererPineapple() {
    while (true) {
        sem_wait(&pineappleJuice);
        lock_guard<mutex> lock(table);
        if (hasCoconutCream) {
            hasCoconutCream = false;
            sem_post(&bacardiBartender);
        } else if (hasRum) {
            hasRum = false;
            sem_post(&pinaColadaBartender);
        } else {
            hasPineapple = true;
        }
    }
}

void ingredientDelivererCoconutCream() {
    while (true) {
        sem_wait(&coconutCream);
        lock_guard<mutex> lock(table);
        if (hasPineapple) {
            hasPineapple = false;
            sem_post(&bacardiBartender);
        } else if (hasRum) {
            hasRum = false;
            sem_post(&boraBoraBartender);
        } else {
            hasCoconutCream = true;
        }
    }
}

void ingredientDelivererRum() {
    while (true) {
        sem_wait(&rum);
        lock_guard<mutex> lock(table);
        if (hasPineapple) {
            hasPineapple = false;
            sem_post(&pinaColadaBartender);
        } else if (hasCoconutCream) {
            hasCoconutCream = false;
            sem_post(&boraBoraBartender);
        } else {
            hasRum = true;
        }
    }
}

void bartender(sem_t &bartender, string name, atomic<unsigned> &count) {
    while (true) {
        sem_wait(&bartender);
        cout << name << " is making a cocktail" << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
        sem_post(&beverageSupplierSem);
        cout << name << " is serving the cocktail" << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
        ++count;
    }
}


int main() {
    sem_init(&boraBoraBartender, 0, 0);
    sem_init(&pinaColadaBartender, 0, 0);
    sem_init(&bacardiBartender, 0, 0);
    sem_init(&pineappleJuice, 0, 0);
    sem_init(&coconutCream, 0, 0);
    sem_init(&rum, 0, 0);
    sem_init(&beverageSupplierSem, 0, 1);

    thread b1(bartender, ref(boraBoraBartender), "Bora Bora Bartender", ref(boraBoraCocktailCount));
    thread b2(bartender, ref(pinaColadaBartender), "Pina Colada Bartender", ref(pinaColadaCocktailCount));
    thread b3(bartender, ref(bacardiBartender), "Bicardi Bartender", ref(bacardiCount));
    thread bs(beverageSupplier);

    // Add the threads for the ingredient deliverers
    thread id1(ingredientDelivererPineapple);
    thread id2(ingredientDelivererCoconutCream);
    thread id3(ingredientDelivererRum);

    signal(SIGINT, [](int) {
        cout << endl;
        cout << "Bora Bora Served: " << boraBoraCocktailCount << " times" << endl;
        cout << "Pina Colada Served: " << pinaColadaCocktailCount << " times" << endl;
        cout << "Bicardi Served: " << bacardiCount << " times" << endl;
        exit(0);
    });

    b1.join();
    b2.join();
    b3.join();
    bs.join();

    id1.join();
    id2.join();
    id3.join();

    return 0;
}
