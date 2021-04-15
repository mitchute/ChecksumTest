#include <bitset>
#include <iostream>
#include <numeric>
#include <vector>


unsigned long long roundWithPrecision_to_ullong(double &x, double const &precision) {
	// rounds to unsigned long long after multiplying by precision
	return (unsigned long long) (x * precision + .5);
}

std::string bitsetAddFloats(std::vector<double> &vals, double const &precision) {

	// example of using bitset to add floating point numbers after
	// converting decimal number to float at the given precision level

	// example: vals = {1.0, 2.0}; precision = 1E0 means 1.0 + 2.0 = 3.0 -> 0b01 + 0b10 = 0b11
	// example: vals = {1.0, 2.0}; precision = 1E1 means 10.0 + 20.0 = 30.0 -> 0b01010 + 0b10100 = 0b11110

	// adapted from: https://stackoverflow.com/a/13283448
	// adapted from: https://stackoverflow.com/a/7533881

	// set size up front
	size_t size = vals.size();

	// multiply by precision and convert to unsigned long long
	std::vector<unsigned long long> rounded(size);
	for (auto it = vals.begin(); it != vals.end(); ++it) {
		auto i = std::distance(vals.begin(), it);
		rounded[i] = roundWithPrecision_to_ullong(vals[i], precision);
	}

	// bitsets of values
	std::vector<std::bitset<sizeof(double) * CHAR_BIT>> bitsets(size);
	for (auto it = bitsets.begin(); it != bitsets.end(); ++it) {
		auto i = std::distance(bitsets.begin(), it);
		auto f = rounded[i];
		bitsets[i] = std::bitset<sizeof(double) * CHAR_BIT>(static_cast<unsigned long long>(f));
	}

	// add bitsets
	std::bitset<sizeof(double) * CHAR_BIT> const m("1");
	std::bitset<sizeof(double) * CHAR_BIT> result;
	for (auto i = 0; i < result.size(); ++i) {
		std::vector<unsigned long long> sumBitset(size + 1u);
		for (auto it = bitsets.begin(); it != bitsets.end(); ++it) {
			auto idx = std::distance(bitsets.begin(), it);
			auto &s = bitsets[idx];
			sumBitset[idx] = ((s >> i) & m).to_ullong();
		}
		auto it = sumBitset.end() - 1;
		auto idx = std::distance(sumBitset.begin(), it);
		sumBitset[idx] = (result >> i).to_ullong();
		std::bitset<sizeof(double) *CHAR_BIT> const diff = std::accumulate(sumBitset.begin(), sumBitset.end(), 0);
		result ^= (diff ^ (result >> i)) << i;
	}

	return result.to_string();
}

struct Material {

	// must match the number of values in this struct to include in the checksum
	constexpr static int numPropsToUseInChecksum = 3;

	double conductivity;
	double density;
	double specificHeat;

	Material() : conductivity(0.0), density(0.0), specificHeat(0.0) {}

	Material(double conductivity, double density, double specificHeat) {
		this->conductivity = conductivity;
		this->density = density;
		this->specificHeat = specificHeat;
	}
};

struct Construction {

	// must match the number of values in this struct to include in the checksum
	constexpr static int numPropsToUseInChecksum = 1;

	double resistance;
	std::vector<Material> materials;

	Construction() : resistance(0.0) {}

	explicit Construction(std::vector<Material> materials) { this->materials = materials; }
	explicit Construction(double resistance) { this->resistance = resistance; }
};

std::string constructionChecksum(Construction const &construction) {

	// compute checksum for construction

	// offset by prime numbers, hopefully there's less of a chance of reversed numbers adding to the same value
	constexpr double conductivityChecksumOffset = 7;
	constexpr double densityChecksumOffset = 13;
	constexpr double specificHeatChecksumOffset = 29;
	constexpr double resistanceChecksumOffset = 59;

	auto &materials = construction.materials;

	// collect material values for checksum
	std::vector<double> checksumInputs;
	for (auto &m : materials) {
		checksumInputs.push_back(m.conductivity * conductivityChecksumOffset);
		checksumInputs.push_back(m.density * densityChecksumOffset);
		checksumInputs.push_back(m.specificHeat * specificHeatChecksumOffset);
	}

	// collect
	checksumInputs.push_back(construction.resistance * resistanceChecksumOffset);

	constexpr double precision = 1E9;
	return bitsetAddFloats(checksumInputs, precision);
}

int main() {

	// values for testing standalone functions
	double testVal1 = 1;
	double testVal2 = 2;

	// result from using function
	std::vector<double> toChecksum = {testVal1, testVal2};
	const double precision = 1E0;
	std::string cs = bitsetAddFloats(toChecksum, precision);
	std::cout << cs << std::endl;

	// test materials
	Material m1 = Material(10.0, 1000.0, 3990.0);// sum: 5000
	Material m2 = Material(20.0, 990.0, 3990.0); // sum: 5000

	std::vector<Material> materials = {m1, m2};
	std::vector<Material> materialsReversed = {m2, m1};

	// test constructions
	Construction c1 = Construction(materials);
	Construction c2 = Construction(materialsReversed);
	Construction c3 = Construction(5000);// resistance = 5000
	std::cout << constructionChecksum(c1) << std::endl;
	std::cout << constructionChecksum(c2) << std::endl;
	std::cout << constructionChecksum(c3) << std::endl;

	return 0;
}
