#include <tbb/tbb.h>
#include <tbb/task.h>

#include <CL/sycl.hpp>
#if FPGA || FEMU
#include <CL/sycl/INTEL/fpga_extensions.hpp>
#endif

#include <oneapi/dpl/execution>
#include <oneapi/dpl/algorithm>
#include <dpct/dpl_extras/iterators.h>

#include <iostream>

#include "misc/utility.hpp"

#ifndef PARALLEL_SORT_STL_H
#define PARALLEL_SORT_STL_H

using namespace cl::sycl;
using namespace oneapi::dpl::execution;

namespace internal
{
	std::size_t g_depth = 0L;

	template<class Container = std::vector<gen::ITEM>, class _Pred>
	void qsort3w(Container& array, std::size_t _First, std::size_t _Last, _Pred compare)
	{
		if (_First >= _Last) return;
		
		std::size_t _Size = array.size(); g_depth++;
		if (_Size > 0)
		{
			std::size_t _Left = _First, _Right = _Last;
			bool is_swapped_left = false, is_swapped_right = false;
			typename Container::value_type _Pivot = array[_First];

			std::size_t _Fwd = _First + 1;
			while (_Fwd <= _Right)
			{
				if (compare(array[_Fwd], _Pivot))
				{
					is_swapped_left = true;
					std::swap(array[_Left], array[_Fwd]);
					_Left++; _Fwd++;
				}

				else if (compare(_Pivot, array[_Fwd])) {
					is_swapped_right = true;
					std::swap(array[_Right], array[_Fwd]);
					_Right--;
				}

				else _Fwd++;
			}

			tbb::task_group task_group;
			task_group.run([&]() {
				if (((_Left - _First) > 0) && (is_swapped_left))
					qsort3w(array, _First, _Left - 1, compare);
			});

			task_group.run([&]() {
                if (((_Last - _Right) > 0) && (is_swapped_right))
					qsort3w(array, _Right + 1, _Last, compare);
			});

			task_group.wait();
		}
	}

	template<class Container, class _Pred >
	void parallel_sort(Container& array, std::size_t _First, \
        std::size_t _Last, _Pred compare)
	{
		g_depth = 0L; 
        tbb::task_group task_group;
        if ((_Last - _First) > 1)
        {        
            std::size_t _Left = _First, _Right = _Last - 1;
            std::size_t _Mid = _First + (_Last - _First) / 2;
            typename Container::value_type _Pivot = array[_Mid];
        
            while (_Left <= _Right)
            {
                while (compare(array[_Left],_Pivot)) _Left++;
                while (compare(_Pivot, array[_Right])) _Right--;
            
                if (_Left <= _Right) {
                    std::swap(array[_Left], array[_Right]);
                    _Left++; _Right--;
                }
            }
        
            task_group.run([&]() {
                internal::qsort3w(array, _First, _Right, compare);
            });

            task_group.run([&]() {
                internal::qsort3w(array, _Left, _Last - 1, compare);
            });
        
            task_group.wait();
        }
        
        else {
            task_group.run_and_wait([&]() {
                internal::qsort3w(array, _First, _Last - 1, compare);
            });
        }
	}
    
  	template<class Container, class _CompKey, class _CompVals>
	void parallel_stable_sort(Container& array, sycl::queue device_queue,
		_CompKey comp_key, _CompVals comp_vals)
	{
        
#if FEMU | CPU
        cl::sycl::usm_allocator<gen::ITEM, usm::alloc::shared> q_array_alloc{ device_queue };      
        cl::sycl::usm_allocator<std::size_t, usm::alloc::shared> q_pv_alloc{ device_queue };      

        std::vector<std::size_t, usm_allocator<std::size_t, usm::alloc::shared>> pv_dd(q_pv_alloc);
        std::vector<gen::ITEM, usm_allocator<gen::ITEM, usm::alloc::shared>> array_dd(q_array_alloc);

        array_dd.reserve(array.size()); array_dd.resize(array.size());
        array_dd.assign(array.begin(), array.end()); array.clear();

        pv_dd.reserve(array_dd.size() + 1); pv_dd.resize(array_dd.size() + 1);
#elif FPGA | GPU       
        gen::ITEM* array_dd = (gen::ITEM*)sycl::malloc_device( array.size()*sizeof(gen::ITEM), device_queue );
        std::size_t* pv_dd = (std::size_t*)sycl::malloc_device( (array.size() + 1)*sizeof(std::size_t), device_queue );
        
        device_queue.memset(&pv_dd[0], 0x00, sizeof(std::size_t) * (array.size() + 1));
        device_queue.wait_and_throw();
#endif
        
#if FEMU | CPU
        tbb::task_arena task_arena(misc::n_tbb_workers);
        task_arena.execute([&](){
            internal::parallel_sort(array_dd, 0L, array_dd.size(), comp_key);
        });
#elif FPGA | GPU
        tbb::task_arena task_arena(misc::n_tbb_workers);
        task_arena.execute([&](){
            internal::parallel_sort(array, 0L, array.size(), comp_key);
        });        
#endif        
        
#if FEMU | CPU
        device_queue.submit([&](sycl::handler& cgh) {
            cgh.parallel_for<class partition_kernel>( \
                cl::sycl::range<1>{ array_dd.size() - 1 }, \
                    [=, ptr=&array_dd[0], pv_ptr=&pv_dd[0]](cl::sycl::id<1> idx) {
                       pv_ptr[idx[0] + 1] = ((comp_key(ptr[idx[0]], ptr[idx[0] + 1]) ||
                           (comp_key(ptr[idx[0] + 1], ptr[idx[0]])))) ? idx[0] + 1 : 0L;
                });
        });     
        
        device_queue.wait_and_throw();

#elif FPGA | GPU
        auto e1 = device_queue.memcpy(&array_dd[0], &array[0], sizeof(gen::ITEM) * array.size());
        auto e2 = device_queue.submit([&](sycl::handler& cgh) {
            cgh.depends_on(e1);
            cgh.parallel_for<class partition_kernel>( \
                cl::sycl::range<1>{ array.size() - 1 }, \
                    [=, ptr=&array_dd[0], pv_ptr=&pv_dd[0]](cl::sycl::id<1> idx) {
                       pv_ptr[idx[0] + 1] = ((comp_key(ptr[idx[0]], ptr[idx[0] + 1]) ||
                           (comp_key(ptr[idx[0] + 1], ptr[idx[0]])))) ? idx[0] + 1 : 0L;
                });
        });

        device_queue.wait_and_throw();
#endif

#if FPGA | CPU | GPU
        auto policy = make_device_policy(device_queue);
#elif FEMU
        auto policy = oneapi::dpl::execution::par_unseq;
#endif

#if FEMU | CPU
        auto _LastIt = std::remove_if(policy, pv_dd.begin() + 1, \
             pv_dd.end(), [&](const std::size_t& pos){ return pos == 0L; });
        std::size_t size_new = std::distance(pv_dd.begin(), _LastIt);

        device_queue.wait_and_throw();
        
        pv_dd[size_new] = array_dd.size();

        task_arena.execute([&](){
            tbb::parallel_for(tbb::blocked_range<std::size_t>(0, size_new), \
                 [&](const tbb::blocked_range<std::size_t>& r) {
                     for (std::size_t index = r.begin(); index != r.end(); index++) 
                          internal::parallel_sort(array_dd, pv_dd[index], pv_dd[index + 1], comp_vals);
                 });        
        });

#elif FPGA | GPU
        std::vector<std::size_t> pv; pv.resize(array.size() + 1);
        device_queue.submit([&](cl::sycl::handler &cgh) {
            cgh.depends_on(e2);
            cgh.memcpy(&pv[0], &pv_dd[0], sizeof(std::size_t) * (array.size() + 1));
        }).wait();
        
        pv.push_back(array.size());
        
        task_arena.execute([&](){
            internal::parallel_sort(pv, 0L, pv.size(), \
                [&](const std::size_t& v1, const std::size_t& v2) { return v1 < v2; });
        });
        
        pv.erase(std::unique(policy, pv.begin(), pv.end()), pv.end());
        device_queue.wait_and_throw();
               
        task_arena.execute([&](){
            tbb::parallel_for(tbb::blocked_range<std::size_t>(0, pv.size() - 1), \
                 [&](const tbb::blocked_range<std::size_t>& r) {
                     for (std::size_t index = r.begin(); index != r.end(); index++) 
                          internal::parallel_sort(array, pv[index], pv[index + 1], comp_vals);
                 });        
        });
#endif

#if FEMU | CPU
        if (array_dd.size() > 0L) {
            array.resize(array_dd.size()); 
            array.assign(array_dd.begin(), array_dd.end());
        }
#endif
    }

    template<class BidirIt, class _CompKey, class _CompVals>
	void sequential_stable_sort(BidirIt _First, BidirIt _Last,
		_CompKey comp_key, _CompVals comp_vals)
	{
		std::sort(_First, _Last, comp_key);

		BidirIt	_First_p = _First, _Last_p = _First_p;
		for (BidirIt _FwdIt = _First + 1; _FwdIt != _Last; _FwdIt++)
		{
			if ((comp_key(*_FwdIt, *(_FwdIt - 1)) || (comp_key(*(_FwdIt - 1), *_FwdIt))))
			{
				_Last_p = _FwdIt;
				if (_First_p < _Last_p)
				{
					std::sort(_First_p, _Last_p, comp_vals);
					_First_p = _Last_p;
				}
			}
		}

		std::sort(_First_p, _Last, comp_vals);
	}
}

#endif // PARALLEL_SORT_STL_H
