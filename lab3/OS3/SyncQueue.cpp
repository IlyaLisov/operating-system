#include "SyncQueue.h"

SyncQueue::SyncQueue(int size) {
	this->size = size;
	this->array = new int[size];
	this->insertPosition = 0;
	this->removePosition = 0;

	availableToDelete = CreateSemaphore(NULL, 0, size, NULL);
	availableToAdd = CreateSemaphore(NULL, size, size, NULL);
	InitializeCriticalSection(&criticalSection);
}

SyncQueue::~SyncQueue() {
	DeleteCriticalSection(&criticalSection);
	CloseHandle(availableToAdd);
	CloseHandle(availableToDelete);
	delete[] array;
}

void SyncQueue::Insert(int element) {
	WaitForSingleObject(availableToAdd, INFINITE);

	EnterCriticalSection(&criticalSection);
	array[insertPosition++] = element;
	if (insertPosition == size)
	{
		insertPosition = 0;
	}
	LeaveCriticalSection(&criticalSection);

	ReleaseSemaphore(availableToDelete, 1, NULL);
}

int SyncQueue::Remove() {
	WaitForSingleObject(availableToDelete, INFINITE);

	EnterCriticalSection(&criticalSection);
	int element = array[removePosition++];
	if (removePosition == size)
	{
		removePosition = 0;
	}
	LeaveCriticalSection(&criticalSection);

	ReleaseSemaphore(availableToAdd, 1, NULL);

	return element;
}