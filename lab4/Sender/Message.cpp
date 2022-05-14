#include <iostream>
#include "Message.h"
#pragma warning(disable : 4996)

Message::Message() {
	strcpy(name, "");
	strcpy(text, "");
}

Message::Message(const char* name, const char* text) {
	strcpy(this->name, name);
	strcpy(this->text, text);
}