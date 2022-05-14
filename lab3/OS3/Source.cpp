#include <iostream>
#include <Windows.h>
#include "SyncQueue.h"

HANDLE start;

struct DataForProducer {
	SyncQueue* queue;
	int id;
	int amount;
};

struct DataForConsumer {
	SyncQueue* queue;
	int amount;
};

DWORD WINAPI producer(LPVOID parameter) {
	WaitForSingleObject(start, INFINITE);

	DataForProducer* producer = (DataForProducer*)parameter;

	for (int i = 0; i < producer->amount; i++) {
		producer->queue->Insert(producer->id * 100 + i);
		std::cout << "Произведено число: " << producer->id * 100 + i << "\n";
		Sleep(17);
	}

	return 0;
}

DWORD WINAPI consumer(LPVOID parameter) {
	WaitForSingleObject(start, INFINITE);

	DataForConsumer* data = (DataForConsumer*)parameter;

	for (int i = 0; i < data->amount; i++) {
		int number = data->queue->Remove();
		std::cout << "Употреблено число: " << number << "\n";
		Sleep(17);
	}

	return 0;
}

int main() {
	std::cout << "Enter queue size:\n";
	int queueSize;
	std::cin >> queueSize;

	start = CreateEvent(NULL, TRUE, FALSE, NULL);

	SyncQueue* queue = new SyncQueue(queueSize);

	std::cout << "Enter Producer amount:\n";
	int amountOfProducers;
	std::cin >> amountOfProducers;

	DataForProducer* producers = new DataForProducer[amountOfProducers];
	for (int i = 0; i < amountOfProducers; i++) {
		std::cout << "Enter amount to produce in " << i + 1 << " thread:\n";
		int amount;
		std::cin >> amount;
		producers[i].amount = amount;
		producers[i].id = i + 1;
		producers[i].queue = *&queue;
	}

	HANDLE* producerHandles = new HANDLE[amountOfProducers];
	DWORD* producerHandleIds = new DWORD[amountOfProducers];

	for (int i = 0; i < amountOfProducers; i++) {
		producerHandles[i] = CreateThread(NULL, 0, producer, (void*)&producers[i], 0, &producerHandleIds[i]);
	}


	std::cout << "Enter Consumer amount:\n";
	int amountOfConsumers;
	std::cin >> amountOfConsumers;

	DataForConsumer* consumers = new DataForConsumer[amountOfConsumers];
	for (int i = 0; i < amountOfConsumers; i++) {
		std::cout << "Enter amount of numbers to remove in " << i + 1 << " thread:\n";
		int amount;
		std::cin >> amount;
		consumers[i].amount = amount;
		consumers[i].queue = *&queue;
	}

	HANDLE* consumerHandles = new HANDLE[amountOfConsumers];
	DWORD* consumerHandleIds = new DWORD[amountOfConsumers];

	for (int i = 0; i < amountOfConsumers; i++) {
		consumerHandles[i] = CreateThread(NULL, 0, consumer, (void*)&consumers[i], 0, &consumerHandleIds[i]);
	}

	HANDLE* allHandles = new HANDLE[amountOfProducers + amountOfConsumers];

	for (int i = 0; i < amountOfProducers; i++) {
		allHandles[i] = producerHandles[i];
	}

	for (int i = 0; i < amountOfConsumers; i++) {
		allHandles[i + amountOfProducers] = consumerHandles[i];
	}

	SetEvent(start);

	WaitForMultipleObjects(amountOfProducers + amountOfConsumers, allHandles, TRUE, INFINITE);

	CloseHandle(start);

	for (int i = 0; i < amountOfProducers; i++) {
		CloseHandle(producerHandles[i]);
	}

	for (int i = 0; i < amountOfConsumers; i++) {
		CloseHandle(consumerHandles[i]);
	}

	return 0;
}