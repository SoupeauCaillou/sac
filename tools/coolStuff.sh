#!/bin/bash

######### Cool things #########
	#colors
	reset="[0m"
	red="[1m[31m"
	green="[1m[32m"
	yellow="[1m[33m"
	blue="[1m[34m"

	#show logs in colors
	info() {
		if [ $# = 1 ]; then
			echo -e "${green}$1${reset}"
		else
			echo -e "$2$1${reset}"
		fi
	}

######### FUNCTIONS #########
	#show how to use the script
	usage_and_quit() {
		info "Usage:\n\t$USAGE" $yellow
		if [ ! -z "$OPTIONS" ]; then
		info "Options:\n\t$OPTIONS" $yellow
		fi
		if [ ! -z "$EXAMPLE" ]; then
			info "Example:\n\t$EXAMPLE" $yellow
		fi
		info "Bybye everybody!"
		exit 1
	}

	#exiting after throwing an error
	error_and_usage_and_quit() {
		info "######## $1 ########" $red
		usage_and_quit
	}

	#exiting after throwing an error
	error_and_quit() {
		info "######## $1 ########" $red
		exit 2
	}

	#ensure that a package is installed
	check_package() {
		if [ ! -z "`type $1 | grep 'not found'`" ]; then
			info "Please ensure you have added $2 to your PATH variable ($1 program missing)" $red
			exit 3
		fi
	}
