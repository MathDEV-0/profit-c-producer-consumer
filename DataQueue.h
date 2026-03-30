#include <stdbool.h>
#include <stdio.h>
#define MAX_SIZE 100

typedef struct { 
    double value; 
    int is_forecast; 
} Data;

// Defining the Queue structure
typedef struct
{
    Data items[MAX_SIZE];
    int front;
    int rear;
} Queue;

// Function to initialize the queue
void initializeQueue(Queue *q)
{
    q->front = -1;
    q->rear = 0;
}

// Function to check if the queue is empty
bool isEmpty(Queue *q)
{
    return (q->front == q->rear - 1);
}

// Function to check if the queue is full
bool isFull(Queue *q)
{
    return (q->rear == MAX_SIZE);
}

// Function to add an element to the queue (Enqueue
// operation)
void enqueue(Queue *q, Data value)
{
    if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }
    q->items[q->rear] = value;
    q->rear++;
}

// Function to remove an element from the queue (Dequeue
// operation)
void dequeue(Queue *q)
{
    if (isEmpty(q))
    {
        printf("Queue is empty\n");
        return;
    }
    q->front++;
}

// Function to get the element at the front of the queue
// (Peek operation)
Data peek(Queue *q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        Data d = {0, 0};
        return d;
    }
    return q->items[q->front + 1];
}

// Function to print the current queue
void printQueue(Queue *q)
{
    if (isEmpty(q))
    {
        printf("Queue is empty\n");
        return;
    }

    printf("Current Queue: ");
    for (int i = q->front + 1; i < q->rear; i++)
    {
        printf("%f ", q->items[i].value);
    }
    printf("\n");
}