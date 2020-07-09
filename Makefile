CXX = dpcpp
CXX_FLAGS = -Wno-everything
CPU_EXE_NAME = parallel_stable_sort.cpu
GPU_EXE_NAME = parallel_stable_sort.gpu
FEMU_EXE_NAME = parallel_stable_sort.femu
FPGA_EXE_NAME = parallel_stable_sort.fpga
OBJ_NAME = parallel_stable_sort.o
SOURCES = src/parallel_stable_sort_oneapi.cpp
BINDIR = .

all: femu

femu:
	[ -d $(BINDIR) ] || mkdir $(BINDIR)
	$(CXX) -DFEMU=1 -D_PSTL_FPGA_EMU=1 -D_PSTL_COMPILE_KERNEL=1 $(CXX_FLAGS) -std=c++17 -o $(BINDIR)/$(FEMU_EXE_NAME) $(SOURCES) -fsycl -fintelfpga -lstdc++ -lsycl -ltbb

fpga:
	[ -d $(BINDIR) ] || mkdir $(BINDIR)
	$(CXX) -DFPGA=1 -D_PSTL_FPGA_DEVICE=1 $(CXX_FLAGS) -std=c++11 -o $(BINDIR)/$(FPGA_EXE_NAME) $(SOURCES) -fsycl -fintelfpga -Xshardware -lsycl -ltbb -Xsv -reuse-exe=$(FPGA_EXE_NAME)

cpu:
	[ -d $(BINDIR) ] || mkdir $(BINDIR)
	$(CXX) -DCPU=1 $(CXX_FLAGS) -std=c++17 -o $(BINDIR)/$(CPU_EXE_NAME) $(SOURCES) -fsycl -lstdc++ -lOpenCL -lsycl -ltbb

gpu:
	[ -d $(BINDIR) ] || mkdir $(BINDIR)
	$(CXX) -DGPU=1 $(CXX_FLAGS) -std=c++11 -o $(BINDIR)/$(GPU_EXE_NAME) $(SOURCES) -fsycl -lstdc++ -lOpenCL -lsycl -ltbb