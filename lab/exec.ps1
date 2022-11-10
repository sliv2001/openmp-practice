echo "Part 1. Sequential"
..\build\sequential.exe
echo ""
echo "Parallel MPI-based"
mpiexec ..\build\mpi_parallel.exe
echo ""
echo "Parallel OMP-based"
..\build\omp_parallel.exe
echo ""
echo "Part 2. Task 1a. MPI-based"
mpiexec ..\build\mpi_parallel_1a.exe
echo ""
echo "Task 1b. OMP-based"
..\build\omp_parallel_1b.exe