#ifndef WRAPPERFUNCTIONS_HPP
#define WRAPPERFUNCTIONS_HPP

#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define recv myRecv
#define accept myAccept
#define socket mySocket
#define bind myBind
#define listen myListen
#define epoll_wait myEpollWait

bool checkFailure(const char *fail_env, int call_counter);
ssize_t myRecv(int sockfd, void *buf, size_t len, int flags);
int myAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int mySocket(int domain, int type, int protocol);
int myBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int myListen(int sockfd, int backlog);
int myEpollWait(int epfd, struct epoll_event *events, int maxevents,
				int timeout);

#endif
