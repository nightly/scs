#if (SCS_MEMORY_TRACKER == 1)

	#include <memory>
	#include <iostream>
	#include <ostream>	

	struct AllocationMetrics {
		size_t total_allocated = 0;
		size_t total_freed = 0;

		size_t TotalUsage() const { return total_allocated; }
		void PrintUsage(std::ostream& os = std::cout) const {
			os << "[Memory Metrics] Total usage: " << total_allocated << " bytes\n";
		}
		size_t CurrentUsage() const { return total_allocated - total_freed; }
	};

	static AllocationMetrics s_allocation_metrics;

	void* operator new(size_t size) {
		std::cout << "[Memory] Allocating " << size << " bytes\n";
		s_allocation_metrics.total_allocated += size;
		return malloc(size);
	}

	// operator new[]
	// operator delete[]

	void operator delete(void* memory, size_t size) {
		std::cout << "[Memory] Freeing " << size << " bytes\n";
		s_allocation_metrics.total_freed += size;
		free(memory);
	}

#endif 