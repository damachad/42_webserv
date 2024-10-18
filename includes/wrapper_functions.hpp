#ifndef WRAPPER_FUNCTIONS_HPP
#define WRAPPER_FUNCTIONS_HPP

#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define recv my_recv
#define accept my_accept
#define socket my_socket
#define bind my_bind
#define listen my_listen
#define epoll_wait my_epoll_wait

bool check_failure(const char *fail_env, int call_counter);
ssize_t my_recv(int sockfd, void *buf, size_t len, int flags);
int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_listen(int sockfd, int backlog);
int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents,
				  int timeout);

#endif
