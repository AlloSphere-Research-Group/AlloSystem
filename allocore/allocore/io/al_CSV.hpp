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
	CSV file reader and writer

	File author(s):
	Andres Cabrera mantaraya36@gmail.com 2017
	Lance Putnam 2022
*/

#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace al{

/// @addtogroup allocore
/// @{

/// CSV (comma-separated value) reader

/// This implementation uses "lazy" evaluation to obtain numerical values.
/// All fields are stored internally as strings and converted to numerical 
/// values when required. Double-quoted fields (containing zero or more 
/// delimiters) and a custom delimiter are supported.
class CSVReader {
public:

	/// A field (value) as a raw character string
	struct Field{
		const char * ptr = nullptr;
		unsigned size = 0;

		/// Get C-string (null-terminated character array)
		const char * c_str() const { return ptr; }

		/// Convert field to specified type
		template <class T> T to() const;

		float toFloat() const;
		double toDouble() const;
		int toInt() const;
	};

	typedef std::vector<Field> Fields;


	/// Read in raw CSV data
	bool read(std::istream& is);

	/// Read in raw CSV data from memory
	bool read(const void * src, unsigned len);

	/// Read data from file
	bool readFile(const std::string& path);


	/// Set delimiter
	CSVReader& delim(char v){ mDelim=v; return *this; }

	/// Set whether the first row should be treated as column headings
	CSVReader& hasHeader(bool v){ mHasHeader=v; return *this; }

	/// Get number of columns
	unsigned numCols() const { return mNumCols; }
	unsigned width() const { return numCols(); }

	/// Get number of rows (excluding header)
	unsigned numRows() const { return mData.size()/numCols(); }
	unsigned height() const { return numRows(); }

	const Fields& data() const { return mData; }
	const Fields& header() const { return mHeader; }

	/// Iterate over all data fields

	/// Use this to extract numerical values from the field strings.
	/// onField arguments are the current field, its column and its row.
	CSVReader& iterate(const std::function<void(Field, int, int)>& onField);

private:
	std::vector<char> mRawData;
	Fields mData;
	Fields mHeader;
	unsigned mNumCols = 0;
	char mDelim = ',';
	bool mHasHeader = true;
	
	bool parseFields(); // parse raw data into fields
};



/// Simple CSV writer
class CSVWriter {
public:

	/// Set column delimiter
	CSVWriter& delim(char v){ mDelim=v; return *this; }
	char delim() const { return mDelim; }

	/// Start new row
	CSVWriter& beginRow();

	/// End current row
	CSVWriter& endRow();

	/// Start row, call function (that adds columns) and end row
	template <class Func>
	CSVWriter& addRow(const Func& f){
		beginRow(); f(); endRow(); return *this;
	}

	/// Add column from value
	template <class T>
	CSVWriter& operator << (const T& v){
		if(mWriteDelim) mSS << mDelim;
		mWriteDelim = true;
		mSS << v;
		return *this;
	}

	/// Clear current contents
	CSVWriter& clear();

	/// Write current contents to file
	bool writeFile(const std::string& path, bool append=false);

	/// Append current contents to end of file
	bool appendToFile(const std::string& path);

private:
	std::ostringstream mSS;
	char mDelim = ',';
	bool mWriteDelim = false;
};

/// @} // end allocore group

} //al::

#endif // include guard
