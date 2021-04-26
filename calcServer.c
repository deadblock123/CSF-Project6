#include <stdio.h>      /* for snprintf */
#include "csapp.h"
#include "calc.h"
#include <string.h>
#include <pthread.h> 
#include <stdio.h>
#define LINEBUF_SIZE 1024

int chat_with_client(struct Calc *calc, int infd, int outfd);
int keep_going = 1;

//data strcture representing a client connection
struct ConnInfo {
	int clientfd;
	int serverfd;
	struct Calc *calc;
};

void *worker(void *arg) {
	struct ConnInfo *info = arg;

	pthread_detach(pthread_self());
	
	int result = chat_with_client(info->calc, info->clientfd, info->clientfd);
	close(info->clientfd);
	free(info);

	if (result == 0) {
		keep_going = 0;
	}
	
	return NULL;
}

void fatal(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

int chat_with_client(struct Calc *calc, int infd, int outfd) {
        rio_t in;
        char linebuf[LINEBUF_SIZE];

        /* wrap standard input (which is file descriptor 0) */
        rio_readinitb(&in, infd);

        /*
         * Read lines of input, evaluate them as calculator expressions,
         * and (if evaluation was successful) print the result of each
         * expression.  Quit when "quit" command is received.
         */
        int done = 0;
        while (!done) {
                ssize_t n = rio_readlineb(&in, linebuf, LINEBUF_SIZE);
                if (n <= 0) {
                        /* error or end of input */
                        done = 1;
                } else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0) {
                        /* quit command */
                        done = 1;
                } else if (strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0) {
			return 0;
		}	
		else {
                        /* process input line */
                        int result;
                        if (calc_eval(calc, linebuf, &result) == 0) {
                                /* expression couldn't be evaluated */
                                rio_writen(outfd, "Error\n", 6);
                        } else {
                                /* output result */
                                int len = snprintf(linebuf, LINEBUF_SIZE, "%d\n", result);
                                if (len < LINEBUF_SIZE) {
                                        rio_writen(outfd, linebuf, len);
                                }
                        }
                }
        }

	return 1;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fatal("Usage: ./arithserver <port>");
	}

	int server_fd = open_listenfd(argv[1]);
	if (server_fd < 0) {
		fatal("Couldn't open server socket\n");
	}
	
	struct Calc *calc = calc_create();
	while (keep_going) {
		int client_fd = Accept(server_fd, NULL, NULL);
		
		if (keep_going == 0) {
			close(client_fd);
			break;
		}

		if (client_fd > 0) {
			struct ConnInfo *info = malloc(sizeof(struct ConnInfo));
			info->clientfd = client_fd;
			info->serverfd = server_fd;
			info->calc = calc;
			pthread_t thr_id;
			if (pthread_create(&thr_id, NULL, worker, info) != 0) {
				fatal("pthread_create failed");
			}
			//keep_going = chat_with_client(calc, client_fd , client_fd);
		}
	}
	close(server_fd);
	calc_destroy(calc);

	return 0;
}
