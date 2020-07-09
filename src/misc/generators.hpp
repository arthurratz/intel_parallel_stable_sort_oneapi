#ifndef GENERATORS_STL_H
#define GENERATORS_STL_H

namespace gen
{
	typedef struct {
		unsigned short key;
		unsigned short value;
	} ITEM;

	template<class Container = tbb::concurrent_vector<ITEM>>
	void generate_objects(Container & a, std::size_t size)
	{
		std::random_device rd; std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist_key(1, 100);
		std::uniform_int_distribution<> dist_val(1, 255);

		a.resize(size);

		tbb::parallel_for(tbb::blocked_range<std::size_t>(0, size), \
			[&](const tbb::blocked_range<std::size_t>& r) {
				for (std::size_t index = r.begin(); index != r.end(); index++)
				{
					ITEM item = { 0 };
					std::memset((void*)& item, 0x00, sizeof(ITEM));
					item.key = dist_key(gen); item.value = dist_val(gen);

					a[index] = item;
				}
			});
	}
}

#endif // GENERATORS_STL_H
