cmake_minimum_required(VERSION 3.20)
project(lab)
include(FindOpenMP)
include(FindMPI)

if(${OpenMP_FOUND})
else()
	message(FATAL_ERROR "No OpenMP found")
endif()
if(${MPI_FOUND})
else()
	message(FATAL_ERROR "No MPI found")
endif()

set(COMPILE_OPTIONS ${COMPILE_OPTIONS} OpenMP_CXX_FLAGS)

add_executable(sequential sequential.cpp)
target_link_libraries(sequential PUBLIC OpenMP::OpenMP_CXX)

add_executable(omp_parallel ompParallel.cpp)
target_link_libraries(omp_parallel PUBLIC OpenMP::OpenMP_CXX)

add_executable(mpi_parallel mpiParallel.cpp)
include_directories(mpi_parallel ${MPI_INCLUDE_PATH})
target_link_libraries(mpi_parallel ${MPI_C_LIBRARIES})

add_executable(mpi_parallel_1a additionalMPI.cpp)
include_directories(mpi_parallel_1a ${MPI_INCLUDE_PATH})
target_link_libraries(mpi_parallel_1a ${MPI_C_LIBRARIES})

add_executable(omp_parallel_1b additionalOMP.cpp)
target_link_libraries(omp_parallel_1b PUBLIC OpenMP::OpenMP_CXX)