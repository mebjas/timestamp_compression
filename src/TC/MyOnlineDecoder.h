#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <math.h>
#include <iomanip>

#pragma once
class MyOnlineDecoder
{
private:
	/// Number that was last decoded
	uint64_t lastNumber = 0;

	/// current delta value based on data read;
	uint64_t delta = 0;
	
	/// Count of items processed
	int count = 0;

	/// Method to get first number
	/// @param:ifp - input file pointer
	/// @returns: first number from the encoded file stream
	double getFirstNumber(std::ifstream *ifp);

	/// Method to write a row to decoded file
	/// @param:number - number to write
	/// @param:ofp - output file pointer
	/// @returns: void
	void writeRow(double number, std::ofstream *ofp);

public:
	/// Method to decode the input file to set of timestamps
	/// @param:ifp - input file pointer
	/// @param:ofp - output file pointer
	/// @returns: void
	void parseAndDecode(std::ifstream *ifp, std::ofstream *ofp);

	/// Method to get count of items processed
	/// @returns: integer count of items processed
	int getCount();
};

