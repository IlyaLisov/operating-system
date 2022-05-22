#include <windows.h>
#include <conio.h>
#include <iostream>
#include "Order.h"

using namespace std;

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
			}
			else {
				cout << "Order not found.\n";
			}
			char c;
			cout << "Press any char to finish read: ";
			cin >> c;
			WriteFile(writePipe, &END_OPERATION, sizeof(END_OPERATION), &bytesWrite, NULL);
		}
		else if (answer == 2) {
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
				cout << "Enter order name:\n";
				cin >> order.name;
				cout << "Enter product count:\n";
				cin >> order.amount;
				cout << "Enter product price:\n";
				cin >> order.price;
				WriteFile(writePipe, &MODIFY, sizeof(MODIFY), &bytesWrite, NULL);
				WriteFile(writePipe, &order, sizeof(order), &bytesWrite, NULL);
			}
			else {
				cout << "Order not found.\n";
			}
			char answer;
			ReadFile(readPipe, &answer, sizeof(answer), &bytesRead, NULL);
			char c;
			cout << "Press any char to finish modify: ";
			cin >> c;
			WriteFile(writePipe, &END_OPERATION, sizeof(END_OPERATION), &bytesWrite, NULL);
		}
		else {
			WriteFile(writePipe, &EXIT, sizeof(EXIT), &bytesWrite, NULL);
			break;
		}
	}

	CloseHandle(writePipe);
	CloseHandle(readPipe);
	return 0;
}
