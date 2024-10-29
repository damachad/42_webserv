#include "WrapperFunctions.hpp"

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

bool checkFailure(const char *fail_env, int call_counter) {
	const char *sys_count_env = std::getenv(fail_env);

	if (sys_count_env && std::atoi(sys_count_env) == call_counter) return true;
	return false;
}

ssize_t myRecv(int sockfd, void *buf, size_t len, int flags) {
	recv_call_counter++;

	if (checkFailure("RECV_FAIL", recv_call_counter)) return -1;

	return ::recv(sockfd, buf, len, flags);
}

int myAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	accept_call_counter++;

	if (checkFailure("ACCEPT_FAIL", accept_call_counter)) return -1;

	return ::accept(sockfd, addr, addrlen);
}

int mySocket(int domain, int type, int protocol) {
	socket_call_counter++;

	if (checkFailure("SOCKET_FAIL", socket_call_counter)) return -1;

	return ::socket(domain, type, protocol);
}

int myBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	bind_call_counter++;

	if (checkFailure("BIND_FAIL", bind_call_counter)) return -1;

	return ::bind(sockfd, addr, addrlen);
}

int myListen(int sockfd, int backlog) {
	listen_call_counter++;

	if (checkFailure("LISTEN_FAIL", listen_call_counter)) return -1;

	return ::listen(sockfd, backlog);
}

int myEpollWait(int epfd, struct epoll_event *events, int maxevents,
				int timeout) {
	epoll_wait_call_counter++;

	if (checkFailure("EPOLL_WAIT_FAIL", epoll_wait_call_counter)) return -1;

	return ::epoll_wait(epfd, events, maxevents, timeout);
}

#define recv myRecv
#define accept myAccept
#define socket mySocket
#define bind myBind
#define listen myListen
#define epoll_wait myEpollWait
