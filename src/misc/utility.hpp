#include "generators.hpp"

#ifndef UTILITY_STL_H
#define UTILITY_STL_H

namespace misc
{
	const std::double_t minval_radix = 7.0;
	const std::double_t maxval_radix = 7.5;
    const std::size_t n_tbb_workers = 32;

	void init(std::vector<gen::ITEM>& a, \
		std::pair<std::size_t, std::size_t> range, std::size_t& count)
	{
		// Use STL Mersenne-Twister random values generator
		std::random_device rd; std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(range.first, range.second);

		if (a.size() <= 0) {
			if (count == 0)
				count = dist(gen);

			std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(4)
				<< " (array size = " << count << " mem: " << count * sizeof(gen::ITEM) / \
                    (std::double_t)std::pow(10,9) << " GiB)..." << "\n";
		}

		gen::generate_objects(a, count);
	}

	std::size_t sorted(std::vector<gen::ITEM> a, std::size_t& position)
	{
		bool is_sorted = true;

		// Iterate through the array and perform a check if its sorted
		for (std::vector<gen::ITEM>::iterator \
			_FwdIt = a.begin(); _FwdIt != a.end() - 1 && is_sorted; _FwdIt++)
		{
			if ((_FwdIt->key > (_FwdIt + 1)->key) ||
				(_FwdIt->key == (_FwdIt + 1)->key &&
					_FwdIt->value > (_FwdIt + 1)->value))
			{
				if (is_sorted == true)
					position = std::distance(a.begin(), _FwdIt);

				is_sorted = false;
			}
		}

		return is_sorted;
	}

	void print_out(std::vector<gen::ITEM>::iterator _First,
		std::vector<gen::ITEM>::iterator _Last)
	{
		// Print out the array
		for (auto it = _First; it != _Last; it++)
			std::cout << "(" << it->key << " " << it->value << ") ";

		std::cout << "\n";

		std::cout << "\n";
	}
}

#endif // UTILITY_STL_H
