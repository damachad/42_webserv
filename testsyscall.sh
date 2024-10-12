#!/bin/zsh

sys_env_list=(RECV_FAIL ACCEPT_FAIL SOCKET_FAIL BIND_FAIL LISTEN_FAIL EPOLL_WAIT_FAIL)

# Function to unset environment variables in the list
clean_sys_env_list() {
    for env_var in $sys_env_list; do
        unset $env_var
    done
    unset "SYS_CALL"
}

# Function to set environment variables for each function and number said function appears
test_fail() {
	local env_var=$1
	local number=$2

	export "$env_var"=1
	export SYS_CALL=$number
}

clean_sys_env_list

