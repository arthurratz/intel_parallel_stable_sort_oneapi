//==============================================================
// Copyright © 2019 Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#include <CL/sycl.hpp>

#if FPGA || FEMU
#include <CL/sycl/intel/fpga_extensions.hpp>
#endif

#include <dpstd/execution>
#include <dpstd/algorithm>
#include <dpstd/iterators.h>

#include <ctime>
#include <deque>
#include <chrono>
#include <vector>
#include <random>
#include <string>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <functional>

#include "parallel_stable_sort_oneapi.hpp"

using namespace cl::sycl;

#ifndef PARALLEL_STABLE_SORT_STL_CPP
#define PARALLEL_STABLE_SORT_STL_CPP

namespace parallel_sort_impl
{
	void stress_test(sycl::queue device_queue)
	{
		while (true)
		{
			std::size_t count = 0L;
			std::vector<gen::ITEM> array, array_copy;

			std::cout << "generating an array of objects";
			// Generate the values of data items in the array to be sorted
			misc::init(array, std::make_pair(std::pow(10, misc::minval_radix), \
				std::pow(10, misc::maxval_radix)), count);

            // Resize the array and copy all items to the second array to be sorted in parallel
			array_copy.resize(array.size());
			std::copy(array.begin(), array.end(), array_copy.begin());

			std::cout << "sorting an array... (sort type: array of objects)\n";

			// Obtain the value of walltime prior to performing the sequential sorting
			auto time_s = std::chrono::high_resolution_clock::now();

			// Perform the sequental sort by using std::sort function
			internal::sequential_stable_sort(array.begin(), array.end(),
				[&](const gen::ITEM& item1, const gen::ITEM& item2) { return item1.key < item2.key;  },
				[&](const gen::ITEM& item1, const gen::ITEM& item2) { return item1.value < item2.value;  });

			// Obtain the value of walltime after to performing the sequential sorting
			auto time_f = std::chrono::high_resolution_clock::now();

			// Compute the overall execution walltime
			auto std_sort_time_elapsed = std::chrono::duration<double>(time_f - time_s);

			array.clear();

			std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(4)
				<< "execution time (std::sort): " << std_sort_time_elapsed.count() << " seconds ";

			// Obtain the value of walltime prior to performing the parallel sorting
			time_s = std::chrono::high_resolution_clock::now();
            
			// Perform the parallel sorting
			internal::parallel_stable_sort(array_copy, device_queue,
				[&](const gen::ITEM& item1, const gen::ITEM& item2) { return item1.key < item2.key;  },
				[&](const gen::ITEM& item1, const gen::ITEM& item2) { return item1.value < item2.value;  });

			// Obtain the value of walltime after to performing the parallel sorting
			time_f = std::chrono::high_resolution_clock::now();

			std::size_t position = 0L;
			// Compute the overall execution walltime spent for parallel sorting
			auto intro_sort_time_elapsed = std::chrono::duration<double>(time_f - time_s);

			// Perform a verification if the array is properly sorted
			bool is_sorted = misc::sorted(array_copy, position);

			// Compute the actual performance gain as the difference of execution walltime values
			std::double_t time_diff = \
				std_sort_time_elapsed.count() - intro_sort_time_elapsed.count();

			std::cout << "<--> (internal::parallel_stable_sort): " << intro_sort_time_elapsed.count() << " seconds " << "\n";

			std::cout << "verification: ";

			if (is_sorted == false) {
				std::cout << "failed at pos: " << position << "\n";
				std::cin.get();
				// Print out the array if the sorting has failed
				//misc::print_out(array_copy.begin() + position, array_copy.end() + position + 10);
			}

			else {
				// Print out the statistics
				std::double_t ratio = intro_sort_time_elapsed.count() / \
					(std::double_t)std_sort_time_elapsed.count();
				std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(2)
					<< "passed... [ time_diff: " << std::fabs(time_diff)
					<< " seconds (" << "ratio: " << (ratio - 1.0) * 100 << "% (" << (1.0f / ratio) << "x faster)) depth = "
					<< internal::g_depth << " ]" << "\n";
			}

			std::cout << "\n";

			array_copy.clear();
		}
	}

	void parallel_sort_demo(sycl::queue device_queue)
	{
		std::size_t count = 0L;
		std::cout << "Enter the number of data items N = "; std::cin >> count;

		std::cout << "generating an array of objects";

		std::vector<gen::ITEM> array;
		misc::init(array, std::make_pair(std::pow(10, misc::minval_radix), \
			std::pow(10, misc::maxval_radix)), count);
        
		auto time_s = std::chrono::high_resolution_clock::now();

		std::cout << "sorting the array of objects in parallel...\n";

		internal::parallel_stable_sort(array, device_queue,
			[&](const gen::ITEM& item1, const gen::ITEM& item2) { return item1.key < item2.key;  },
			[&](const gen::ITEM& item1, const gen::ITEM& item2) { return item1.value < item2.value;  });

		auto time_f = std::chrono::high_resolution_clock::now();

		std::size_t position = 0L;
		auto intro_sort_time_elapsed = std::chrono::duration<double>(time_f - time_s);

		std::cout << "Execution Time: " << intro_sort_time_elapsed.count()
				  << " seconds " << "depth = " << internal::g_depth << " ";

		bool is_sorted = misc::sorted(array, position);

		std::cout << "(verification: ";

		if (is_sorted == false) {
			std::cout << "failed at pos: " << position << "\n";
			std::cin.get();
			misc::print_out(array.begin() + position, array.end() + position + 10);
		}

		else {
			std::cout << "passed...)" << "\n";
		}

		char option = '\0';
		std::cout << "Do you want to output the array [Y/N]?"; std::cin >> option;

		if (option == 'y' || option == 'Y')
			misc::print_out(array.begin(), array.end());
	}
}

constexpr std::size_t n_tbb_workers = 31L;

int main()
{
	std::string logo = "\nParallel Stable Sort v.3.00 (CPU+GPU+FPGA) by Arthur V. Ratz";
	std::cout << logo << "\n\n";

#if FEMU
    sycl::intel::fpga_emulator_selector device_selector{};
#elif CPU
	cpu_selector device_selector{};
#elif GPU
	gpu_selector device_selector{};
#elif FPGA
	sycl::intel::fpga_selector device_selector{};
#else 
	default_selector device_selector{};
#endif
	queue device_queue(device_selector);

#if CPU    
	std::cout << "Device : " << device_queue.get_device().get_info<info::device::name>() \ 
        << " (" << device_queue.get_device().get_info<info::device::max_compute_units>() << " cores)" << std::endl;
#else
	std::cout << "Device : " << device_queue.get_device().get_info<info::device::name>() \ 
        << " @ " << device_queue.get_device().get_info<info::device::max_clock_frequency>() << "Mhz (" << \ 
            device_queue.get_device().get_info<info::device::max_compute_units>() << " cores)" << std::endl;
#endif

	char option = '\0';
	std::cout << "\nDo you want to run stress test first [Y/N]?"; std::cin >> option;
	std::cout << "\n";

	if (option == 'y' || option == 'Y')
		parallel_sort_impl::stress_test(device_queue);
	if (option == 'n' || option == 'N')
		parallel_sort_impl::parallel_sort_demo(device_queue);

	return 0;
}

#endif // PARALLEL_STABLE_SORT_STL_CPP