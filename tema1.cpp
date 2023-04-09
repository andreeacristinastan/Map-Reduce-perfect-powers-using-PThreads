#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <math.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
using namespace std;

typedef struct
{
    int mapper_id;
    vector<string> *fileNames;
    int totalReducers;
    pthread_mutex_t *mutex;
    pthread_barrier_t *barrier;
    string processFileName;
    unordered_map<int, unordered_set<int>> *hashMap;
    vector<unordered_map<int, unordered_set<int>>> *listOfAll;

} mapper_struct;

typedef struct
{
    pthread_barrier_t *barrier;
    vector<unordered_map<int, unordered_set<int>>> *listOfAll;
    pthread_mutex_t *mutexRed;
    string fileDest;
    int thread_id;

} reducer_struct;

/**
 * @brief -Functia scrie in fisiere rezultatul pentru fiecare putere
 *
 * @param myRed - pointer la structura mea reducer
 * @param res - rezultatul ce trebuie pus in fisierul de iesire
 */
void writeFunction(reducer_struct *myRed, int res)
{
    ofstream fileForRes;
    fileForRes.open(myRed->fileDest);

    fileForRes << res;
    fileForRes.close();
}

/**
 * @brief - functia primeste ca parametru un vector de numere si verifica pentru
 *          fiecare daca sunt puteri perfecte pentru puterea data ca parametru
 *
 * @param vector - vectorul de numere
 * @param numberOfElements  - numarul total de elemente
 * @param power - puterea pentru care calculam
 * @param res - vectorul in care stocam rezultatul
 * @param cnt - dimensiunea vectorului res
 * @return int* - vectorul format
 */
int *nThRoot(int vector[], int numberOfElements, int power, int res[], int *cnt)
{
    int start = 1, end;
    long mid;

    for (int i = 0; i < numberOfElements; i++)
    {
        start = 1;
        end = vector[i];
        while (start <= end)
        {
            mid = (start + end) / 2;
            long poww = 1;

            for (int j = 0; j < power; j++)
            {
                poww *= mid;

                if (poww > vector[i])
                {
                    break;
                }
            }
            if (poww == vector[i])
            {
                res[(*cnt)] = vector[i];
                (*cnt) += 1;
            }

            if (poww < vector[i])
            {
                start = mid + 1;
            }

            else
            {
                end = mid - 1;
            }
        }
    }
    return res;
}

/**
 * @brief - functie care primeste un thread si se ocupa cu
 *          citirea numerelor din fisierul pentru care se efectueaza
 *          prelucrarea si se actualizeaza hashtable-ul thread-ului cu
 *          numerele ce sunt puteri perfecte din fisier.
 *
 * @param myMap - pointer la thread-ul pentru care se efectueaza prelucrarea
 *              (mapper)
 */
void processFunction(mapper_struct *myMap)
{
    ifstream fileOfNumbers;
    fileOfNumbers.open(myMap->processFileName);

    int allNumbers;

    fileOfNumbers >> allNumbers;

    int myVec[allNumbers], cnt = 0;

    for (int i = 0; i < allNumbers; i++)
    {
        fileOfNumbers >> myVec[i];
    }

    for (int i = 2; i <= myMap->totalReducers + 1; i++)
    {
        cnt = 0;
        int res[allNumbers];
        unordered_set<int> resSet;
        
        if (myMap->hashMap->count(i) != 0)
        {
            for (auto j = myMap->hashMap->at(i).begin(); j != myMap->hashMap->at(i).end(); j++)
            {
                resSet.insert(*j);
            }
        }

        nThRoot(myVec, allNumbers, i, res, &cnt);

        for (int j = 0; j < cnt; j++)
        {
            resSet.insert(res[j]);
        }
        if (myMap->hashMap->count(i) != 0)
        {
            myMap->hashMap->at(i) = resSet;
        }
        else
        {
            myMap->hashMap->insert(make_pair(i, resSet));
        }
    }
}

/**
 * @brief - functia apelata de reduceri cand sunt creati.
 *          Efectueaza contorizarea numarului total de numere
 *          ce sunt puteri perfecte pentru numarul reprezentat de
 *          id-ul reducerului + 1
 *
 * @param arg - pointer la structura reducer
 */
void *reducer(void *arg)
{
    reducer_struct *myRed = (reducer_struct *)arg;
    pthread_barrier_wait(myRed->barrier);

    unordered_set<int> reducerSet;

    for (unsigned int i = 0; i < myRed->listOfAll->size(); i++)
    {
        unordered_map<int, unordered_set<int>> hashMap = myRed->listOfAll->at(i);

        for (auto key_v : hashMap)
        {
            int key = key_v.first;
            if (key == myRed->thread_id + 1)
            {
                for (const int &x : key_v.second)
                {
                    reducerSet.insert(x);
                }
            }
        }
    }

    pthread_mutex_lock(myRed->mutexRed);
    myRed->fileDest = "out" + to_string(myRed->thread_id + 1) + ".txt";
    pthread_mutex_unlock(myRed->mutexRed);

    writeFunction(myRed, reducerSet.size());
    return NULL;
}

/**
 * @brief - functia apelata de maperi cand sunt creati.
 *          Realizeaza asignarea de fisiere pentru thread-uri
 *          si apeleaza functia ce proceseaza fiecare fisier.
 *
 * @param arg - pointer la structura maper
 */
void *mapper(void *arg)
{
    mapper_struct *myMap = (mapper_struct *)arg;

    while (myMap->fileNames->size() > 0)
    {
        pthread_mutex_lock(myMap->mutex);

        if (myMap->fileNames->size() > 0)
        {
            myMap->processFileName = myMap->fileNames->at(0);
            myMap->fileNames->erase(myMap->fileNames->begin());

            pthread_mutex_unlock(myMap->mutex);
            processFunction(myMap);
        }
        else
        {
            pthread_mutex_unlock(myMap->mutex);
            break;
        }
    }
    pthread_mutex_lock(myMap->mutex);

    myMap->listOfAll->at(myMap->mapper_id) = *(myMap->hashMap);
    pthread_mutex_unlock(myMap->mutex);
    pthread_barrier_wait(myMap->barrier);
    return NULL;
}

/**
 * @brief - functia principala unde are loc crearea si stergerea thread-urilor
 *          precum si popularea anumitor campuri din structura de maper si reducer
 *
 * @param argv - vector unde se afla numarul de reduceri si de maperi + numele
 *              fisierului de test al temei
 */
int main(int argc, char *argv[])
{
    ifstream file;
    file.open(argv[3]);

    int err, numberOfFiles;
    file >> numberOfFiles;

    vector<string> processFileName;
    string name;
    vector<mapper_struct *> myMapFree;
    vector<reducer_struct *> myRedFree;

    for (int i = 0; i < numberOfFiles; i++)
    {
        file >> name;
        processFileName.push_back(name);
    }
    int mappers = atoi(argv[1]);
    int reducers = atoi(argv[2]);

    void *status;
    pthread_t mapperThreads[mappers], reducerThreads[reducers];

    pthread_mutex_t mutexInit, mutexInitRed;

    pthread_mutex_init(&mutexInit, NULL);
    pthread_mutex_init(&mutexInitRed, NULL);

    pthread_barrier_t barrierInit;

    pthread_barrier_init(&barrierInit, NULL, mappers + reducers);

    vector<unordered_map<int, unordered_set<int>>> *listOfAll = new vector<unordered_map<int, unordered_set<int>>>(mappers + 1);

    for (int i = 1; i <= mappers + reducers; i++)
    {
        if (i <= mappers)
        {
            mapper_struct *myMap = new mapper_struct();
            myMap->fileNames = new vector<string>();
            myMap->fileNames = &processFileName;
            myMap->mutex = &mutexInit;
            myMap->barrier = &barrierInit;
            myMap->mapper_id = i;
            myMap->totalReducers = reducers;
            myMap->listOfAll = listOfAll;
            myMap->hashMap = new unordered_map<int, unordered_set<int>>;
            myMapFree.push_back(myMap);

            err = pthread_create(&mapperThreads[i], NULL, mapper, myMap);
            if (err)
            {
                printf("Eroare la crearea thread-ului %d\n", i);
                exit(-1);
            }
        }

        else
        {
            reducer_struct *myRed = new reducer_struct();
            myRed->thread_id = i - mappers;
            myRed->mutexRed = &mutexInitRed;
            myRed->barrier = &barrierInit;
            myRed->listOfAll = listOfAll;
            myRedFree.push_back(myRed);

            err = pthread_create(&reducerThreads[i - mappers], NULL, reducer, myRed);
            if (err)
            {
                printf("Eroare la crearea thread-ului %d\n", i);
                exit(-1);
            }
        }
    }

    for (int i = 1; i <= mappers + reducers; i++)
    {
        if (i <= mappers)
        {
            err = pthread_join(mapperThreads[i], &status);
            if (err)
            {
                printf("Eroare la asteptarea thread-ului %d\n", i);
                exit(-1);
            }
        }
        else
        {
            err = pthread_join(reducerThreads[i - mappers], &status);
            if (err)
            {
                printf("Eroare la asteptarea thread-ului %d\n", i - mappers);
                exit(-1);
            }
        }
    }

    for (unsigned int i = 0; i < myMapFree.size(); i++)
    {

        myMapFree.at(i)->fileNames->clear();
        myMapFree.at(i)->hashMap->clear();
        delete myMapFree.at(i);
    }

    for (unsigned int i = 0; i < myRedFree.size(); i++)
    {
        myRedFree.at(i)->listOfAll->clear();
        delete myRedFree.at(i);
    }

    delete listOfAll;

    pthread_mutex_destroy(&mutexInit);
    pthread_mutex_destroy(&mutexInitRed);
    pthread_barrier_destroy(&barrierInit);
    return 0;
}