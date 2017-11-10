#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <math.h>
#include <iomanip>
#include "MyOnlineDecoder.h"
using namespace std;

#pragma region PRIVATE METHODS
/// Method to get first number
/// @param:ifp - input file pointer
/// @returns: first number from the encoded file stream
double MyOnlineDecoder::getFirstNumber(std::ifstream *ifp) {
	uint64_t fullNumber = 0;
	uint8_t bytes[8] = { 0 };

	ifp->read((char *)bytes, 8);
	for (int8_t i = 0; i < 8; i++) {
		fullNumber <<= 8;
		fullNumber |= bytes[i];
	}

	this->lastNumber = fullNumber;
	return ((double)fullNumber / 1000000);
}

/// Method to write a row to decoded file
/// @param:number - number to write
/// @param:ofp - output file pointer
/// @returns: void
void MyOnlineDecoder::writeRow(double number, std::ofstream *ofp) {
#ifdef  _DEBUG2
	cout << fixed << setprecision(6) << number << endl;
#endif

	(*ofp) << std::fixed << std::setprecision(6) << number << endl;
	++this->count;
}
#pragma endregion

#pragma region PUBLIC METHODS
/// Method to decode the input file to set of timestamps
/// @param:ifp - input file pointer
/// @param:ofp - output file pointer
/// @returns: void
void MyOnlineDecoder::parseAndDecode(std::ifstream *ifp, std::ofstream *ofp) {
	ifp->seekg(ifstream::_Seekbeg);
	int8_t i;						// to be used for iterations
	uint8_t bytes[8];				// to store data loaded from file
									// TODO: only 4 bytes are needed;
	uint8_t prefix,					// Indicates the prefix encoded in beginning of
									// first byte of every encoded delta;
		bufferedMarker;				// Indicates if some data was appended to long byte
									// if > 0, 00, 01 & 11 indicates different type of buffering
	uint64_t number;				// to store current number

	/// Get first row and write to file;
	this->writeRow(this->getFirstNumber(ifp), ofp);

	while (!ifp->eof()) {
		if (ifp->fail()) {
			cout << "IFP failed halt" << endl;
			return;
		}
		bufferedMarker = 0;
		this->delta = 0;
		number = this->lastNumber;

		/// Get first byte; the prefix information is encoded in the first byte
		/// as YY xx xx xx; the YY value indicates the prefix information;
		bytes[0] = 0;
		ifp->read((char *)bytes, 1);
		prefix = bytes[0] >> 6;

		if (prefix == 0) {
			//// So that eof is not mistaken as 0 value
			if (bytes[0] == 0 && ifp->eof()) {
				return;
			}

			//// indicates 1byte model to represent value 1
			if ((bytes[0] & 32) == 32) {
				number += 1;
			}
		}
		else {
			for (i = 1; i <= prefix; i++) bytes[i] = 0;

			ifp->read((char *)bytes + 1, prefix);
			for (i = prefix; i >= 1; i--) {
				if (i == prefix) {
					bufferedMarker = bytes[i] & 192;
					bytes[i] = bytes[i] & 63;
				}

				this->delta |= bytes[i];
				if (i != 1) this->delta <<= 8;
				else this->delta <<= 6;
			}

			this->delta |= (bytes[0] & 63);
			number += this->delta;
		}

		this->writeRow(((double)number / 1000000), ofp);
#pragma region HANDLE BUFFERED DATA
		if (prefix == 0) {
			//// check for bufferred data, looking at xx xx YY YY area
			//// in 1Byte model data
			for (i = 1; i >= 0; i--) {
				bufferedMarker = bytes[0] & (3 << (i * 2));
				if (bufferedMarker == (2 << (i * 2))) {
					//// bufferred 0
					this->writeRow(((double)number / 1000000), ofp);
				}
				else if (bufferedMarker == (1 << (i * 2))) {
					//// bufferred 1
					this->writeRow(((double)(number + 1) / 1000000), ofp);
					number += 1;
				}
			}
		}
		else {
			//// Check for buffered data in last index of >1Byte model
			//// Handle buffered data if there is
			switch (bufferedMarker)
			{
			case 64:
				// add a same number
				this->writeRow(((double)number / 1000000), ofp);
				break;
			case 192:
				// write the same number twice
				this->writeRow(((double)number / 1000000), ofp);
				this->writeRow(((double)number / 1000000), ofp);
				break;
			case 128:
				/// add number + 1
				this->writeRow(((double)(number + 1) / 1000000), ofp);
				number += 1;
				break;
			default:
				/// when no data is buffered;
				break;
			}
		}
#pragma endregion

		this->lastNumber = number;
	}
}

/// Method to get count of items processed
/// @returns: integer count of items processed
int MyOnlineDecoder::getCount()
{
	return this->count;
}
#pragma endregion