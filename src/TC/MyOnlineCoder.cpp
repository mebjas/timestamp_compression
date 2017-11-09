#include "stdafx.h"
#include "MyOnlineCoder.h"
#include "stdafx.h"
#include <fstream>
#include <string>
#include <iostream>
#include <math.h>
#include <iomanip>
using namespace std;

#pragma region PRIVATE METHODS
/// Method to encode first timestamp to 8Bytes
/// TODO: can be stored in 7Bytes as well; minor optimization
/// @param:uint64_t number - number with microseconds accuracy, no decimal
/// @param:ofp - output file pointer
/// @returns: void
void MyOnlineCoder::encodeFirstString(
	uint64_t number,
	std::ofstream *ofp)
{
	this->lastNumber = number;
	uint8_t bytes[8] = { 0 };
	for (int8_t i = 7; i >= 0; i--) {
		bytes[i] = number;
		number >>= 8;
	}
	ofp->write((char *)bytes, 8);

	/// Set this number as last number
	++ this->count;
}
#pragma endregion

#pragma region PUBLIC METHODS
/// Method to get next timestamp from file
/// @param:ifp - input file pointer
/// @returns: string format of timestamp from @param:ifp
std::string MyOnlineCoder::getNextLine(std::ifstream *ifp) {
	if (!*ifp) return "";

	std::string response;
	std::getline(*ifp, response);
	return response;
}

/// Method to code next timestamp
/// @param:timestamp - string format of timestamp
/// @param:ofp - output file pointer
/// @returns: void
void MyOnlineCoder::codeNextTimestamp(std::string timestamp, std::ofstream *ofp) {
	uint64_t number = std::stod(timestamp) * 1000000;
	int8_t i,					// for iterations
		prefix = 0,				// to store information on no of bytes needed for encoding this delta
		secondPrefix;			// indiacates state of buffering

#ifdef _DEBUG2
	std::cout << fixed << setprecision(8) << number << " -> " << intPart << ", " << decimalPart << endl;
#endif

	//// NOTE: this code is for litte endian systems like WINDOWS
	//// TODO: how much would it differ otherwise?
	if (this->count == 0) {
		return this->encodeFirstString(number, ofp);
	}

	this->delta = number - this->lastNumber;
	this->lastNumber = number;

	if (this->delta <= 1) {
#pragma region CASE WHEN DATA IS BUFFERED
		if (this->bufferPrefix != -1) {
			if (this->bufferPrefix == 0) {
#pragma region BUFFERING IN SHORTER BITS
				//// TODO: line of codes can be shortened here;
				//// Case when a 0 or 1 further encodes other 0 or 1
				//// currently as 0 or 1 looks like 00X00000, so we will
				//// use last 4 bits to store 0 or 1 as 10 or 01 respectively
				secondPrefix = this->bytes[0] & 15;
				if (secondPrefix == 0) {
					//// means data will be fed to xx xx YY xx position
					if (this->delta == 0) this->bytes[0] |= 8;
					else this->bytes[0] |= 4;
					this->count++;
					return;
				}
				else {
					//// means data will be fed to xx xx YY xx position
					if (this->delta == 0) this->bytes[0] |= 2;
					else this->bytes[0] |= 1;
					this->count++;
					return this->flush(ofp);
				}
#pragma endregion
			}
			else {
#pragma region BUFFERING IN LONGER BYTE ENCODING
				//// This indicates some data is buffered; For delta = 0 or 1
				//// some data can be encoded in the flushed buffer;

				//// The information shall be added in these bits of last byte
				//// YY xx xx xx; Get value of YY in next step
				secondPrefix = (this->bytes[this->bufferPrefix] & 192);

				if (this->delta == 0) {
					if (secondPrefix == 0) {
						//// No data already encoded;
						//// Will do first level of encoding by adding
						//// 01xxxxxx in the last byte 
						this->bytes[this->bufferPrefix] |= 64;
						this->count++;

						/// Now leave it as such, shall be flushed in next step
						return;
					}
					else if (secondPrefix == 64) {
						//// One level encoding already done;
						//// Now we will encode 11xxxxxx in last byte
						this->bytes[this->bufferPrefix] |= 192;

						//// Now increase the count by two and flush
						this->count++;
						return this->flush(ofp);
					}
					else {
						//// TODO: debug & check if this occur, shouldn't ideally;
						throw new exception("Unhandled case: This shouldn't occur");
					}
				}
				else {
					//// case when delta = 1
					if (secondPrefix == 0) {
						//// Encode 10xxxxxx to the buffered data and flush
						this->bytes[this->bufferPrefix] |= 128;
						this->count++;
						return this->flush(ofp);
					}
					else if (secondPrefix == 64) {
						//// Flush currently buffered data
						this->flush(ofp);

						//// this 1 value will be encoded in a 1 byte model
						this->bytes[0] = 32;		/// 00100000
						this->bufferPrefix = 0;
						return this->flush(ofp);
					}
					else {
						//// TODO: debug & check if this occur, shouldn't ideally;
						throw new exception("Unhandled case: This shouldn't occur");
					}
				}
#pragma endregion
			}
		}
#pragma endregion

		//// Case when delta = [0, 1] - one byte model
		//// 0 will be stored as 00 00 XX XX & 1 as 00 10 XX XX
		//// XX XX will be used to buffer more data;
		this->bytes[0] = (this->delta == 1) ? 32 : 0;

		this->bufferPrefix = 0;
		return;
	}
	else if (this->delta <= 4095) {
		//// BINARY(4095) = 0000111111111111, largest value that can be stored in 12 bits
		//// this will be our 2byte model; format: 01xxxxxx YYxxxxxx
		//// (YY is information about buffering), xx.. represents the delta value
		prefix = 1;
	}
	else if (this->delta <= 1048575) {
		//// 1048575 is the largest value that can be stored in 20bits
		//// this is our 3 byte model, format: 10xxxxxx xxxxxxxx YYxxxxxx
		//// (YY is information about buffering), xx.. represents the delta value
		prefix = 2;
	}
	else if (this->delta <= 268435455) {
		//// 268435455 is the largest value that can be stored in 28 bits
		//// 4 byte model
		//// format: 11xxxxxx xxxxxxxx xxxxxxxx YYxxxxxx
		//// (YY is information about buffering), xx.. represents the delta value
		prefix = 3;
	}
	else {
		//// TODO: Case not supported as of now 
		throw new exception ("Unexpected case; Not supported; HALT");
	}

	//// flush previous buffer if exists
	this->flush(ofp);

	this->bytes[0] = prefix;
	this->bytes[0] <<= 6;
	this->bytes[0] |= (this->delta & 63);
	this->delta >>= 6;
	for (i = 1; i <= prefix; i++, this->delta >>= 8) {
		this->bytes[i] = this->delta;
	}

	/// this data will be flushed in next iteration
	this->bufferPrefix = prefix;
}

/// Method to safely flush the buffered data
/// @param:ofp - output file pointer
/// @returns: void
void MyOnlineCoder::flush(std::ofstream *ofp) {
	if (this->bufferPrefix == -1) return;

	/// Flush the data in buffer to IO
	ofp->write((char *) this->bytes, this->bufferPrefix + 1);
	this->count++;
	this->bufferPrefix = -1;
}

/// Method to get count of items processed
int MyOnlineCoder::getCount() {
	return this->count;
}
#pragma endregion
