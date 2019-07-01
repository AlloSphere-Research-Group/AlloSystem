#ifndef INCLUDE_AL_CSVREADER_HPP
#define INCLUDE_AL_CSVREADER_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	CSV File reader

	File author(s):
	Andres Cabrera mantaraya36@gmail.com 2017
*/

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

/**
 * @brief The CSVReader class reads simple CSV files
 *
 * To use, first create a CSVReader object and call addType() to add the type of a
 * column. Then call readFile().
 *
 * Once the file is in memory it can be read as columns of floats using
 * getColumn(). Or the whole CSV data can be copied to memory by defining
 * a struct that will hold the values from each row from the csv file and calling
 * copyToStruct() to create a vector with the data from the CSV file.
 *
 * This reader is currently very naive (but efficient) and might choke with
 * complex or malformed CSV files.
 *
 * \code
typedef struct {
	char s[32];
	double val1, val2, val3;
	bool b;
} RowTypes;

int main(int argc, char *argv[]) {

	CSVReader reader;
	reader.addType(CSVReader::STRING);
	reader.addType(CSVReader::REAL);
	reader.addType(CSVReader::REAL);
	reader.addType(CSVReader::REAL);
	reader.addType(CSVReader::BOOLEAN);
	reader.readFile("examples/csv/I13893-gm-21DIV-0001.csv");

	std::vector<RowTypes> rows = reader.copyToStruct<RowTypes>();
	for(auto row: rows) {
		std::cout << std::string(row.s) << " : "
		          << row.val1 << "   "
		          << row.val2 << "   "
		          << row.val3 << "   "
		          << (row.b ? "+" : "-")
		          << std::endl;
	}

	std::vector<double> column1 = reader.getColumn(1);
	for(auto value: column1) {
		std::cout << value << std::endl;
	}
	std::cout << " Num rows:" << rows.size() << std::endl;
	return 0;
	\endcode
 */
class CSVReader {
public:
	typedef enum {
		STRING,
		REAL,
		INTEGER,
		BOOLEAN,
		NONE
	} DataType;

	CSVReader() {
		// TODO We could automatically add types by trying to parse the file
	}

	~CSVReader();

	/**
	 * @brief readFile reads the CSV file into internal memory
	 * @param fileName the csv file name
	 * \returns whether the file was successfully read.
	 */
	bool readFile(std::string fileName);

	/**
	 * @brief Set the delimiter used by readFile for parsing values
	 */
	CSVReader& delim(char v){ mDelim=v; return *this; }

	/**
	 * @brief addType
	 * @param type
	 */
	void addType(DataType type) {
		mDataTypes.push_back(type);
	}

	/**
	 * @brief Returns all columns from the csv file
	 * @param index column index
	 * @return vector with the data
	 */
	template<class DataStruct>
	std::vector<DataStruct> copyToStruct() {
		std::vector<DataStruct> output;
		if (sizeof(DataStruct) < calculateRowLength()) {
			return output;
		}
		for (auto row: mData) {
			DataStruct newValues;
			memcpy(&newValues, row, sizeof(newValues));
			output.push_back(newValues);
		}

		return output;
	}

	/**
	 * @brief Returns a column from the csv file
	 * @param index column index
	 * @return vector with the data
	 */
	std::vector<double> getColumn(int index);


private:

	size_t calculateRowLength();

	const size_t maxStringSize = 32;

	std::vector<std::string> mColumnNames;
	std::vector<DataType> mDataTypes;
	std::vector<char *> mData;
	char mDelim = ',';
};

#endif // INCLUDE_AL_CSVREADER_HPP
