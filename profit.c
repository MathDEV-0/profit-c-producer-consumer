#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "DataQueue.h"
#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
#endif

#define BUFFER_LEN  200
#define PLOT_SIZE   50
#define REAL_WINDOW 3
void *p_func(void *args);
void *c_func(void *args);
void *f_func(void *args);
// void *plot_func(void *args);
void write_csv(Data d);

Queue *g_data_queue;
int pipe_fd[2];

typedef struct {
    double value;
    int is_forecast;
} PlotData;

typedef struct {
    double values[REAL_WINDOW];
    int count;        
    int index;        
} RealBuffer;
RealBuffer real_buffer = { .count = 0, .index = 0 };

PlotData plot_buffer[PLOT_SIZE];
int plot_index = 0;
FILE *csv_file;

pthread_mutex_t plot_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t is_full = PTHREAD_COND_INITIALIZER;



int main(int argc, char *argv[]){
    #ifdef _WIN32
        if (_pipe(pipe_fd, sizeof(Data) * 10, _O_BINARY) == -1) {
            perror("_pipe");
            exit(EXIT_FAILURE);
        }
    #else
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    #endif

    pthread_t producer_t;
    pthread_t consumer_t;
    pthread_t forecast_t;
    pthread_t plot_t;
    
    g_data_queue = malloc(sizeof(Queue));
    initializeQueue(g_data_queue);
    
    int res;
    void *thread_result;
    
    srand(time(NULL));

    csv_file = fopen("output.csv", "w");

    if (!csv_file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(csv_file, "timestamp,value,type\n");
    fflush(csv_file);

    pthread_create(&producer_t,NULL,p_func,NULL);
    pthread_create(&consumer_t,NULL,c_func,NULL);
    pthread_create(&forecast_t,NULL,f_func,NULL);
    // pthread_create(&plot_t, NULL, plot_func, NULL);
    
    pthread_join(producer_t,NULL);
    pthread_join(consumer_t,NULL);
    pthread_join(forecast_t,NULL);
    // pthread_join(plot_t,NULL);

    fclose(csv_file); 
    return 0;
}

void *p_func(void *args){
    while(1){
        pthread_mutex_lock(&mutex);
        while(isFull(g_data_queue)){
            pthread_cond_wait(&is_full, &mutex);
        }
        Data d;
        static double last_value = 50.0; 

        double variation = (rand() % 11 - 5); 
        d.value = last_value + variation;

        // evita sair de faixa
        if (d.value < 0) d.value = 0;
        if (d.value > 100) d.value = 100;

        last_value = d.value;
        printf("Producing: %f\n", d.value);
        d.is_forecast = 0;
        enqueue(g_data_queue, d);
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        usleep(500000);
    }
}

void *c_func(void *args){
    while(1){
        pthread_mutex_lock(&mutex);
        while(isEmpty(g_data_queue)){
            pthread_cond_wait(&not_empty, &mutex);
        }

        Data d = peek(g_data_queue);
        dequeue(g_data_queue);
        
        printf("Consumed: %f (%s)\n",d.value,d.is_forecast ? "forecast" : "real");
        write_csv(d);
        pthread_mutex_lock(&plot_mutex);
        plot_buffer[plot_index % PLOT_SIZE].value = d.value;
        plot_buffer[plot_index % PLOT_SIZE].is_forecast = d.is_forecast;
        plot_index++;
        pthread_mutex_unlock(&plot_mutex);
        pthread_cond_signal(&is_full);
        pthread_mutex_unlock(&mutex);
        if (!d.is_forecast) {
            write(pipe_fd[1], &d, sizeof(Data));
        }
        usleep(500000);
    }
}
void *f_func(void *args){
    Data d;

    while(1){
        read(pipe_fd[0], &d, sizeof(Data));

        real_buffer.values[real_buffer.index] = d.value;
        real_buffer.index = (real_buffer.index + 1) % REAL_WINDOW;
        if (real_buffer.count < REAL_WINDOW)real_buffer.count++;

        if (real_buffer.count >= 2) {
            double sum = 0;
            for (int i = 0; i < real_buffer.count; i++) {
                sum += real_buffer.values[i];
            }
            //Precisa de 2 valores na janela, para gerar um predict (média móvel)
            double forecast_value = sum / real_buffer.count; 

            Data f;
            f.value = forecast_value;
            f.is_forecast = 1;
            pthread_mutex_lock(&plot_mutex);

            int future_index = (plot_index + 1) % PLOT_SIZE;

            plot_buffer[future_index].value = forecast_value;
            plot_buffer[future_index].is_forecast = 1;

            pthread_mutex_unlock(&plot_mutex);
            
            pthread_mutex_lock(&mutex);
            while (isFull(g_data_queue)) {
                pthread_cond_wait(&is_full, &mutex);
            }
            enqueue(g_data_queue, f);
            pthread_cond_signal(&not_empty);
            pthread_mutex_unlock(&mutex);
        }
        usleep(500000);
    }   
}
// void *plot_func(void *args){
//     while(1){
//         pthread_mutex_lock(&plot_mutex);

//         #ifdef _WIN32
//             system("cls");
//         #else
//             system("clear");
//         #endif

//         printf("=== GRAPH ===\n");

//         for(int i = 0; i < PLOT_SIZE; i++){
//             double v = plot_buffer[i].value;
//             int bars = (int)v;

//             if (bars > 50) bars = 50;

//             printf("%5.1f | ", v);

//             for(int j = 0; j < bars; j++){
//                 printf("#");
//             }

//             if(plot_buffer[i].is_forecast){
//                 printf("  <-- forecast *");
//             }

//             printf("\n");
//         }

//         pthread_mutex_unlock(&plot_mutex);

//         sleep(1);
//     }
// }

void write_csv(Data d) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    pthread_mutex_lock(&file_mutex);

    fprintf(csv_file, "%ld.%09ld,%f,%s\n",
            ts.tv_sec,
            ts.tv_nsec,
            d.value,
            d.is_forecast ? "forecast" : "real");

    fflush(csv_file);

    pthread_mutex_unlock(&file_mutex);
}