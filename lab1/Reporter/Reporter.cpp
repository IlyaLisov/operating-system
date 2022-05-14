#include <windows.h>
#include <iostream>
#include <fstream>
#include <list>

#pragma warning(disable : 4996)

using namespace std;

struct Employee
{
	int num;
	char name[10];
	double hours;
};

list<Employee> readEmployees(char* fileName) {
	ifstream in(fileName, ios::binary);
	list<Employee> employees;
	
	while (in.peek() != EOF)
	{
		Employee employee;
		in.read((char*)&employee, sizeof(Employee));
		employees.push_back(employee);
	}

	in.close();

	return employees;
}

bool employeeComparator(const Employee& employee1, const Employee& employee2) {
	return employee1.num < employee2.num;
}

int main(int argc, char* argv[])
{
	ofstream fout(argv[2]);
	setlocale(LC_ALL, "Russian");
	double pay = atof(argv[3]);

	list<Employee> employees = readEmployees(argv[1]);
	employees.sort(employeeComparator);

	fout << "Отчет по файлу \"" << argv[0] << "\"\n";
	for (auto const& iterator : employees) {
		fout << iterator.num << " " << iterator.name << " " << iterator.hours << " " << (iterator.hours * pay) << "\n";
	}

	fout.close();
	system("pause");
	return 1;
}