#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> // count, min
#include <cstdlib> // atoi, atof, strtod
#include <cstdint> // int32_t
#include "allocore/io/al_CSVReader.hpp"


class Tokenizer{
public:
	Tokenizer(const std::string& src, char delim=',')
	:	mStream(src), mDelim(delim)
	{}

	bool operator()(){
		if(mStream.good()){
			getline(mStream, mToken, mDelim);
			return true;
		}
		return false;
	}

	const std::string& token() const { return mToken; }

private:
	std::stringstream mStream;
	std::string mToken;
	char mDelim;
};


CSVReader::~CSVReader() {
	for (auto row: mData) {
		delete[] row;
	}
	mData.clear();
}

size_t CSVReader::typeSize(CSVReader::DataType type) const{
	switch(type){
		case STRING:  return sizeof(char)*maxStringSize;
		case INTEGER: return sizeof(int32_t);
		case REAL:    return sizeof(double);
		case BOOLEAN: return sizeof(bool);
		default:      return 0;
	}
}

bool CSVReader::readFile(std::string fileName) {
	std::ifstream f(fileName);
	if (!f.is_open()) {
		std::cerr << "Could not open: " << fileName << std::endl;
		return false;
	}

	std::string line;

	{	// Get column names (assuming they are the first row)
		getline(f, line);
		Tokenizer tk(line, mDelim);
		mColumnNames.clear();
		while(tk()){
			mColumnNames.push_back(tk.token());
		}
	}

	auto derivingTypeIndex = mDataTypes.size();

	// No data types defined, so use a default
	if(mDataTypes.empty()){
		for(unsigned i=0; i<mColumnNames.size(); ++i) addType(REAL);
	}

	auto derivingTypes = [&](){ return derivingTypeIndex < mDataTypes.size(); };

	auto rowLength = calculateRowLength();

	while (getline(f, line)) {
		if (line.size() == 0) {
			continue;
		}
		if (std::count(line.begin(), line.end(), mDelim) == int(mDataTypes.size() - 1)) { // Check that we have enough commas
			Tokenizer tk(line, mDelim);
			char *row = new char[!derivingTypes() ? rowLength : mDataTypes.size()*maxStringSize];
			mData.push_back(row);
			char *data = mData.back();
			int byteCount = 0;
			for(auto& type : mDataTypes) {
				if(!tk()) break; // Failed to get next token (CSV field)
				const auto& field = tk.token();

				if(derivingTypes()){
					char * pEnd;
					std::strtod(field.c_str(), &pEnd);
					// pEnd == '\0' if number parse successful
					if(*pEnd != '\0') type = STRING;
					//printf("Column %u derived as %s\n", derivingTypeIndex, type==STRING?"string":"real");
					++derivingTypeIndex;
					if(!derivingTypes()){ // hit last column
						rowLength = calculateRowLength();
					}
				}

				switch (type){
				case STRING:{
					auto stringLen = std::min(maxStringSize-1, field.size());
					std::memcpy(data + byteCount, field.data(), stringLen * sizeof(char));
					data[byteCount + stringLen] = '\0';
					} break;
				case INTEGER:{
					int32_t val = std::atoi(field.data());
					std::memcpy(data + byteCount, &val, typeSize(type));
					} break;
				case REAL:{
					double val = std::atof(field.data());
					std::memcpy(data + byteCount, &val, typeSize(type));
					} break;
				case BOOLEAN:{
					bool val = field == "True" || field == "true";
					std::memcpy(data + byteCount, &val, typeSize(type));
					} break;
				case NONE:
					break;
				}

				byteCount += typeSize(type);
			}
		}
	}
	if (f.bad()) {
		std::cerr << "Error reading: " << fileName << std::endl;
		return false;
	}

	return true;
}

size_t CSVReader::columnByteOffset(int col) const {
	int offset = 0;
	for(int i = 0; i < col; i++){
		offset += typeSize(mDataTypes[i]);
	}
	return offset;
}

size_t CSVReader::calculateRowLength() const {
	return columnByteOffset(mColumnNames.size());
}
