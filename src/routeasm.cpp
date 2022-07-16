#include "routeasm.h"


#ifndef AUTOPILOT_INTERFACE
void printHelp();
bool assemblefile(std::string inputfile, std::string outputfile);
#else
bool assemblefile(std::string inputfile, std::string filestring, uint8_t*& dataptr, int& datasize);
#endif
//void showMessage(std::string filepath, const char* msg, INT_T line = -1);


#ifdef AUTOPILOT_INTERFACE
std::string compileLog;
#endif

float gnss_zerolat, gnss_zerolong;
bool gnss_zero_defined;

template <typename T1, typename T2>
bool compare(T1 str1, T2 str2) {
	return strcmp(str1, str2) == 0;
}

// AUTOPILOT_INTERFACE option compiles file differently if it is contained in program

#ifndef AUTOPILOT_INTERFACE
int main(int argc, char** argv) {
	INT_T ret = 0;
	std::string inputfile;
	std::string outputfile = "a.bin";

	INT_T i = 1;
	while (i < argc) {
		switch (*argv[i]) {
		case '-':
			if (*(argv[i] + 1) == '-') {
				if (compare(argv[i], "--help")) {
					printHelp();
					goto end;
				}
			}
			else {
				if (compare(argv[i], "-o")) {
					if (++i < argc) {
						outputfile = argv[i];
					}
					else {
						std::cout << "Error: no output file specified\n";
						ret = -1;
						goto end;
					}
				}
				else if (compare(argv[i], "-h")) {
					printHelp();
					goto end;
				}
			}
			break;

		default:
			inputfile = argv[i];
			break;
		}

		++i;
	}

	if (argc == 1) {
		printHelp();
		goto end;
	}

	if (inputfile.empty()) {
		std::cout << "Error: no input file specified\n";
		ret = -1;
		goto end;
	}

	if (!assemblefile(inputfile, outputfile)) ret = -1;

end:
	if (ret != 0) std::cout << "Build failed\n";
	return ret;
}
#else
bool routeasm(std::string inputfile, std::string filestring, uint8_t*& writeback, int& size) {
	compileLog.clear();
	if (!assemblefile(inputfile, filestring, writeback, size)) {
		compileLog.append("Build Failed");
		return false;
	}
	compileLog.append("Build Succeeded");
	return true;
}
#endif


// Increment pointer to beyond whitespace
inline void ptrws(const char*& strptr) {
	while (*strptr == ' ' || *strptr == '\t') ++strptr;
}

// Find next white space on character pointer
inline void ptrnextws(const char*& strptr) {
	//strptr = strchr(strptr, ' ');
	strptr = strpbrk(strptr, " \t");
}

// Find next character value on pointer
// increments past as many whitespace as required
inline void ptrnextvalue(const char*& strptr) {
	//strptr = strchr(strptr, ' ');
	strptr = strpbrk(strptr, " \t");
	while (*strptr == ' ' || *strptr == '\t') ++strptr;
}

// Increment pointer beyond selected character to remove
inline void ptrrm(const char*& strptr, char rmchar) {
	while (*strptr == rmchar) ++strptr;
}

// find next character value on pointer with selected
// seperating value
// only increments past one of the separating value
inline void ptrnextvalue(const char*& strptr, char seperator) {
	strptr = strchr(strptr, seperator);
	++strptr;
}


void showMessage(std::string filepath, const char* msg, INT_T line = -1) {
#ifndef AUTOPILOT_INTERFACE
	if (line > -1) printf("%s(%d): %s\n", filepath.c_str(), line, msg);
	else printf("%s: %s\n", filepath.c_str(), msg);
#else
	char buffer[256];
	if (line > -1) sprintf(buffer, "%s(%d): %s\n", filepath.c_str(), line, msg);
	else sprintf(buffer, "%s: %s\n", filepath.c_str(), msg);
	compileLog.append(buffer);
#endif
}


void unknown(std::string filepath, int line = -1) {
#ifndef AUTOPILOT_INTERFACE
	if (line > -1) printf("%s(%d): Error: Unknown command\n", filepath.c_str(), line);
	else printf("%s: Error: Unknown command\n", filepath.c_str());
#else
	char buffer[256];
	if (line > -1) sprintf(buffer, "%s(%d): Error: Unknown command\n", filepath.c_str(), line);
	else sprintf(buffer, "%s: Error: Unknown command\n", filepath.c_str());
	compileLog.append(buffer);
#endif
}


// assembled data
std::vector<uint8_t> data;

// array of integer names
std::map<std::string, uint8_t> integers;

// keep track of line number
INT_T linenumber = 1;

bool pushVarData(std::string name, std::string inputfile) {
	auto it = integers.find(name);
	if (it != integers.end()) {
		data.push_back(integers[name]);
		return true;
	}
	else {
		char buffer[64];
		sprintf(buffer, "Error: reference to undefined variable \"%s\"", name.c_str());
		showMessage(inputfile, buffer, linenumber);
		return false;
	}
}


#define radians(x) ((x) * 0.01745329251994329576923690768489)
void gps_cartesian(float latitude, float longitude, float* x, float* y) {
	float multiplier = 111194.9266;

	*x = (latitude - gnss_zerolat) * multiplier;
	*y = (longitude - gnss_zerolong) * multiplier * cos(radians(latitude));
}


#ifndef AUTOPILOT_INTERFACE
bool assemblefile(std::string inputfile, std::string outputfile) {
#else
bool assemblefile(std::string inputfile, std::string filestring, uint8_t * &dataptr, int& datasize) {
#endif
	gnss_zero_defined = false;
	linenumber = 1;
	//std::cout << inputfile << "\n";
	//std::cout << outputfile << "\n";
	data.clear();
	integers.clear();

	namespace fs = std::filesystem;
	fs::path currentpath = fs::current_path();
#ifndef AUTOPILOT_INTERFACE
	std::string inputpath = currentpath.string();
	//#if defined(_WIN32) || defined(_WIN64)
	//	inputpath.append("\\");
	//#else

	inputpath.append("/");
	//#endif
	inputpath.append(inputfile);
#else
	std::string inputpath = inputfile;
#endif
	std::replace(inputpath.begin(), inputpath.end(), '\\', '/');

#ifndef AUTOPILOT_INTERFACE
	std::string outputpath = currentpath.string();
	outputpath.append("/");
	outputpath.append(outputfile);
	std::replace(outputpath.begin(), outputpath.end(), '\\', '/');

	//std::cout << inputpath << "\n";
	//std::cout << outputpath << "\n";

	std::string filestring;
	if (!readFileToString(inputpath, filestring)) {
		std::cout << "Error opening file: " << inputpath << "\n";
		return false;
	}
#endif
	// insert newline at start of file
	filestring.insert(0, "\n");
	// add space at end of string
	filestring.push_back(' ');
	// make string lower case
	transform(filestring.begin(), filestring.end(), filestring.begin(), ::tolower);
	const char* lineptr = filestring.c_str();

	// check if code after "END"
	bool end = false;
	INT_T endLength = 0;

	// loop through lines of file
	while (lineptr = strchr(lineptr, '\n')) {
		++lineptr;
		// remove whitespace at beginning of line
		ptrws(lineptr);
		// check if line is empty and continue if true
		if (*lineptr == '\n') continue;

		// switch first character of line
		switch (*lineptr) {
		case 'a':
			if (strncmp(lineptr, "assign", 6) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(ASSIGN);
				// get value 1
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifiers requred for ASSIGN", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get value 2
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: only one identifier given for ASSIGN", linenumber);
					return false;
				}
				// find size of integer name
				size = strcspn(lineptr, " \n");
				name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else if (strncmp(lineptr, "add_assign", 10) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(ADD_ASSIGN);
				// get value 1
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier requred for ADD_ASSIGN", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get increment value
				ptrnextvalue(lineptr);
				// check value is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: no value given for ADD_ASSIGN", linenumber);
					return false;
				}
				// record value
				int16_t value = atoi(lineptr);
				data.push_back((uint8_t)value);
				data.push_back((uint8_t)(value >> 8));
			}
			else if (strncmp(lineptr, "add", 3) == 0 && (*(lineptr + 3) == '\n' || *(lineptr + 3) == '\0' || *(lineptr + 3) == ' ' || *(lineptr + 3) == ';')) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(ADD);
				// get value 1
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifiers requred for ADD", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get value 2
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: only one identifier given for ADD", linenumber);
					return false;
				}
				// find size of integer name
				size = strcspn(lineptr, " \n");
				name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get value 3
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: only two identifiers given for ADD", linenumber);
					return false;
				}
				// find size of integer name
				size = strcspn(lineptr, " \n");
				name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 'b':
			if (strncmp(lineptr, "break_while", 11) == 0) data.push_back(BREAK_WHILE);
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 'd':
			if (strncmp(lineptr, "decrement", 9) == 0 || strncmp(lineptr, "dec", 3)) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(DECREMENT);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier required for DECREMENT", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				//data.push_back(integers[name]);
				if (!pushVarData(name, inputpath)) return false;
			}
			else if (strncmp(lineptr, "div_assign", 10) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(DIV_ASSIGN);
				// get value 1
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier requred for DIV_ASSIGN", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get increment value
				ptrnextvalue(lineptr);
				// check value is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: no value given for DIV_ASSIGN", linenumber);
					return false;
				}
				// record value
				int16_t value = atoi(lineptr);
				data.push_back((uint8_t)value);
				data.push_back((uint8_t)(value >> 8));
			}
			else if (strncmp(lineptr, "div", 3) == 0 && (*(lineptr + 3) == '\n' || *(lineptr + 3) == '\0' || *(lineptr + 3) == ' ' || *(lineptr + 3) == ';')) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(ADD);
				// get value 1
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifiers requred for DIV", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get value 2
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: only one identifier given for DIV", linenumber);
					return false;
				}
				// find size of integer name
				size = strcspn(lineptr, " \n");
				name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get value 3
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: only two identifiers given for DIV", linenumber);
					return false;
				}
				// find size of integer name
				size = strcspn(lineptr, " \n");
				name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 'e':
			if (strncmp(lineptr, "endwhile", 8) == 0) data.push_back(ENDWHILE);
			else if (strncmp(lineptr, "endif", 5) == 0) data.push_back(ENDIF);
			else if (strncmp(lineptr, "endfor", 6) == 0) data.push_back(ENDFOR);
			else if (strncmp(lineptr, "end", 3) == 0 && (*(lineptr + 3) == '\n' || *(lineptr + 3) == '\0' || *(lineptr + 3) == ' ' || *(lineptr + 3) == ';')) {
				data.push_back(END);
				end = true;
				endLength = data.size();
			}
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 'f':
			if (strncmp(lineptr, "for_var", 7) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(FOR_VAR);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier required for FOR_VAR", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " ");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else if (strncmp(lineptr, "for", 3) == 0 && (*(lineptr + 3) == '\n' || *(lineptr + 3) == '\0' || *(lineptr + 3) == ' ' || *(lineptr + 3) == ';')) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(FOR);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: argument required for FOR", linenumber);
					return false;
				}
				int16_t value = atoi(lineptr);
				data.push_back((uint8_t)value);
				data.push_back((uint8_t)(value >> 8));
			}
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 'i':
			if (strncmp(lineptr, "if_z", 4) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(IF_Z);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier required for IF_Z", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else if (strncmp(lineptr, "if_nz", 5) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(IF_NZ);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier required for IF_NZ", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else if (strncmp(lineptr, "if_pos", 6) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(IF_POS);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier required for IF_POS", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else if (strncmp(lineptr, "if_neg", 6) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(IF_Z);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier required for IF_NEG", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else if (strncmp(lineptr, "integer", 8) == 0 || strncmp(lineptr, "int", 3) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(INTEGER);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier required for INTEGER", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " ");
				std::string intname = std::string(lineptr, size);
				INT_T index = integers.size();
				integers[intname] = index;
				data.push_back((uint8_t)index);

				ptrnextvalue(lineptr);
				// check value is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: no value given for INTEGER", linenumber);
					return false;
				}
				// record value
				int16_t value = atoi(lineptr);
				data.push_back((uint8_t)value);
				data.push_back((uint8_t)(value >> 8));
			}
			else if (strncmp(lineptr, "increment", 9) == 0 || strncmp(lineptr, "inc", 3)) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(INCREMENT);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier required for INCREMENT", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 'l':
			if (strncmp(lineptr, "launch", 6) == 0) data.push_back(LAUNCH);
			else if (strncmp(lineptr, "land", 4) == 0 && (*(lineptr + 4) == '\n' || *(lineptr + 4) == '\0' || *(lineptr + 4) == ' ' || *(lineptr + 4) == ';')) data.push_back(LAND);
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 'm':
			if (strncmp(lineptr, "mul_assign", 10) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(MUL_ASSIGN);
				// get value 1
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier requred for MUL_ASSIGN", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get increment value
				ptrnextvalue(lineptr);
				// check value is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: no value given for MUL_ASSIGN", linenumber);
					return false;
				}
				// record value
				int16_t value = atoi(lineptr);
				data.push_back((uint8_t)value);
				data.push_back((uint8_t)(value >> 8));
			}
			else if (strncmp(lineptr, "mul", 3) == 0 && (*(lineptr + 3) == '\n' || *(lineptr + 3) == '\0' || *(lineptr + 3) == ' ' || *(lineptr + 3) == ';')) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(MUL);
				// get value 1
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifiers requred for MUL", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get value 2
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: only one identifier given for MUL", linenumber);
					return false;
				}
				// find size of integer name
				size = strcspn(lineptr, " \n");
				name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get value 3
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: only two identifiers given for MUL", linenumber);
					return false;
				}
				// find size of integer name
				size = strcspn(lineptr, " \n");
				name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 'p':
			if (strncmp(lineptr, "point_lla", 9) == 0) {
				//if (!gnss_zero_defined) {
				//	showMessage(inputpath, "Error: home position not defined, use .home_ll to define", linenumber);
				//	return false;
				//}
				//// record line end
				//const char* lineend = strchr(lineptr, '\n');
				//// create type converter
				//Float_Converter converter;
				//// allocate memory
				//data.reserve(13);
				//data.push_back(POINT);
				//// go to first value
				//ptrnextvalue(lineptr);
				//if (lineptr >= lineend) {
				//	showMessage(inputpath, "Error: no arguments given for POINT_LLA, 3 required", linenumber);
				//	return false;
				//}
				//float latitude = atof(lineptr);
				//// next value
				//ptrnextvalue(lineptr);
				//if (lineptr >= lineend) {
				//	showMessage(inputpath, "Error: one argument given for POINT_LLA, 3 required", linenumber);
				//	return false;
				//}
				//float longitude = atof(lineptr);
				//// next value
				//ptrnextvalue(lineptr);
				//if (lineptr >= lineend) {
				//	showMessage(inputpath, "Error: two arguments given for POINT_LLA, 3 required", linenumber);
				//	return false;
				//}
				//float altitude = atof(lineptr);

				//float x, y;
				//gps_cartesian(latitude, longitude, &x, &y);

				//float coords[3] = { x, y, -altitude };
				//for (int i = 0; i < 3; ++i) {
				//	converter.value = coords[i];
				//	for (int j = 0; j < 4; ++j) {
				//		data.push_back(converter.reg[j]);
				//	}
				//}

				// store end of line
				const char* lineend = strchr(lineptr, '\n');
				// create type converter
				Float_Converter converter;
				// allocate memory
				data.reserve(13);
				data.push_back(POINT_LLA);
				// go to next value
				for (INT_T i = 0; i < 3; ++i) {
					ptrnextvalue(lineptr);
					converter.value = atof(lineptr);
					for (INT_T j = 0; j < 4; ++j) {
						data.push_back(converter.reg[j]);
					}
				}

				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: invalid arguments to mnemonic POINT_LLA", linenumber);
					return false;
				}
			}
			else if (strncmp(lineptr, "point", 5) == 0 && (*(lineptr + 5) == '\n' || *(lineptr + 5) == '\0' || *(lineptr + 5) == ' ' || *(lineptr + 5) == ';')) {
				// store end of line
				const char* lineend = strchr(lineptr, '\n');
				// create type converter
				Float_Converter converter;
				// allocate memory
				data.reserve(13);
				data.push_back(POINT);
				// go to next value
				for (INT_T i = 0; i < 3; ++i) {
					ptrnextvalue(lineptr);
					converter.value = atof(lineptr);
					for (INT_T j = 0; j < 4; ++j) {
						data.push_back(converter.reg[j]);
					}
				}

				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: invalid arguments to mnemonic POINT", linenumber);
					return false;
				}
			}
			else if (strncmp(lineptr, "print", 5) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(PRINT);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier required for PRINT", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 'r':
			if (strncmp(lineptr, "rtl", 3) == 0 && (*(lineptr + 3) == '\n' || *(lineptr + 3) == '\0' || *(lineptr + 3) == ' ' || *(lineptr + 3) == ';')) data.push_back(RTL);
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 's':
			if (strncmp(lineptr, "sub_assign", 10) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(SUB_ASSIGN);
				// get value 1
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier requred for SUB_ASSIGN", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get increment value
				ptrnextvalue(lineptr);
				// check value is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: no value given for SUB_ASSIGN", linenumber);
					return false;
				}
				// record value
				int16_t value = atoi(lineptr);
				data.push_back((uint8_t)value);
				data.push_back((uint8_t)(value >> 8));
			}
			else if (strncmp(lineptr, "sub", 3) == 0 && (*(lineptr + 3) == '\n' || *(lineptr + 3) == '\0' || *(lineptr + 3) == ' ' || *(lineptr + 3) == ';')) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(SUB);
				// get value 1
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifiers requred for SUB", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get value 2
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: only one identifier given for SUB", linenumber);
					return false;
				}
				// find size of integer name
				size = strcspn(lineptr, " \n");
				name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;

				// get value 3
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: only two identifiers given for SUB", linenumber);
					return false;
				}
				// find size of integer name
				size = strcspn(lineptr, " \n");
				name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

		case 'w':
			if (strncmp(lineptr, "while_var", 9) == 0) {
				// record line end
				const char* lineend = strchr(lineptr, '\n');
				data.push_back(WHILE_VAR);
				ptrnextvalue(lineptr);
				// check name is given
				if (lineptr >= lineend) {
					showMessage(inputpath, "Error: identifier required for WHILE_VAR", linenumber);
					return false;
				}
				// find size of integer name
				INT_T size = strcspn(lineptr, " \n");
				std::string name = std::string(lineptr, size);
				if (!pushVarData(name, inputpath)) return false;
			}
			else if (strncmp(lineptr, "while", 5) == 0 && (*(lineptr + 5) == '\n' || *(lineptr + 5) == '\0' || *(lineptr + 5) == ' ' || *(lineptr + 5) == ';')) data.push_back(WHILE);
			else {
				unknown(inputpath, linenumber);
				return false;
			}
			break;

			//case '.':
			//	if (strncmp(lineptr, ".home_ll", 7) == 0) {
			//		// record line end
			//		const char* lineend = strchr(lineptr, '\n');
			//		ptrnextvalue(lineptr);
			//		// check first value is given
			//		if (lineptr >= lineend) {
			//			showMessage(inputpath, "Error: no arguments given for \".home_ll\", two required", linenumber);
			//			return false;
			//		}
			//		gnss_zerolat = atof(lineptr);
			//		ptrnextvalue(lineptr);
			//		// check second value is given
			//		if (lineptr >= lineend) {
			//			showMessage(inputpath, "Error: one argument given for \".home_ll\", two required", linenumber);
			//			return false;
			//		}
			//		gnss_zerolong = atof(lineptr);
			//		gnss_zero_defined = true;
			//	}
			//	else {
			//		unknown(inputpath, linenumber);
			//		return false;
			//	}
			//	break;

		case ';':
		case '\n':
		case '\0':
			++lineptr;
			break;

		default:
			unknown(inputpath, linenumber);
			return false;
			break;
		}

		++linenumber;
	}

	// check end directive included in program
	if (!end) {
		showMessage(inputpath, "Error: no \"END\" mnemonic found");
		return false;
	}

	if (endLength < data.size()) {
		showMessage(inputpath, "Warning: unreachable code after \"END\" mnemonic");
	}

	//for (auto it = data.begin(); it != data.end(); ++it) {
	//	printf("%02X\n", *it);
	//}

#ifndef AUTOPILOT_INTERFACE
	uint8_t* dataptr = (uint8_t*)malloc(data.size());
#else
	dataptr = (uint8_t*)realloc(dataptr, data.size());
	datasize = data.size();
#endif
	std::copy(data.begin(), data.end(), dataptr);
#ifndef AUTOPILOT_INTERFACE
	writeDataToFile(outputpath, dataptr, data.size());
	free(dataptr);
#endif

	return true;
}


#ifndef AUTOPILOT_INTERFACE
void printHelp() {
#if defined(_WIN32) || defined(_WIN64)
	std::cout << "Usage: routeasm.exe [-o outfile] filename\n\n";
#else
	std::cout << "Usage: routeasm [-o outfile] filename\n\n";
#endif

	std::cout << "-o outfile   define output file path\n";
	std::cout << "-h (--help)  display this help screen\n";
}
#endif


#ifdef AUTOPILOT_INTERFACE
void routeasm_get_log(std::string & routeLog) {
	routeLog.assign(compileLog);
}
#endif