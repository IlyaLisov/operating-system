#include <iostream>

struct Message {
	char name[100];
	char text[100];

	Message();

	Message(const char* name, const char* text);
};