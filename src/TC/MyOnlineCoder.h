#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <math.h>
#include <iomanip>

#pragma once
class MyOnlineCoder
{
private:
	/// Indicates last number that was processed
	uint64_t lastNumber = 0;

	/// Indicates the delta of current number and last number
	uint64_t delta = 0;

	/// buffer prefix: indicates if some data is bufferred;
	/// value = -1, indicates no buffer
	/// value = {1, 2, 3} indicates active buffer
	int8_t bufferPrefix = -1;

	/// storage for writing data to file
	uint8_t bytes[8];

	/// No of items that have been processed
	int count = 0;

	/// Method to encode first timestamp
	/// @param:uint64_t number - number with microseconds accuracy, no decimal
	/// @param:ofp - output file pointer
	/// @returns: void
	void MyOnlineCoder::encodeFirstString(
		uint64_t number,
		std::ofstream *ofp);

public:

	/// Method to get next timestamp from file
	/// @param:ifp - input file pointer
	/// @returns: string format of timestamp from @param:ifp
	std::string getNextLine(std::ifstream *ifp);

	/// Method to code next timestamp
	/// @param:timestamp - string format of timestamp
	/// @param:ofp - output file pointer
	/// @returns: void
	void codeNextTimestamp(std::string timestamp, std::ofstream *ofp);

	/// Method to safely flush the buffered data
	/// @param:ofp - output file pointer
	/// @returns: void
	void flush(std::ofstream *ofp);

	/// Method to get count of items processed
	/// @returns: integer count of items processed
	int getCount();
};

