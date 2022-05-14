#include <iostream>
#include "windows.h"
#include <string>
#include <thread>

using namespace std;

struct Parameter {
	int size;
	double* coefficients;
	double x;
	double result = 0;
};

DWORD WINAPI ThreadProc(LPVOID lpParameters) {
	Parameter* parameter = (Parameter*)lpParameters;

	double result = 0;
	for (int i = 0; i < parameter->size; i++) {
		double addend = parameter->coefficients[i];
		for (int j = 0; j < parameter->size - i; j++) {
			addend *= parameter->x;
		}
		result += addend;
		Sleep(15);
	}

	parameter->result = result;
	cout << "\n" << "p(" << parameter->x << ") = " << result << "\n";
	return 0;
}

HANDLE startPolynomThread(Parameter& parameter) {
	DWORD lpThreadId;
	HANDLE handle;

	handle = CreateThread(NULL, 0, ThreadProc, (void*)&parameter, 0, &lpThreadId);
	if (!handle) {
		cout << "Поток не был создан." << "\n";
	}

	return handle;
}

int main() {
	cout << "Введите степень полинома в числителе:" << "\n";
	int Pn;
	cin >> Pn;

	cout << "\n" << "Введите коэффициенты полинома в числителе:" << "\n";
	double* Pcoefficients = new double[Pn];
	for (int i = 0; i < Pn; i++) {
		cin >> Pcoefficients[i];
	}

	cout << "\n" << "Введите степень полинома в знаменателе:" << "\n";
	int Qn;
	cin >> Qn;

	cout << "\n" << "Введите коэффициенты полинома в знаменателе:" << "\n";
	double* Qcoefficients = new double[Qn];
	for (int i = 0; i < Qn; i++) {
		cin >> Qcoefficients[i];
	}

	cout << "\n" << "Введите x:" << "\n";
	double x;
	cin >> x;

	Parameter parameter1;
	parameter1.x = x;
	parameter1.size = Pn;
	parameter1.coefficients = Pcoefficients;

	Parameter parameter2;
	parameter2.x = x;
	parameter2.size = Qn;
	parameter2.coefficients = Qcoefficients;

	HANDLE* handles = new HANDLE[2];
	handles[0] = startPolynomThread(parameter1);
	handles[1] = startPolynomThread(parameter2);

	WaitForMultipleObjects(2, handles, TRUE, INFINITE);

	CloseHandle(handles[0]);
	CloseHandle(handles[1]);

	cout << "f(" << x << ") = " << parameter1.result / parameter2.result;

	return 0;
}