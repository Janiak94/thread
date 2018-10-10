# Assignment report on threads (hpcgp xxx)

## Program layout

The program is implemented in a single file. It consists of a main thread, a thread that handles printing to file and t threads that perform calculation. First, the main thread parses the input arguments: degree of x, matrix size and number of threads t. After checking that they are correcltly formatted, the correct complex roots to x^d = 1 are calculated and stored. The threads handling the calculations are created as well as the thread that is printing the results to files. A global storage array is used to save the reached root and number of iterations it took to get there.

A calculation thread starts with obtaining consecutive points corresponding to an entire line in the global storage array, to use as starting points for the newton algorithm. The points are assigned using a global variable storing the first index of the next sequence of available points. After a thread has claimed the next sequence, the index is incremented. To stop multiple threads from using the same points, mutual exclusion is used to only allow one thread accessing the index at a time. The next step of the thread is to find the which root each point converges to, and the number of iterations required. The implementation of the newton algorithm can be summarized by the following steps:

1) Check if x_k is in the ring in the complex plane defined by radiuses (1 - tol^2, 1 + tol^2). The reason is that every root has absolute value 1, and therefore lies in the set.
	1) If no, check if the absolute value of x_k approaches infinity (max tolerance).
		  * If no, the x_k is no root and must be incremented. x_(k+1) = (x^d-1)/(d*x^(d-1)). x^(d-1) and x^d are calculated by repeated multiplication in a for loop.
		  * If yes, x_k is approaching infinity and the algorithm is stopped. Set the element in the global storage array corresponding to the start point to indicate the "infinity root" was reached.
	2) If yes, loop through all roots and check if |x_k - root_i| < tol for some root i.
		  * If yes, record which root has been found in the global storage array and stop the algorithm.
		  * Else, increment x_k in the same manner as above.

2) Chose next point, if there are any left in the buffer assigned to the thread, and repeat step (1). Otherwise try to acquire a new set of starting points. If there are none left (the global index is equal to the total number of points) the thread is shut down.

Meanwhile, there is an active thread writing the results to two files. It tries to access the elements in the storage array corresponding to the first sequence of starting points. If there are no results at this point of time, the thread will sleep for a short time and then try again. When the thread finds something in the array it will copy the values from the array. It will assign a color and grayscale value according to the values of the copied points and then write it to the two files. The scheme is repeated for the next sequence of starting points, until all points have been written to the file.

## Design ideas

We chose to use local buffers for the calculation threads and the writing thread. The calculation threads calculates a full line of points and stores them to local buffers, only accessing the global buffer for storage once for each chunck of points. Local buffers enables more parallell computations since it reduces the calls to the critical sections that are locked with mutex calls. In the writing thread a similair approch is used to reduce the number of calls to both critical sections and fwrite. The size of local buffers as a full line of points is to ensure that the local buffers are always filled.

In the writing thread, memcpy was used insead of sprintf to fill up the local buffers ahead of writing to the files, which proved to be a faster implementation.

When ordering the if-else statements we made sure that the most likely case was placed in the "if-statement" to reduce the number of jumps in the code. Our assuption is that it will lead to more contiguous order of execution.  

An idea we had from the begining was to prefer global variables instead of passing arguments to the thread functions. It was mainly used for convenience and could be changed, we would then get smaller scopes for the variables which is preferable in some cases. Later we also added local copies of some global variables in the functions such as the degree of the polynomial. We had an idea that it would give the threads faster access to the variables, but in all honesty we are actually not sure about this.

When checking if a point is close to any root, we check if the squared distance  is closer than the squared tolerance. We use this scheme to avoid the square root operation.  


## Performance

We profiled our program with gprof, gcov, perf and valgrind with 10 threads, 1000x1000 points and a seventh degree polynomial.
  * gprof
  
	We saw that almost all of the time is spent inside the calculation thread and inside "\_\_muldc3" (\~40% each). The function call is probably due to the fact that we use "complex.h" to handle our multiplication. With the "-ffast-math" flag to the compiler we eliminated the call to "\_\_muldc3" which slightly improved the performance by around 15%. We should however be cautious with the compiler flag, there is a reason for the program to call those functions. We checked the output from the program and it still looked fine so this time it seems like we got away with it.
Another solution could be to define our own multiplication for complex numbers and inline that function but the standard library which we used should have decent performance.
  * gcov
  
	This allowed us to see how many times each row of the code was excecuted and we saw that a particular line was called a lot, namely the line that checked which root was the closest. To reduce the number of excecutions we added another condition before the root check was made and we said that the check should only be done once the absolute value of the point was close to 1. This improved our performance.
  * perf
  
	Perf gives us the L1-cache load misses. For performance we want as few as possible. The number we got was slightly less than 6000 load misses. This seem ok when we consider that some lines in the code are executed several million times.
  * valgrind
  
	* tool=memcheck
	
		This showed us that we had no memory leaks. However the amount of memory used on the heap is pretty large with the current implementation, roughly 8 Mb If chars were used instead this would reduce by a factor 4 (if ints are 4 bytes long on gantenbein). Also, currently the memory on the heap is used throughout the whole execution. If a dynamic buffer was used the program could potentially use way more RAM.
	* tool=cachegrind
	
		This gave us similar data to perf, we have a very low miss rate for cache loading, less than 0.1%.
