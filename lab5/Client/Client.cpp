#include <windows.h>
#include <conio.h>
#include <iostream>

using namespace std;

char READ = 'r';
char MODIFY = 'm';
char ORDER_NOT_FOUND = 'n';
char ORDER_FOUND = 'f';
char EXIT = 'e';
char END_OPERATION = 'd';
char END_MODIFY = 'n';

struct Order {
	int n;
	char name[10];
	int amount;
	double price;
};

int main(int argc, char* argv[]) {
	HANDLE writePipe = (HANDLE)atoi(argv[1]);
	HANDLE readPipe = (HANDLE)atoi(argv[2]);

	while (true) {
		DWORD bytesWrite;
		DWORD bytesRead;
		cout << "1) Read\n2) Modify\n3) Exit\n";
		int answer;
		cin >> answer;
		system("cls");
		if (answer == 1) {
			int orderNumber;
			cout << "Enter order number:\n";
			cin >> orderNumber;

			WriteFile(writePipe, &READ, sizeof(READ), &bytesWrite, NULL);
			WriteFile(writePipe, &orderNumber, sizeof(orderNumber), &bytesWrite, NULL);

			char serverAnswer;
			ReadFile(readPipe, &serverAnswer, sizeof(serverAnswer), &bytesRead, NULL);
			if (serverAnswer == ORDER_FOUND) {
				Order order;
				ReadFile(readPipe, &order, sizeof(order), &bytesRead, NULL);
				cout << "\nOrder name:\n" << order.name << "\nProduct count:\n" << order.amount << "\nProduct price:\n" << order.price << endl;
			} else {
				cout << "Order not found.\n";
			}

			WriteFile(writePipe, &END_OPERATION, sizeof(END_OPERATION), &bytesWrite, NULL);
		} else if (answer == 2) {
			int orderNumber;
			cout << "Enter order number:\n";
			cin >> orderNumber;

			WriteFile(writePipe, &READ, sizeof(READ), &bytesWrite, NULL);

			WriteFile(writePipe, &orderNumber, sizeof(orderNumber), &bytesWrite, NULL);

			char serverAnswer;
			ReadFile(readPipe, &serverAnswer, sizeof(serverAnswer), &bytesRead, NULL);

			if (serverAnswer == ORDER_FOUND) {
				Order order;

				ReadFile(readPipe, &order, sizeof(order), &bytesRead, NULL);

				WriteFile(writePipe, &END_OPERATION, sizeof(END_OPERATION), &bytesWrite, NULL);

				cout << "\nOrder name:\n" << order.name << "\nProduct count:\n" << order.amount << "\nProduct price:\n" << order.price << endl << endl;

				cout << "Enter new order name:\n";
				cin >> order.name;

				cout << "Enter new product count:\n";
				cin >> order.amount;

				cout << "Enter new product price:\n";
				cin >> order.price;

				WriteFile(writePipe, &MODIFY, sizeof(MODIFY), &bytesWrite, NULL);

				WriteFile(writePipe, &order, sizeof(order), &bytesWrite, NULL);
			} else {
				cout << "Order not found.\n";
			}

			char answer;
			ReadFile(readPipe, &answer, sizeof(answer), &bytesRead, NULL);

			WriteFile(writePipe, &END_OPERATION, sizeof(END_OPERATION), &bytesWrite, NULL);
		} else {
			WriteFile(writePipe, &EXIT, sizeof(EXIT), &bytesWrite, NULL);
			break;
		}
	}

	CloseHandle(writePipe);
	CloseHandle(readPipe);
	return 0;
}

