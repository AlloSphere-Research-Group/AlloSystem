
#include "allocore/io/al_CSVReader.hpp"

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
	reader.readFile("allocore/share/data/test.csv");

	std::vector<RowTypes> rows = reader.copyToStruct<RowTypes>();
	for(auto row: rows) {
		std::cout << std::string(row.s) << " : "
		          << row.val1 << "   "
		          << row.val2 << "   "
		          << row.val3 << "   "
		          << (row.b ? "+" : "-")
		          << std::endl;
	}
	std::cout << " ---------- Num rows:" << rows.size() << std::endl;

	std::vector<double> column1 = reader.getColumn(1);
	for(auto value: column1) {
		std::cout << value << std::endl;
	}
	return 0;
}
