#pragma once
#include <string>
#include <typeinfo>

namespace Lamb {
	template<class T>
	std::string ToString() {
        std::string name = typeid(T).name();

        const std::string classPrefix = "class ";
        const std::string structPrefix = "struct ";

        if (name.find(classPrefix) == 0) {
            name = name.substr(classPrefix.size());
        }
        else if (name.find(structPrefix) == 0) {
            name = name.substr(structPrefix.size());
        }

		return name;
	}
}