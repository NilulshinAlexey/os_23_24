#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>


typedef struct messagePipe{

	char *data;
	int fd1[2];
	int fd2[2];
	int size;
} Pipe;

void sendMessage(Pipe *p, int fd){
	
	write(fd, p->data, p->size);
}

size_t recieveMessage(Pipe *p, int fd){
	
	size_t size = read(fd, p->data, p->size);
	return size;
}

int main(){

	FILE* ifile = fopen("file", "r");
	FILE* ofile = fopen("outfile", "w");
	struct stat buff;
	fstat(fileno(ifile), &buff);
	size_t file_size = buff.st_size;

	Pipe p;
	p.data = (char *) malloc(file_size);
	
	if (pipe(p.fd1) < 0 || pipe(p.fd2) < 0){
		printf("Pipe error!\n");
		fclose(ifile);
		fclose(ofile);
		free(p.data);
		return 0;
	}

	pid_t pid = fork();

	if (pid < 0){
		printf("Fork error\n");
		fclose(ifile);
		fclose(ofile);
		free(p.data);
		return 0;
	}

	if (pid){
		
		printf("%d\n", pid);
		size_t size = 0;

		while(size = read(fileno(ifile), p.data, file_size)){
			
			p.data[size] = 0;
			sendMessage(&p, p.fd1[1]);
			p.size = size;

			recieveMessage(&p, p.fd2[0]);
			write(fileno(ofile), p.data, p.size); 
		}
	}

	else{
		
		while(recieveMessage(&p, p.fd1[0]) > 0)
			sendMessage(&p, p.fd2[1]);
	}

	free(p.data);
	fclose(ifile);
	fclose(ofile);
	return 0;
}
