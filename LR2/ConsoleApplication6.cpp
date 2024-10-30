#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "pthread.h"
#include <vector>

#pragma comment(lib,"pthreadVCE2.lib")

using namespace std;

int findNOD(long a, long b);

typedef struct {
    int a;
    int b;
    int id;
    int available;
} Task;

const int availableThreads = 8;
const string input = "input.txt";
const string output = "output.txt";
// Общее количество доступных потоков-исполнителей
pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_cond = PTHREAD_COND_INITIALIZER;
Task tasks[availableThreads];


// Функция, выполняемая исполнителями
void* worker_function(void* arg) {
    Task* task = (Task*)arg;

    while (1) {
        pthread_mutex_lock(&task_mutex);

        // Ожидаем, пока задача не станет доступной
        while (!task->available) {
            pthread_cond_wait(&task_cond, &task_mutex);
        }

        // Проверяем, если это сигнал завершения
        if (task->a == -1 && task->b == -1) {
            pthread_mutex_unlock(&task_mutex);
            break;
        }

        // Выполняем задачу
        int result = findNOD(task->a, task->b);

        // Создаем файл для записи результата
        char filename[20];
        sprintf(filename, "result_%d.txt", task->id);
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%d, %d: %d\n", task->a, task->b, result);
            fclose(file);
        }
        else {
            perror("Не удалось открыть файл");
        }

        // Помечаем задачу как выполненную
        task->available = 0;

        pthread_mutex_unlock(&task_mutex);
    }
    return NULL;
}


int main() {
    pthread_t workers[availableThreads];
    for (int i = 0; i < availableThreads; i++) {
        tasks[i].id = i;
        tasks[i].available = 0;
        pthread_create(&workers[i], NULL, worker_function, (void*)&tasks[i]);
    }

    FILE* file = fopen("input.txt", "r");
    char inputline[256];

    if (file) {
        int a, b;
        while (fgets(inputline, sizeof(inputline), file)) {
            string temp = inputline;
            istringstream line(temp);
            while (line) {
                line >> a >> b;
                int assigned = 0;
                while (!assigned) {
                    for (int i = 0; i < availableThreads; ++i) {
                        if (!tasks[i].available) {
                            tasks[i].a = a;
                            tasks[i].b = b;
                            tasks[i].available = 1;
                            assigned = 1;
                            pthread_cond_signal(&task_cond);
                            break;
                        }
                    }
                    if (!assigned) {
                        pthread_cond_wait(&task_cond, &task_mutex);
                    }
                }

                pthread_mutex_unlock(&task_mutex);
            }
        }
    }

    // Отправляем сигнал завершения всем потокам
    pthread_mutex_lock(&task_mutex);
    for (int i = 0; i < availableThreads; ++i) {
        tasks[i].a = -1;
        tasks[i].b = -1;
        tasks[i].available = 1;
    }
    pthread_cond_broadcast(&task_cond);
    pthread_mutex_unlock(&task_mutex);

    // Ожидаем завершения всех потоков
    for (int i = 0; i < availableThreads; ++i) {
        pthread_join(workers[i], NULL);
    }

    return 0;
}

int findNOD(long a, long b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}