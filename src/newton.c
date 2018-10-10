#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define TOL 1e-6
#define MAX_VAL 1e10
double complex *roots_exact;
unsigned long int _number_of_points, _current_index = 0, BUFFER_SIZE;
pthread_mutex_t index_lock;
int *entr, _d;

void *printerThread() {
	char *colors[10]= {"255 000 000 ","000 255 000 ","000 000 255 ","255 255 000 ","000 255 255 ","255 000 255 ","120 060 000 ","200 200 050 ", "000 000 000 ", "150 000 150 "};
	unsigned long int number_of_points = _number_of_points;
	FILE *g_file, *c_file;
	struct timespec sleep_timespec;
	sleep_timespec.tv_sec = 0;
	sleep_timespec.tv_nsec = 1e5;
	// assigning gray scale
	char grays[256][13];
	for(int i = 0; i < 10; ++i){
		sprintf(grays[i],"00%i 00%i 00%i ",i,i,i);
	}
	for(int i = 10; i < 100; ++i){
		sprintf(grays[i],"0%d 0%d 0%d ",i,i,i);
	}
	for(int i = 100; i < 256; ++i){
		sprintf(grays[i],"%d %d %d ",i,i,i);
	}

    	char c_file_name[50];
    	sprintf(c_file_name, "newton_attractors_x%d.ppm", _d);
    	char g_file_name[50];
    	sprintf(g_file_name,"newton_convergence_x%d.ppm", _d);

    	c_file = fopen(c_file_name, "w");
    	g_file = fopen(g_file_name, "w");

    	fprintf(c_file, "P3\n%d %d\n255\n", number_of_points, number_of_points);
    	fprintf(g_file, "P3\n%d %d\n255\n", number_of_points, number_of_points);
	char *gray, *color;
	gray = (char*) malloc(12*number_of_points+1);
	color = (char*) malloc(12*number_of_points+1);
	int *copy = (int*) malloc(2*BUFFER_SIZE*sizeof(int));
    	int counter = 0;
	for(unsigned long int x = 0; x < 2*number_of_points*number_of_points;){
		pthread_mutex_lock(&index_lock);
		//copy from main buffer to local buffer
		if(entr[x] != -1){
			memcpy(copy, &entr[x], 2*BUFFER_SIZE*sizeof(int));
		}
		pthread_mutex_unlock(&index_lock);
		//if no element has been written to main buffer, wait
		if(entr[x] == -1){
			nanosleep(&sleep_timespec, NULL);
			continue;
		}
		for(int y = 0; y < 2*BUFFER_SIZE;){
			//write elements to the line buffer
			if(counter < number_of_points){
				memcpy(&gray[12*counter], grays[copy[y+1] < 255 ? copy[y+1] : 0], 12);
				memcpy(&color[12*counter], colors[copy[y]], 12);
				++counter;
				y+=2;
			}
			//write buffer (line) to files
			else{
				memcpy(&gray[12*number_of_points], "\n", 1);
				memcpy(&color[12*number_of_points], "\n", 1);
				fwrite(gray, 12*number_of_points+1,1, g_file);
				fwrite(color, 12*number_of_points+1,1, c_file);
				counter = 0;
			}
		}
		//increment for looå
		x+=2*BUFFER_SIZE;
	}
	//write the last buffer to files
	fwrite(color, 12*number_of_points+1,1, c_file);
	fwrite(gray, 12*number_of_points+1,1, g_file);
    	fclose(c_file);
    	fclose(g_file);
	free(copy);
	free(color);
	free(gray);
    	return NULL;
}

void *calculation_thread(){
	unsigned long int number_of_points = _number_of_points;
	int d = _d, i;
	int close_to_root;
	double dx = 4.0/(number_of_points-1);
	complex double x, value;
	pthread_mutex_lock(&index_lock);
	unsigned long int assigned_index = _current_index;
	int out_buffer[2*BUFFER_SIZE];
	_current_index+=BUFFER_SIZE;
	pthread_mutex_unlock(&index_lock);
	//buffer that hold precomputed complex values for each pixel
	complex double x_buffer[BUFFER_SIZE];

	while((unsigned long int) assigned_index < (unsigned long int) number_of_points*number_of_points){
		//where in the assigned buffer are we?
		int n;
		//compute complex value for each pixel in the buffer
		for(n = 0; n < BUFFER_SIZE; ++n){
			x_buffer[n] = dx*((assigned_index+n)%number_of_points) - 2 - I*(dx*((assigned_index+n)/number_of_points) - 2);
		}
		for(n = 0; n < BUFFER_SIZE; ++n){
			i = 0, close_to_root = -1;
			double cond;
			//check convergence, break if
			while(1){
				cond = creal(x_buffer[n]*conj(x_buffer[n]));
				//are we outside the ring containing the roots?
				if(cond > 1+TOL || cond < 1-TOL){
					//are we still in the search region, if so update the point
					if(fabs(creal(x_buffer[n])) < MAX_VAL && fabs(cimag(x_buffer[n])) < MAX_VAL){
						value = 1.0;
						for(int k = 0; k < d-1; ++k){
							value *= x_buffer[n];
						}
						x_buffer[n] = x_buffer[n] - (value*x_buffer[n] -1)/(d*value);
					}
					//we have diverged and end the iterations
					else{
						close_to_root = d;
						out_buffer[2*n] = close_to_root;
						out_buffer[2*n+1] = i;
						break;
					}
				}
				//we are in the ring that contains all roots
				else{
					//which root are we close to?
					for(int j = 0; j < d ; ++j){
						if(creal((x_buffer[n]-roots_exact[j])*conj(x_buffer[n]-roots_exact[j])) <= TOL){
							close_to_root = j;
							out_buffer[2*n] = close_to_root;
							out_buffer[2*n+1] = i;
							break;
						}
					}
					//if root found, break
					if(close_to_root != -1){
						break;
					}
					//update the point
					else{
						value = 1.0;
						for(int k = 0; k < d-1; ++k){
							value *= x_buffer[n];
						}
						x_buffer[n] = x_buffer[n] - (value*x_buffer[n] -1)/(d*value);
					}
				}
				//increment number of iterations
				++i;
			}
		}
		//put in main buffer when all points in local buffer are calculated and assign a new buffer
		pthread_mutex_lock(&index_lock);
		memcpy(&entr[2*assigned_index], out_buffer, 2*BUFFER_SIZE*sizeof(int));
		assigned_index = _current_index;
		_current_index += BUFFER_SIZE;
		pthread_mutex_unlock(&index_lock);
	}
	return NULL;
}


int main(int argc, char *argv[]){
	size_t number_of_threads;
	unsigned long int number_of_points;
	if (argc != 4) {
		printf("ERROR, must have 3 arguments.\n");
		return -1;
	}
	int arg1 = 0;
	unsigned long int arg2 = 0;
	int arg3 = 0;
	for (int i = 1; i<4; ++i) {
		if (memcmp( "-t", argv[i], 2)== 0) {
			arg1 = 1;
			number_of_threads = (int) strtol(&argv[i][2], argv, 10);
		} else if (memcmp( "-l", argv[i], 2)== 0) {
			arg2 = 1;
			number_of_points = (unsigned long int) strtol(&argv[i][2], argv, 10);
		} else {
			arg3 = 1;
			_d = (int) strtol(argv[i], argv, 10);
		}
	}
	if (arg1 * arg2 * arg3 == 0) {
		printf("ERROR, all 3 arguments must be correct.\n");
		return -1;
	}
	_number_of_points = number_of_points;
	//make each buffer a row long
	BUFFER_SIZE = number_of_points;
	entr = (int*) malloc(sizeof(int)* number_of_points*number_of_points*2);

	//set all values of the main buffer to -1 to indicate that they have not been set
	for(unsigned long int i = 0; i< (unsigned long int) number_of_points*number_of_points*2; ++i){
    		entr[i] = -1;
	}

	roots_exact = (double complex *) malloc(sizeof(double complex) * _d);
	for (int i = 0; i<_d; ++i) {
		roots_exact[i] = cos(2*M_PI*i/_d) + sin(2*M_PI*i/_d) * I;
	}

	//allocate memory for threads
	pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t) * number_of_threads);
	pthread_t *write_thread = (pthread_t*) malloc(sizeof(pthread_t));
	int ret;
	//create threads
	for(int i = 0; i < number_of_threads; ++i){
		if((ret =pthread_create(threads+i, NULL, calculation_thread, NULL))){
			printf("Error creating thread: %d\n", ret);
			exit(1);
		}
	}
	if((ret = pthread_create(write_thread, NULL, printerThread, NULL))){
		printf("Error creating thread: %d\n", ret);
		exit(1);
	}
	//join threads
	for(int i = 0; i < number_of_threads; ++i){
		if((ret = pthread_join(threads[i], NULL))){
			printf("Error joining thread: %d\n", ret);
			exit(1);
		}
	}
	if((ret = pthread_join(*write_thread,NULL))){
		printf("Error creating thread: %d\n", ret);
		exit(1);
	}
	pthread_mutex_destroy(&index_lock);
	free(threads);
	free(write_thread);
	free(entr);
	free(roots_exact);
	return 0;
}
