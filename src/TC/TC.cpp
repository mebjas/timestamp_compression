#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <Windows.h>
#include <math.h>
#include <iomanip>
#include "MyOnlineCoder.h"
#include "MyOnlineDecoder.h"
#include <time.h>
using namespace std;

#ifdef _DEBUG
LARGE_INTEGER frequency;        // ticks per second
LARGE_INTEGER t1, t2;           // ticks
double elapsedTime;
#endif

////// Set of constants
////// TODO: get these as command line inputs
//const char *inputFile = ".\\timestamps.txt",
//	*encodedOutputFile = ".\\timestamps_encoded.txt",
//	*decodedOutputFile =  ".\\timestamps_decoded.txt";

/// Method to print command line usage format
/// @returns: int = -1
int printInputFormat() {
	cout << "Invalid syntax" << endl;
	cout << "encoding example: TC.exe -e .\\timestamps.txt .\\timestamps_encoded.txt" << endl;
	cout << "decoding example: TC.exe -d .\\timestamps.txt .\\timestamps_encoded.txt" << endl;
	return -1;
}

/// Main function with command line input
int main(int argc,      // Number of strings in array argv
	char *argv[],   // Array of command-line argument strings
	char *envp[])
{
	/// Validate command line args
	if (argc != 4) return printInputFormat();
	else if (std::strlen(argv[1]) != 2) return printInputFormat();

#ifdef _DEBUG
	/// get ticks per second - to profile encoding and decoding method
	QueryPerformanceFrequency(&frequency);
#endif

	/// define variables
	std::ifstream ifp;
	std::ofstream ofp;
	MyOnlineCoder coder;
	MyOnlineDecoder decoder;
	string timestamp;

	if (argv[1][0] == '-' && argv[1][1] == 'e') {
		//// ----- ENCODING -----------------------------------------
		ifp.open(argv[2], ios::in);
		if (!ifp) {
			printf("Input file not found; HALT;");
			return -1;
		}

		ifp.seekg(ios::_Seekbeg);
		ofp.open(argv[3], ios::out | ios_base::binary);
		if (!ofp) {
			printf("Unable to write to encoded output file; HALT;");
			return -1;
		}

		//// Encoding process
		cout << "Begin Encoding" << endl;

#ifdef _DEBUG
		QueryPerformanceCounter(&t1);
#endif

		while ((timestamp = coder.getNextLine(&ifp)) != "")
		{
			coder.codeNextTimestamp(timestamp, &ofp);
		}

		/// Note: this final flush is important
		coder.flush(&ofp);

		cout << "Encoding done; Processed: " << coder.getCount() << " rows" << endl;

#ifdef _DEBUG
		QueryPerformanceCounter(&t2);
		// compute and print the elapsed time in millisec
		elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
		cout << "Time taken (in micro seconds) : " << elapsedTime << endl;
		cout << "Avg Time taken (in micro seconds) : " << elapsedTime / coder.getCount() << endl;
#endif

		ifp.close();
		ofp.close();
	}
	else if (argv[1][0] == '-' && argv[1][1] == 'd') {
		//// ----- DECODING -----------------------------------------
		ifp.open(argv[2], ios::in | ios_base::binary);
		if (!ifp) {
			printf("Unable to read from encoded input file; HALT;");
			std::getchar();
			return -1;
		}

		ifp.seekg(ios::_Seekbeg);
		ofp.open(argv[3], ios::out);
		if (!ofp) {
			printf("Unable to write to decoded output file; HALT;");
			std::getchar();
			return -1;
		}

		cout << "Begin Decoding" << endl;
#ifdef _DEBUG
		QueryPerformanceCounter(&t1);
#endif

		decoder.parseAndDecode(&ifp, &ofp);
		cout << "Decoding done; Processed: " << decoder.getCount() << " rows" << endl;

#ifdef _DEBUG
		QueryPerformanceCounter(&t2);
		// compute and print the elapsed time in millisec
		elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
		cout << "Time taken (in micro seconds) : " << elapsedTime << endl;
		cout << "Avg Time taken (in micro seconds) : " << elapsedTime / decoder.getCount() << endl;
#endif

		ifp.close();
		ofp.close();
	}
	else {
		return printInputFormat();
	}
	return 0;
}