// Sloth.h

#pragma once

namespace Sloth {

	bool ReadRegInfo(const char regName[], const char field[], char* data);
	bool WriteRegInfo(const char regName[], const char field[] , const char data[]);

}
