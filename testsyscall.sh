#!/bin/zsh

sys_env_list=(RECV_FAIL ACCEPT_FAIL SOCKET_FAIL BIND_FAIL LISTEN_FAIL EPOLL_WAIT_FAIL)

# Function to unset environment variables in the list
clean_sys_env_list() {
    for env_var in $sys_env_list; do
        unset $env_var
    done
}

# Function to set environment variables for each function and number said function appears
test_fail() {
	local env_var=$1
	local number=$2

	export $env_var=$number
}

# Tests all of the fails on their 1st attempt!
## Cleans previous sets
clean_sys_env_list
## Sets up each var and tests it sequentially
for env_var in $sys_env_list; do
    echo "testing  $env_var"
    test_fail $env_var 2
    valgrind --leak-check=full --track-fds=yes ./webserv conf/default3.conf
    unset $env_var
done
