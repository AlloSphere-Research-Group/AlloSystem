#include <cstdlib> // atoi, atof
#include <cstring> // memcpy
#include <fstream>
#include "allocore/io/al_CSV.hpp"

using namespace al;

template<> double CSVReader::Field::to<double>() const { return std::atof(c_str()); }
template<> float CSVReader::Field::to<float>() const { return to<double>(); }
template<> int CSVReader::Field::to<int>() const { return std::atoi(c_str()); }
template<> std::string CSVReader::Field::to<std::string>() const { return c_str(); }
float CSVReader::Field::toFloat() const { return to<float>(); }
double CSVReader::Field::toDouble() const { return to<double>(); }
int CSVReader::Field::toInt() const { return to<int>(); }

bool CSVReader::read(std::istream& is){
	is.seekg(0, is.end);
	unsigned numBytes = is.tellg();
	is.seekg(0, is.beg);
	mRawData.resize(numBytes + 1);
	mRawData.back() = '\0';
	is.read(mRawData.data(), numBytes);
	return parseFields();
}

bool CSVReader::read(const void * src, unsigned len){
	mRawData.resize(len + 1);
	mRawData.back() = '\0';
	std::memcpy(&mRawData[0], src, len);
	return parseFields();
}

bool CSVReader::readFile(const std::string& path){
	std::ifstream f(path);
	if(f.is_open()){
		return read(f); // read using istream
	}
	return false;
}

CSVReader& CSVReader::iterate(const std::function<void(Field, int, int)>& onField){
	int row = 0;
	int col = 0;
	for(const auto& field : mData){
		onField(field, col, row);
		if(++col == mNumCols){
			col = 0;
			++row;
			// catch spurious fields (like new lines) at end
			if(row == numRows()) break;
		}
	}
	return *this;
}

bool CSVReader::parseFields(){
	mHeader.clear();
	mData.clear();

	Field field;
	field.ptr = mRawData.data();
	mNumCols = 0;
	bool inFirstRow = true;
	bool inNewLine = false; // for handling different line endings \n, \n\r, ...
	bool inString = false; // reading a double-quoted field?

	for(unsigned i=0; i<(mRawData.size()-1); ++i){
		auto& c = mRawData[i];

		auto setFieldBeg = [&](){
			c = '\0'; // so C-strings work
			field.ptr = mRawData.data() + i+1;
		};

		auto addField = [&](){
			if(inFirstRow) ++mNumCols;
			auto * beg = (char *)field.ptr;
			auto * end = mRawData.data() + i;
			field.size = end - beg;
			if(field.size >= 2){
				auto& c1 = beg[0];
				auto& c2 = end[-1];
				if('\"'==c1 && '\"'==c2){
					c1 = c2 = '\0'; // so C-strings work
					++field.ptr;
					field.size -= 2;
				}
			}
			if(mHasHeader && inFirstRow){
				mHeader.push_back(field);
			} else {
				mData.push_back(field);
			}
		};

		if('\"' == c){
			inNewLine = false;
			inString ^= true;
		} else if(!inString){ // if in double-quotes, ignore parsing rules
			if('\n' == c || '\r' == c){ // at end of row
				if(!inNewLine) addField();
				setFieldBeg();
				inFirstRow = false;
				inNewLine = true;
			} else { // in row
				inNewLine = false;
				if(mDelim == c){
					addField();
					setFieldBeg();
				}
			}
		}
	}
	return true;
}


CSVWriter& CSVWriter::beginRow(){ mWriteDelim=false; return *this; }

CSVWriter& CSVWriter::endRow(){ mSS << "\n"; return *this; }

CSVWriter& CSVWriter::clear(){ decltype(mSS)().swap(mSS); return *this; }

bool CSVWriter::writeFile(const std::string& path, bool append){
	auto flags = std::ios::out;
	if(append) flags |= std::ios::app;
	std::ofstream fs(path, flags);
	if(fs.good()){
		fs << mSS.str();
		return true;
	}
	return false;
}

bool CSVWriter::appendToFile(const std::string& path){
	return writeFile(path,true);
}
