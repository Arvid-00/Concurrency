#pragma once
#ifndef INC_VECTOR
#define INC_VECTOR
#include <stdlib.h>
#include <string>
#include <iostream>
#include <type_traits>
#include <vector>
#include <thread>

class IncVec {

	//template<class T>
	void IncrementVector(std::vector<int>& v)
	{
		std::cout << "Incrementing vector...." << std::endl;
		for (int i = 0; i < v.size(); i++)
			v[i]++;
	}

};

#endif INC_VECTOR