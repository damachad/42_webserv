#include "wrapper_functions.hpp"

#include "stdlib.h"

#undef recv
#undef accept
#undef socket
#undef bind
#undef listen
#undef epoll_wait

int recv_call_counter = 0;
int accept_call_counter = 0;
int socket_call_counter = 0;
int bind_call_counter = 0;
int listen_call_counter = 0;
int epoll_wait_call_counter = 0;

bool check_failure(const char *fail_env, int call_counter) {
	const char *sys_count_env = std::getenv(fail_env);

	if (sys_count_env && std::atoi(sys_count_env) == call_counter) return true;
	return false;
}

ssize_t my_recv(int sockfd, void *buf, size_t len, int flags) {
	recv_call_counter++;

	if (check_failure("RECV_FAIL", recv_call_counter)) return -1;

	return ::recv(sockfd, buf, len, flags);
}

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	accept_call_counter++;

	if (check_failure("ACCEPT_FAIL", accept_call_counter)) return -1;

	return ::accept(sockfd, addr, addrlen);
}

int my_socket(int domain, int type, int protocol) {
	socket_call_counter++;

	if (check_failure("SOCKET_FAIL", socket_call_counter)) return -1;

	return ::socket(domain, type, protocol);
}

int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	bind_call_counter++;

	if (check_failure("BIND_FAIL", bind_call_counter)) return -1;

	return ::bind(sockfd, addr, addrlen);
}

int my_listen(int sockfd, int backlog) {
	listen_call_counter++;

	if (check_failure("LISTEN_FAIL", listen_call_counter)) return -1;

	return ::listen(sockfd, backlog);
}

int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents,
				  int timeout) {
	epoll_wait_call_counter++;

	if (check_failure("EPOLL_WAIT_FAIL", epoll_wait_call_counter)) return -1;

	return ::epoll_wait(epfd, events, maxevents, timeout);
}

#define recv my_recv
#define accept my_accept
#define socket my_socket
#define bind my_bind
#define listen my_listen
#define epoll_wait my_epoll_wait
