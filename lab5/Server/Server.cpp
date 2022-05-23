#include <iostream>
#include <fstream>
#include <Windows.h>
#include "Order.h"

#pragma warning(disable : 4996)

volatile int readerCount = 0;
volatile int modifierCount = 0;
CRITICAL_SECTION cs;
HANDLE semaphore;

std::string GetExeFileName() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	return std::string(buffer);
}

std::string GetExePath() {
	std::string f = GetExeFileName();
	return f.substr(0, f.find_last_of("\\/"));
}

Order* findOrder(int orderNumber) {
	std::ifstream in(filename, std::ios::binary);
	while (!in.eof()) {
		Order* order = new Order;
		in.read((char*)order, sizeof(Order));
		if (order->n == orderNumber) {
			in.close();
			return order;
		}
	}

	in.close();

	return nullptr;
}

void modify(Order order) {
	std::fstream f(filename, std::ios::binary | std::ios::in | std::ios::out);
	int pos = 0;
	int orderSize = sizeof(Order);
	while (!f.eof())
	{
		Order o;
		f.read((char*)&o, sizeof(Order));
		if (order.n == o.n) {
			f.seekp(pos * orderSize, std::ios::beg);
			f.write((char*)&order, sizeof(Order));
			f.close();
			return;
		}
		else {
			pos++;
		}
	}
}

DWORD WINAPI client(LPVOID data) {
	HANDLE writePipe;
	HANDLE readPipe;
	HANDLE clientReadPipe;
	HANDLE clientWritePipe;
	SECURITY_ATTRIBUTES sa;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (!CreatePipe(&readPipe, &clientWritePipe, &sa, 0)) {
		std::cout << "Create pipe failed.\n";
		system("pause");
		return 0;
	}

	if (!CreatePipe(&clientReadPipe, &writePipe, &sa, 0)) {
		std::cout << "Create pipe failed.\n";
		system("pause");
		return 0;
	}

	char path[333];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	sprintf(path, "Client %d %d", (int)clientWritePipe, (int)clientReadPipe);
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if (!CreateProcess(NULL, path, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		std::cout << "The new process is not created.\n";
		return 0;
	}

	while (true) {
		DWORD bytesRead;
		DWORD bytesWrite;
		char clientWant;
		ReadFile(readPipe, &clientWant, sizeof(clientWant), &bytesRead, NULL);
		if (clientWant == READ) {
			EnterCriticalSection(&cs);
			++readerCount;
			if (readerCount == 1) {
				WaitForSingleObject(semaphore, INFINITE);
			}
			LeaveCriticalSection(&cs);
			int orderNumber;
			ReadFile(readPipe, &orderNumber, sizeof(int), &bytesRead, NULL);
			Order* order = findOrder(orderNumber);
			if (order != nullptr) {
				WriteFile(writePipe, &ORDER_FOUND, sizeof(ORDER_FOUND), &bytesWrite, NULL);
				WriteFile(writePipe, order, sizeof(Order), &bytesWrite, NULL);
			}
			else {
				WriteFile(writePipe, &ORDER_NOT_FOUND, sizeof(ORDER_NOT_FOUND), &bytesWrite, NULL);
			}
			char end;
			ReadFile(readPipe, &end, sizeof(end), &bytesRead, NULL);
			EnterCriticalSection(&cs);
			--readerCount;
			if (readerCount == 0) {
				ReleaseSemaphore(semaphore, 1, NULL);
			}
			LeaveCriticalSection(&cs);
		} else if (clientWant == MODIFY) {
			WaitForSingleObject(semaphore, INFINITE);
			Order order;
			ReadFile(readPipe, &order, sizeof(Order), &bytesRead, NULL);
			modify(order);
			WriteFile(writePipe, &END_MODIFY, sizeof(END_MODIFY), &bytesWrite, NULL);
			char end;
			ReadFile(readPipe, &end, sizeof(end), &bytesRead, NULL);
			ReleaseSemaphore(semaphore, 1, NULL);
		} else {
			break;
		}
	}
	return 0;
}

void createBinaryFile() {
	std::ofstream out(filename, std::ios::binary);

	std::cout << "How many orders you want write?\n";
	int num;
	std::cin >> num;

	for (int i = 0; i < num; i++) {
		std::cout << "(" << i + 1 << "/" << num << ")\n";
		Order order;
		std::cout << "Enter order number:\n";
		std::cin >> order.n;

		std::cout << "Enter order name:\n";
		std::cin >> order.name;

		std::cout << "Enter product count:\n";
		std::cin >> order.amount;

		std::cout << "Enter product price:\n";
		std::cin >> order.price;

		out.write((char*)&order, sizeof(struct Order));
		system("cls");
	}

	out.close();
}

void checkDataInBinaryFile() {
	std::ifstream in(filename, std::ios::binary);
	std::cout << "Orders from file after writing:\n";
	while (!in.eof()) {
		Order order;
		in.read((char*)&order, sizeof(Order));
		std::cout << "Order number:\n" << order.n << "\nOrder name:\n" << order.name << "\nProduct count:\n" << order.amount << "\nProduct price:\n" << order.price << std::endl << std::endl;
	}

	in.close();
}

void main() {
	InitializeCriticalSection(&cs);
	semaphore = CreateSemaphore(NULL, 1, 1, NULL);

	std::cout << "Enter file order name:\n";
	std::cin >> filename;

	createBinaryFile();
	checkDataInBinaryFile();

	std::cout << "Enter client count:\n";
	int clientCount;
	std::cin >> clientCount;
	HANDLE* handles = new HANDLE[clientCount];
	DWORD* ID = new DWORD[clientCount];
	for (int i = 0; i < clientCount; i++) {
		handles[i] = CreateThread(NULL, NULL, client, (void*)i, 0, &ID[i]);
	}

	WaitForMultipleObjects(clientCount, handles, TRUE, INFINITE);
	checkDataInBinaryFile();

	for (int i = 0; i < clientCount; i++) {
		CloseHandle(handles[i]);
	}

	system("pause");
}