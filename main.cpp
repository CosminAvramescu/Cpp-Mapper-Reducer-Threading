#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include <set>

using namespace std;

class Mapper;

class Reducer;

class Barrier;

class Barrier {
public:
    pthread_barrier_t barrier;
    pthread_mutex_t mutex;
    int M, R;
    char inputFilename[50];
    static inline int index;
    static inline int lastReadedFile;
    Mapper *mapper;
    Reducer *reducer;

    Barrier(int M, int R, Mapper *mapper, Reducer *reducer, char *inputFilename) {
        pthread_mutex_init(&mutex, nullptr);
        pthread_barrier_init(&barrier, nullptr, M + R);
        this->R = R;
        this->M = M;
        this->mapper = mapper;
        this->reducer = reducer;
        strcpy(this->inputFilename, inputFilename);
    }

    ~Barrier() = default;
};

class Mapper {
public:
    vector<vector<long>> lists;
    static inline int n;
    static inline vector<string> filenamesVector;

    Mapper() = default;

    ~Mapper() = default;

    static void init(char *filename) {
        string line;
        string file;
        ifstream input(filename);
        if (input.is_open()) {
            getline(input, line, '\n');
            Mapper::n = stoi(line);

            for (int k = 0; k < Mapper::n; k++) {
                Mapper::filenamesVector.emplace_back();
            }
            for (int k = 0; k < Mapper::n; k++) {
                getline(input, line, '\n');
                Mapper::filenamesVector[k] = line;
            }
            input.close();
        }
    }

    void parseFiles(const char *file, int R) {
        string line;
        ifstream input(file);
        if (input.is_open()) {
            getline(input, line, '\n');
            while (getline(input, line, '\n')) {
                isPerfectPower(stoi(line), R);
            }
            input.close();
        }
    }

    void isPerfectPower(int number, int R) {
        if (number > 0) {
            if (number == 1) {
                for (int i = 2; i <= R + 1; i++) {
                    lists[i].push_back(1);
                }
                return;
            }

            for (int j = 2; j <= R + 1; j++) {
                if (BS(j, 2, (int) sqrt(number), number) != -1) {
                    lists[j].push_back(number);
                }
            }
        }
    }

    int BS(int p, int left, int right, int x) {
        if (right >= left) {
            int mid = left + (right - left) / 2;

            if (pow(mid, p) == x)
                return mid;

            if (pow(mid, p) > x)
                return BS(p, left, mid - 1, x);

            return BS(p, mid + 1, right, x);
        }

        return -1;
    }
};

class Reducer {
public:
    vector<long> list;
    set<long> uniqueList;

    Reducer() = default;

    ~Reducer() = default;

    void init(Mapper *mapper, int k, int M) {
        for (int i = 0; i < M; i++) {
            for (long j: mapper[i].lists[k]) {
                this->list.push_back(j);
            }
        }
    }

    void addInSet() {
        for (long i: list) {
            this->uniqueList.insert(i);
        }
    }

    int countUniqueValues() const {
        return (int) uniqueList.size();
    }
};

void get_args(int argc, char **argv, int *M, int *R, char *filename) {
    if (argc < 4) {
        printf("Numar insuficient de parametri: ./tema1 M R filename\n");
        exit(1);
    }

    *M = stoi(argv[1]);
    *R = stoi(argv[2]);
    strcpy(filename, argv[3]);
}

void print(int R, Reducer *reducer) {
    FILE *out;
    string f = "out";
    string extension = ".txt";

    for (int i = 2; i < R + 2; i++) {
        string filename = f;
        filename += to_string(i) + extension;
        out = fopen((filename).c_str(), "w");
        fputs((to_string(reducer[i - 2].countUniqueValues())).c_str(), out);
        fclose(out);
    }
}

void *thread_function_mapper(void *arg) {
    auto *b = (Barrier *) arg;

    pthread_mutex_lock(&b->mutex);
    int threadIndex = Barrier::index;
    Barrier::index++;
    long unsigned int lastReadedFile = Barrier::lastReadedFile;
    Barrier::lastReadedFile++;
    pthread_mutex_unlock(&b->mutex);

    while (lastReadedFile < Mapper::filenamesVector.size()) {
        b->mapper[threadIndex].parseFiles(Mapper::filenamesVector
                                    .at(lastReadedFile).c_str(), b->R);
        if ((long unsigned int) b->M == Mapper::filenamesVector.size()) {
            lastReadedFile = Mapper::filenamesVector.size();
        } else {
            if (lastReadedFile < Mapper::filenamesVector.size()) {
                pthread_mutex_lock(&b->mutex);
                lastReadedFile = Barrier::lastReadedFile;
                Barrier::lastReadedFile++;
                pthread_mutex_unlock(&b->mutex);
                continue;
            } else {
                break;
            }
        }
    }

    pthread_barrier_wait(&b->barrier);
    pthread_exit(nullptr);
}

void *thread_function_reducer(void *arg) {
    auto *b = (Barrier *) arg;
    pthread_barrier_wait(&b->barrier);

    pthread_mutex_lock(&b->mutex);
    int index = Barrier::index;
    Barrier::index++;
    pthread_mutex_unlock(&b->mutex);

    //pana in acest punct, index va avea valoarea lui M (nr de mapperi)
    //pentru a accesa indecsii din vectorul de reduceri (care pleaca de la 0), fac index-nr de mapperi
    b->reducer[index - b->M].init(b->mapper, index - b->M + 2, b->M);
    b->reducer[index - b->M].addInSet();
    print(b->R, b->reducer);

    pthread_exit(nullptr);
}

int main(int argc, char *argv[]) {
    int M, R;
    char filename[50];

    get_args(argc, argv, &M, &R, filename);

    Mapper mapper[M];
    Reducer reducer[R];
    Barrier barrier(M, R, mapper, reducer, filename);

    Mapper::init(filename);

    for (int i = 0; i < M; i++) {
        for (int j = 0; j <= R + 1; j++) {
            mapper[i].lists.emplace_back();
        }
    }

    pthread_t tid[M + R];
    //se creeaza thread-urile
    for (int i = 0; i < M + R; i++) {
        if (i < M) {
            pthread_create(&tid[i], nullptr, thread_function_mapper, &barrier);
        } else {
            pthread_create(&tid[i], nullptr, thread_function_reducer, &barrier);
        }
    }

    // se asteapta thread-urile
    for (int i = 0; i < M + R; i++) {
        pthread_join(tid[i], nullptr);
    }

    for (int k = 0; k < M; k++) {
        for (long unsigned int i = 2; i < mapper[k].lists.size(); i++) {
            cout << i << ": ";
            for (int j: mapper[k].lists[i]) {
                cout << j << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    return 0;
}

