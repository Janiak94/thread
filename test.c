#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define M_PI 3.141592653589793238462643383279502884197169399375105820974
#define BUFFER_SIZE 1000
#define TOL 1e-6
#define MAX_VAL 1e10
double complex *roots_exact;
int _d;
size_t _number_of_points;
size_t _current_index = 0;
pthread_mutex_t index_lock;
int *entr, **row_ptr;

void *printerThread() {
	char *colors[8]= {"255 0 0\t","0 255 0\t","0 0 255\t","255 255 0\t","0 255 255\t","255 0 255\t","120 60 0\t","200 200 50\t"};
	size_t number_of_points = _number_of_points;
	FILE *g_file, *c_file;
	struct timespec sleep_timespec;
	sleep_timespec.tv_sec = 0;
	sleep_timespec.tv_nsec = 50;
	char grays[256][256];
	for(int i = 0; i < 256; ++i){
		sprintf(grays[i],"%d %d %d\t",i,i,i);
	}

    	char c_file_name[50];
    	sprintf(c_file_name, "newton_attractors_x%d.ppm", _d);
    	char g_file_name[50];
    	sprintf(g_file_name,"newton_convergence_x%d.ppm", _d);

    	c_file = fopen(c_file_name, "w");
    	g_file = fopen(g_file_name, "w");

    	fprintf(c_file, "P3\n%d %d\n255\n", number_of_points, number_of_points);
    	fprintf(g_file, "P3\n%d %d\n255\n", number_of_points, number_of_points);
	char *gray, *color, buffer[50];
	gray = (char*) malloc(50*BUFFER_SIZE);
	color = (char*) malloc(50*BUFFER_SIZE);
	int *copy = (int*) malloc(2*BUFFER_SIZE*sizeof(int));
    	int g_temp,c_temp, counter = 0, total=0;
	size_t x;
	for(x = 0; x < 2*BUFFER_SIZE*(number_of_points*number_of_points/BUFFER_SIZE);){
		pthread_mutex_lock(&index_lock);
		if(entr[x] != -1){
			memcpy(copy, &entr[x], 2*BUFFER_SIZE*sizeof(int));
			x+=2*BUFFER_SIZE;
		}
		pthread_mutex_unlock(&index_lock);
		if(entr[x] == -1){
			nanosleep(&sleep_timespec, NULL);
			continue;
		}
		for(int y = 0;y < BUFFER_SIZE;){
			if(counter <= number_of_points){
				strcat(gray, grays[copy[2*y+1] < 255 ? 255-copy[2*y+1]: 0]);	
				strcat(color, colors[copy[2*y]]);
				counter++;
				++y;
			}else{
				strcat(gray, "\n");
				strcat(color, "\n");
				total+=counter;
				counter = 0;
			}
		}
		fwrite(color, strlen(color),1, c_file);
		fwrite(gray, strlen(gray),1, g_file);
		memset(gray,0,50*BUFFER_SIZE);
		memset(color,0,50*BUFFER_SIZE);
	}
	for(; x < 2*number_of_points*number_of_points;){
		pthread_mutex_lock(&index_lock);
		if(entr[x] != -1){
			memcpy(copy, &entr[x], 2*(number_of_points*number_of_points%BUFFER_SIZE)*sizeof(int));
			++x;
		}
		pthread_mutex_unlock(&index_lock);
		if(entr[x] == -1){
			nanosleep(&sleep_timespec, NULL);
			continue;
		}
		for(int y = 0; (x+y) < 2*number_of_points*number_of_points;){
			if(counter <= number_of_points){
				strcat(gray, grays[255-copy[2*y+1]]);	
				strcat(color, colors[copy[2*y]]);
				counter++;
				++y;
			}else{
				strcat(gray, "\n");
				strcat(color, "\n");
				total+=counter;
				counter = 0;
			}
		}
		fwrite(color, strlen(color),1, c_file);
		fwrite(gray, strlen(gray),1, g_file);
		memset(gray,0,50*BUFFER_SIZE);
		memset(color,0,50*BUFFER_SIZE);
	}
	printf("%d\n",total);
    	fclose(c_file);
    	fclose(g_file);
	free(copy);
	free(color);
	free(gray);
    	return NULL;
}

void *calculation_thread(){
	size_t number_of_points = _number_of_points, d = _d, i;
	int close_to_root;
	double dx = 4.0/(number_of_points-1);
	complex double x, value;
	pthread_mutex_lock(&index_lock);
	size_t assigned_index = _current_index;
	int out_buffer[2*BUFFER_SIZE];
	_current_index+=BUFFER_SIZE;
	pthread_mutex_unlock(&index_lock);
	complex double x_buffer[BUFFER_SIZE];

	while(assigned_index < number_of_points*number_of_points){
		int n;
		for(n = 0; n < BUFFER_SIZE; ++n){
			x_buffer[n] = dx*((assigned_index+n)%number_of_points) - 2 - I*(dx*((assigned_index+n)/number_of_points) - 2);
		}
		for(n = 0; n < BUFFER_SIZE; ++n){
			i = 0, close_to_root = -1;
			double cond;
			do {
				cond = creal(x_buffer[n]*conj(x_buffer[n]));
				if(cond > 1+TOL || cond < 1-TOL){
					if(fabs(creal(x_buffer[n])) < MAX_VAL && fabs(cimag(x_buffer[n])) < MAX_VAL){
						value = 1.0;
						for(int k = 0; k < d-1; ++k){
							value *= x_buffer[n];
						}
						x_buffer[n] = x_buffer[n] - (value*x_buffer[n] -1)/(d*value);
					}else{
						close_to_root = d;
						out_buffer[2*n] = close_to_root;
						out_buffer[2*n+1] = i;
						break;
					}
				}else{
					for(int j = 0; j < d ; ++j){
						if(creal((x_buffer[n]-roots_exact[j])*conj(x_buffer[n]-roots_exact[j])) <= TOL){
							close_to_root = j;
							out_buffer[2*n] = close_to_root;
							out_buffer[2*n+1] = i;
							break;
						}
					}
					if(close_to_root != -1){
						break;
					}else{
						value = 1.0;
						for(int k = 0; k < d-1; ++k){
							value *= x_buffer[n];
						}
						x_buffer[n] = x_buffer[n] - (value*x_buffer[n] -1)/(d*value);
					}
				}
				++i;
			}while(1);
		}
		pthread_mutex_lock(&index_lock);
		memcpy(&entr[2*assigned_index], out_buffer, 2*n*sizeof(int));
		assigned_index = _current_index;
		_current_index += BUFFER_SIZE;
		pthread_mutex_unlock(&index_lock);
	}
	return NULL;
}


int main(int argc, char *argv[]){
	size_t number_of_threads;
	size_t number_of_points;
	if (argc != 4) {
		printf("ERROR, must have 3 arguments.\n");
		return -1;
	}
	int arg1 = 0;
	int arg2 = 0;
	int arg3 = 0;
	for (int i = 1; i<4; ++i) {
		if (memcmp( "-t", argv[i], 2)== 0) {
			arg1 = 1;
			number_of_threads = (int) strtol(&argv[i][2], argv, 10);
		} else if (memcmp( "-l", argv[i], 2)== 0) {
			arg2 = 1;
			number_of_points = (int) strtol(&argv[i][2], argv, 10);
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
	entr = (int*) malloc(sizeof(int)* number_of_points*number_of_points*2);
	row_ptr = (int**) malloc(sizeof(int*) * number_of_points);
	for(size_t i = 0; i < number_of_points; ++i){
		row_ptr[i] = entr + i*2*number_of_points;
	}

	for(size_t i = 0; i< number_of_points*number_of_points*2; ++i)
    		entr[i] = -1;

	int debug = 1;
	roots_exact = (double complex *) malloc(sizeof(double complex) * _d);
	for (int i = 0; i<_d; ++i) {
		roots_exact[i] = cos(2*M_PI*i/_d) + sin(2*M_PI*i/_d) * I;
	}

	pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t) * number_of_threads);
	pthread_t *write_thread = (pthread_t*) malloc(sizeof(pthread_t));
	int ret;
	for(int i = 0; i < number_of_threads; ++i){
		if((ret =pthread_create(threads+i, NULL, calculation_thread, NULL))){
			printf("Error creating thread: %d\n", ret);
			exit(1);
		}
	}
	pthread_create(write_thread, NULL, printerThread, NULL);
	for(int i = 0; i < number_of_threads; ++i){
		if((ret = pthread_join(threads[i], NULL))){
			printf("Error joining thread: %d\n", ret);
			exit(1);
		}
	}
	pthread_join(*write_thread,NULL);

	pthread_mutex_destroy(&index_lock);
	free(threads);
	free(write_thread);
	free(entr);
	free(row_ptr);
	free(roots_exact);
	return 0;
}
