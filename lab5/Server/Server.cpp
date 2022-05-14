#include <iostream>
#include <fstream>
#include <Windows.h>

#pragma warning(disable : 4996)

using namespace std;

char READ = 'r';
char MODIFY = 'm';
char ORDER_NOT_FOUND = 'n';
char ORDER_FOUND = 'f';
char EXIT = 'e';
char END_OPERATION = 'd';
char END_MODIFY = 'n';
char filename[20];

volatile int readerCount = 0;
CRITICAL_SECTION cs;
HANDLE semaphore;

struct Order {
	int n;
	char name[10];
	int amount;
	double price;
};

string GetExeFileName() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	return std::string(buffer);
}

string GetExePath() {
	std::string f = GetExeFileName();
	return f.substr(0, f.find_last_of("\\/"));
}

Order* findOrder(int orderNumber) {
	ifstream in(filename, ios::binary);
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
	fstream f(filename, ios::binary | ios::in | ios::out);
	int pos = 0;
	int orderSize = sizeof(Order);
	while (!f.eof())
	{
		Order o;
		f.read((char*)&o, sizeof(Order));
		if (order.n == o.n) {
			f.seekp(pos * orderSize, ios::beg);
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
		cout << "Create pipe failed.\n";
		system("pause");
		return 0;
	}

	if (!CreatePipe(&clientReadPipe, &writePipe, &sa, 0)) {
		cout << "Create pipe failed.\n";
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
		cout << "The new process is not created.\n";
		return 0;
	}

	while (true) {
		DWORD readBytes;
		DWORD writeBytes;

		char inputCommand;
		ReadFile(readPipe, &inputCommand, sizeof(inputCommand), &readBytes, NULL);

		if (inputCommand == READ) {
			EnterCriticalSection(&cs);
			++readerCount;
			if (readerCount == 1) {
				WaitForSingleObject(semaphore, INFINITE);
			}
			LeaveCriticalSection(&cs);

			int orderNumber;
			ReadFile(readPipe, &orderNumber, sizeof(int), &readBytes, NULL);
			Order* order = findOrder(orderNumber);

			if (order != nullptr) {
				WriteFile(writePipe, &ORDER_FOUND, sizeof(ORDER_FOUND), &writeBytes, NULL);
				WriteFile(writePipe, order, sizeof(Order), &writeBytes, NULL);
			}
			else {
				WriteFile(writePipe, &ORDER_NOT_FOUND, sizeof(ORDER_NOT_FOUND), &writeBytes, NULL);
			}

			char end;
			ReadFile(readPipe, &end, sizeof(end), &readBytes, NULL);

			EnterCriticalSection(&cs);
			--readerCount;
			if (readerCount == 0) {
				ReleaseSemaphore(semaphore, 1, NULL);
			}
			LeaveCriticalSection(&cs);
		}
		else if (inputCommand == MODIFY) {
			WaitForSingleObject(semaphore, INFINITE);

			Order order;
			ReadFile(readPipe, &order, sizeof(Order), &readBytes, NULL);
			modify(order);
			WriteFile(writePipe, &END_MODIFY, sizeof(END_MODIFY), &writeBytes, NULL);

			char end;
			ReadFile(readPipe, &end, sizeof(end), &readBytes, NULL);

			ReleaseSemaphore(semaphore, 1, NULL);
		} else {
			break;
		}
	}
	return 0;
}

void createBinaryFile() {
	ofstream out(filename, ios::binary);

	cout << "How many orders you want write?\n";
	int num;
	cin >> num;

	for (int i = 0; i < num; i++) {
		cout << "(" << i+1 << "/" << num << ")\n";
		Order order;
		cout << "Enter order number:\n";
		cin >> order.n;

		cout << "Enter order name:\n";
		cin >> order.name;

		cout << "Enter product count:\n";
		cin >> order.amount;

		cout << "Enter product price:\n";
		cin >> order.price;

		out.write((char*)&order, sizeof(struct Order));
		system("cls");
	}

	out.close();
}

void checkDataInBinaryFile()
{
	ifstream in(filename, ios::binary);
	cout << "Orders from file after writing:\n";
	while (!in.eof())
	{
		Order order;
		in.read((char*)&order, sizeof(Order));
		cout << "Order number:\n" << order.n << "\nOrder name:\n" << order.name << "\nProduct count:\n" << order.amount << "\nProduct price:\n" << order.price << endl << endl;
	}

	in.close();
}

void main() {
	InitializeCriticalSection(&cs);
	semaphore = CreateSemaphore(NULL, 1, 1, NULL);

	cout << "Enter file order name:\n";
	cin >> filename;

	createBinaryFile();
	checkDataInBinaryFile();

	cout << "Enter client count:\n";
	int clientCount;
	cin >> clientCount;
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