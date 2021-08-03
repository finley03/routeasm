// utility file to be included everywhere

#ifndef UTIL_H
#define UTIL_H

// standard library includes

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <map>

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4996)
#endif

// type definitions

#if _WIN64
#define _ENV64
#else
#if _WIN32
#define _ENV32
#else
#error UNKNOWN_WORD_SIZE
#endif
#endif

// some type macros
#ifdef _ENV64
typedef int64_t INT_T;
typedef uint64_t UINT_T;
typedef double FLT_T;
#define INT_T_MAX INT64_MAX
#define UINT_T_MAX UINT64_MAX
#define INT_T_MIN INT64_MIN
#define UINT_T_MIN UINT64_MIN
#endif

#ifdef _ENV32
typedef int32_t INT_T;
typedef uint32_t UINT_T;
typedef float FLT_T;
#define INT_T_MAX INT32_MAX
#define UINT_T_MAX UINT32_MAX
#define INT_T_MIN INT32_MIN
#define UINT_T_MIN UINT32_MIN
#endif

#define _INT(a) static_cast<INT_T>(a)
#define _UINT(a) static_cast<UINT_T>(a)

// some fixed width casting macros
#define _I32(a) static_cast<int32_t>(a)
#define _U32(a) static_cast<uint32_t>(a)
#define _F32(a) static_cast<float>(a)
#define _I64(a) static_cast<int64_t>(a)
#define _U64(a) static_cast<uint64_t>(a)
#define _F64(a) static_cast<double>(a)

// define some comparison macros

#define ABS(a) ((a < 0) ? -(a) : a)
#define MAX_2(a, b) ((a > b) ? a : b)
#define MIN_2(a, b) ((b > a) ? a : b)
#define MAX_3(a, b, c) (MAX_2(MAX_2(a, b), c))
#define MIN_3(a, b, c) (MIN_2(MIN_2(a, b), c))
#define UMAX_2(a, b) MAX_2(ABS(a), ABS(b))
#define UMIN_2(a, b) MIN_2(ABS(a), ABS(b))
#define UMAX_3(a, b, c) MAX_3(ABS(a), ABS(b), ABS(c))
#define UMIN_3(a, b, c) MIN_3(ABS(a), ABS(b), ABS(c))

typedef union {
	float value;

	uint8_t reg[4];
} Float_Converter;

// Reads file given in const char* path
// to C++ std::string. Uses container type
// because returning dynamically allocated arrays
// is strange at best.
//template <typename T>
//bool readFileToString(T filePath, std::string& writeback);
template <typename T>
bool readFileToString(T filePath, std::string& writeback) {
	// open filestream using standard library
	std::ifstream f(filePath);
	// check if file successfully opened
	// therefore check if the file exists
	if (!f.is_open()) {
		std::cout << "Error: could not find file: " << filePath << "\n";
		return false;
	}
	// use stringstream as a go between
	std::stringstream buffer;
	buffer << f.rdbuf();
	// close file
	f.close();
	// convert to string and populate writeback variable
	writeback = buffer.str();

	return true;
}


template <typename T>
void writeStringToFile(T filePath, std::string& string) {
	// create file
	std::ofstream f(filePath, std::ofstream::trunc);
	f << string;
	f.close();
}


template <typename T1, typename T2>
void writeDataToFile(T1 filePath, uint8_t* data, T2 size) {
	std::ofstream f(filePath, std::ofstream::trunc);
	f.write((const char*)data, size);
	f.close();
}


// check if c++ indexable container type contains value
template <typename T1, typename T2>
bool contains(T1& container, T2 value) {
	return std::find(container.begin(), container.end(), value) != container.end();
}


template <typename T1, typename T2>
T1 round(T1 value, T2 multiple) {
	std::cout << value << "  " << multiple << "\n";
	INT_T base_multiplier = value / multiple;
	T1 round_down = static_cast<T1>(base_multiplier) * multiple;
	T1 round_up = round_down + multiple;
	T1 result = (value - round_down < round_up - value) ? round_down : round_up;
	std::cout << result << "\n";
	return result;
}

#endif