#!/bin/bash

######### Cool things #########
	#colors
	reset="[0m"
	red="[1m[31m"
	green="[1m[32m"
	yellow="[1m[33m"
	blue="[1m[34m"
    default_color="$yellow"

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
		info "Usage:\n\t$SAC_USAGE" $yellow
		if [ ! -z "$SAC_OPTIONS" ]; then
		info "Options:\n\t$SAC_OPTIONS" $yellow
		fi
		if [ ! -z "$$SAC_EXAMPLE" ]; then
			info "$SAC_EXAMPLE:\n\t$$SAC_EXAMPLE" $yellow
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

	#ensure that a package ($1, first arg) is installed (optionnaly in $2)
	check_package() {
		msg="Please ensure you have installed AND added '$1' to your PATH variable"
		if [ $# = 2 ]; then
			msg+="(PATH should be '$2')"
		fi

		type $1 >/dev/null 2>&1 || { info "$msg" $red; exit 3; }
	}
