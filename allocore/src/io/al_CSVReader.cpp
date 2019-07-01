#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> // count, min
#include <cstdlib> // atoi, atof
#include <cstdint> // int32_t
#include "allocore/io/al_CSVReader.hpp"

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
	size_t rowLength = calculateRowLength();

	getline(f, line); // TODO Process column names
//	std::cout << line << std::endl;
	while (getline(f, line)) {
		if (line.size() == 0) {
			continue;
		}
		if (std::count(line.begin(), line.end(), mDelim) == int(mDataTypes.size() - 1)) { // Check that we have enough commas
			std::stringstream ss(line);
			char *row = new char[rowLength];
			mData.push_back(row);
			char *data = mData.back();
			int byteCount = 0;
			for(auto type:mDataTypes) {
				std::string field;
				std::getline(ss, field, mDelim);

				switch (type){
				case STRING:{
					auto stringLen = std::min(maxStringSize, field.size());
					std::memcpy(data + byteCount, field.data(), stringLen * sizeof(char));
					break;}
				case INTEGER:{
					int32_t val = std::atoi(field.data());
					std::memcpy(data + byteCount, &val, typeSize(type));
					break;}
				case REAL:{
					double val = std::atof(field.data());
					std::memcpy(data + byteCount, &val, typeSize(type));
					break;}
				case BOOLEAN:{
					bool val = field == "True" || field == "true";
					std::memcpy(data + byteCount, &val, typeSize(type));
					break;}
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

std::vector<double> CSVReader::getColumn(int index) const {
	std::vector<double> out;
	int offset = 0;
	for (int i = 0; i < index; i++) {
		offset += typeSize(mDataTypes[i]);
	}
//	std::cout << offset << std::endl;
	for (auto row: mData) {
		double *val = (double *)(row + offset);
		out.push_back(*val);
	}
	return out;
}

size_t CSVReader::calculateRowLength() const {
	size_t len = 0;;
	for(auto type:mDataTypes) {
		len += typeSize(type);
	}
//	std::cout << len << std::endl;
	return len;
}
