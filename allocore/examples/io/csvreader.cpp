/*
Allocore Example: Reading CSV File

Description:
Reading data from a CSV (comma separated values) file.

Author:
Lance Putnam
*/

#include <iostream>
#include "allocore/io/al_CSV.hpp"


int main() {
	using namespace al;

	CSVReader r;

	// Indicate the first row should be treated as a header
	r.hasHeader(true);

	if(r.readFile(RUN_MAIN_SOURCE_DIR "../../share/data/test.csv")){

		std::cout << "Read " << r.numRows() << " rows, " << r.numCols() << " cols and " << r.data().size() << " data fields\n";

		// Print out column names
		for(auto field : r.header()){
			std::cout << field.c_str() << " ";
		}
		std::cout << std::endl;

		// Iterate over data fields, printing them out
		// This is where you convert strings to numbers (deserialize).
		r.iterate([&](auto field, int col, int row){
			switch(col){
			case 1: case 2: case 3:
				std::cout << field.toDouble() << " ";
				break;
			default:
				std::cout << field.c_str() << " ";
			}
			
			if(r.numCols() == col+1) std::cout << "\n";
		});

	} else {
		std::cout << "Error reading file\n";
	}
}
