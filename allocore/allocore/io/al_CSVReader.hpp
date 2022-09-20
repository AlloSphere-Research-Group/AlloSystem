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
#include <cstring> // memcpy

namespace al{

/// @addtogroup allocore
/// @{

/**
 * @brief The CSVReader class reads simple CSV files
 *
 * CSV is a delimited data format that has fields/columns separated by the comma
 * character and records/rows terminated by newlines. "CSV" formats vary greatly
 * in choice of separator character. In particular, in locales where the comma
 * is used as a decimal separator, semicolon, TAB, or other characters are used 
 * instead. [Reference: https://en.wikipedia.org/wiki/Comma-separated_values].
 *
 * To use, first create a CSVReader object and call addType() to add the type of
 * a column. Then call readFile().
 *
 * Once the file is in memory it can be read as columns of floats using
 * getColumn(). Or the whole CSV data can be copied to memory by defining
 * a struct that will hold the values from each row from the csv file and 
 * calling copyToStruct() to create a vector with the data from the CSV file.
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

	CSVReader(){}

	~CSVReader();

	/**
	 * @brief readFile reads the CSV file into internal memory
	 * @param fileName the csv file name
	 * \returns whether the file was successfully read.
	 *
	 * If the column data types have not been defined before calling this
	 * function using addType, then the reader will do its best job to derive
	 * the types.
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
	std::vector<DataStruct> copyToStruct() const {
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
	 *
	 * Values will be implicitly casted from their stored DataType to T.
	 */
	template <class T>
	std::vector<T> getColumn(int index) const;

	std::vector<double> getColumn(int index) const {
		return getColumn<double>(index);
	}


	/// Get column names
	const std::vector<std::string>& columnNames() const { return mColumnNames; }

private:

	size_t columnByteOffset(int col) const;
	size_t calculateRowLength() const;
	size_t typeSize(CSVReader::DataType type) const;

	const size_t maxStringSize = 32;

	std::vector<std::string> mColumnNames;
	std::vector<DataType> mDataTypes;
	std::vector<char *> mData;
	char mDelim = ',';
};

/// @} // end allocore group

template <class T>
std::vector<T> CSVReader::getColumn(int index) const {
	std::vector<T> out;
	auto offset = columnByteOffset(index);
	auto type = mDataTypes[index];
	for (auto row: mData) {
		switch(type){
		case REAL:
			out.push_back(*(const double *)(row + offset));
			break;
		case INTEGER:
			out.push_back(*(const int32_t *)(row + offset));
			break;
		case BOOLEAN:
			out.push_back(*(const bool *)(row + offset));
			break;
		default:;
		}
	}
	return out;
}

template <>
inline std::vector<std::string> CSVReader::getColumn<std::string>(int index) const {
	std::vector<std::string> out;
	auto offset = columnByteOffset(index);
	auto type = mDataTypes[index];
	for (auto row: mData) {
		switch(type){
		case STRING:
			out.emplace_back((const char *)(row + offset));
			break;
		default:;
		}
	}
	return out;
}

} //al::

#endif // include guard
