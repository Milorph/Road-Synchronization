#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <queue>
#include <fstream>
#include <iostream>

using namespace std;

int carCount = 0;
int passedCar = 0;
int carLimit = 100;

sem_t sem_north, sem_south;
pthread_mutex_t locker;

bool flagState = false;   // false - sleep; true - woken up

struct Car {
  int carID;
  char direction;         // 'N' - North, 'S' - South
  time_t arrival_time;
  time_t start_time;
  time_t end_time;
};

queue<Car> north;
queue<Car> south;

/* pthread_sleep takes an integer number of seconds to pause the current thread We provide this function 
because one does not exist in the standard pthreads library. We simply use a function that has a timeout. */

int pthread_sleep (int seconds) {
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;

    if (pthread_mutex_init(&mutex,NULL)) {
    return -1;
    }
    if (pthread_cond_init(&conditionvar,NULL)) {
    return -1;
    }

    timetoexpire.tv_sec = (unsigned int)time(NULL) + seconds;
    timetoexpire.tv_nsec = 0;

    return pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
}

// Producers

void* northTraffic(void* arg) {
  while(passedCar < carLimit) {
    do {
        sem_wait(&sem_north);             // Wait for a car to arrive from the North
        pthread_mutex_lock(&locker);      // Locker mutex before modifying shared resources
        
        Car car = {++carCount, 'N', time(NULL), 0, 0};
        north.push(car);

        pthread_mutex_unlock(&locker);    // Unlock mutex after modifying shared resources
        sem_post(&sem_north);              // Signal the flag person that there is a new car waiting

    } while ((rand() % 10) <= 8 && carCount < carLimit);

    pthread_sleep(20);
  }
  pthread_exit(NULL);
}

void* southTraffic(void* arg) {
    while(passedCar < carLimit) {
        do {
            sem_wait(&sem_south);
            pthread_mutex_lock(&locker);      // Locker mutex before modifying shared resources

            Car car = {++carCount, 'S', time(NULL), 0, 0};
            south.push(car);

            pthread_mutex_unlock(&locker);    // Unlock mutex after modifying shared resources
            sem_post(&sem_south);

    } while ((rand() % 10) < 8 && carCount < carLimit);

        pthread_sleep(20);
    }
    pthread_exit(NULL);
}

// ofstream output to .log file

void carLog(Car* car) {
    std::ofstream logFile("car.log", std::ios_base::app);

    std::ifstream checkFile("car.log");
    if (checkFile.peek() == std::ifstream::traits_type::eof()) {
    logFile << "carID\tdirection\tarrival-time\tstart-time\tend-time\n";
    }

    logFile <<  car->carID << "\t\t\t" << car->direction << "\t\t";

    char buffer[10];
    strftime(buffer, sizeof(buffer), " %H:%M:%S", localtime(&car->arrival_time));
    logFile << buffer << "\t\t";

    strftime(buffer, sizeof(buffer), " %H:%M:%S", localtime(&car->start_time));
    logFile << buffer << "\t";

    strftime(buffer, sizeof(buffer), " %H:%M:%S", localtime(&car->end_time));
    logFile << buffer << "\n";

    // Flush the output stream and close the file
    logFile.flush();
    logFile.close();
}


void flagLog() {
    std::ofstream logFile("flagperson.log", std::ios_base::app);

    std::ifstream checkFile("flagperson.log");
    if (checkFile.peek() == std::ifstream::traits_type::eof()) {
        logFile << "Time\t\t\t\tState\n";
    }

    time_t curr = time(NULL);
    char buffer[10];
  strftime(buffer, sizeof(buffer), " %H:%M:%S", localtime(&curr));

    logFile << buffer << "\t";

    if (flagState) {
        logFile << "\t\twoken-up\n";
    } else {
        logFile << "\t\tsleep\n";
    }

    // Flush the output stream and close the file
    logFile.flush();
    logFile.close();
}




// function for each car thread
void* carThread(void* arg) {
    Car* car = new Car(*(Car*)arg);
    car->start_time = time(NULL);
    pthread_sleep(1);
    car->end_time = time(NULL);
    carLog(car);
    passedCar++;
    delete car;
    pthread_exit(NULL);
}

// Consumer helper
void northPass() {
    pthread_t eachCar;
    sem_wait(&sem_north);
    pthread_mutex_lock(&locker);
    Car car = north.front();
    north.pop();
    pthread_create(&eachCar, NULL, &carThread, (void*)&car);
    pthread_join(eachCar, NULL);
    pthread_mutex_unlock(&locker);
    sem_post(&sem_north);
}

void southPass() {
    pthread_t eachCar;
    sem_wait(&sem_south);
    pthread_mutex_lock(&locker);
    Car car = south.front();
    south.pop();
    pthread_create(&eachCar, NULL, &carThread, (void*)&car);
    pthread_join(eachCar, NULL);
    pthread_mutex_unlock(&locker);
    sem_post(&sem_south);
}

// Consumer
void *worker(void *arg) {
    flagLog();

    while(passedCar < carLimit) {
        if (!north.empty() || !south.empty() && !flagState) {
            flagState = true;
            flagLog();
        }

    if (!north.empty()) {
      while (!north.empty()) {
        northPass();
        if (north.size() < 10 && south.size() > 10) {
          break;
        }
      }
    } else if (!south.empty()) {
      while (!south.empty()) {
        southPass();
        if (south.size() < 10 && north.size() > 10) {
          break;
        }
      }
    }
    if (north.empty() && south.empty() && flagState) {
      flagState = false;
      flagLog();
    }
  }
  pthread_exit(NULL);
}

/* The main function is just an infinite loop that spawns off a second thread * that also is an infinite loop.
We should see two messages from the worker for every one from main. */
int main(int argc, char* argv[]) {
    if (argc == 2) {
        carLimit = atoi(argv[1]);
    }
    if (argc != 2)
    {
        printf("Usage: %s <total numebr of cars>\n", argv[0]);
        return 1;
    }

    // Initialize semaphores and mutex
    sem_init(&sem_north, 0, 1);
    sem_init(&sem_south, 0, 1);
    pthread_mutex_init(&locker, NULL);

    pthread_t flagPerson, northQ, southQ;

    pthread_create(&flagPerson, NULL, worker, NULL);
    pthread_create(&northQ, NULL, northTraffic, NULL);
    pthread_create(&southQ, NULL, southTraffic, NULL);

    while(passedCar < carLimit) {
        printf("Process is Running, Please Wait");
        fflush(stdout);
        for (int i = 0; i < 3; i++) {
            pthread_sleep(2);
            printf(". ");
            fflush(stdout);
        }
        printf("\n");
        pthread_sleep(3);
    }
    // join thread
    pthread_join(flagPerson, NULL);
    pthread_join(northQ, NULL);
    pthread_join(southQ, NULL);

    // destroy all unused mutex and semaphore
    pthread_mutex_destroy(&locker);

    sem_destroy(&sem_north);
    sem_destroy(&sem_south);
    printf("You're done!");
    return 0;
}
