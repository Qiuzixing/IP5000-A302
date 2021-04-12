# How to download SDK from git

## Pre-requirement
- Linux development environment (Ubuntu 32bits)
- sudo apt-get install git

## SSH key installation

Use following shell commands:
```
	# Create ~/.ssh folder if not exists
	mkdir -p ~/.ssh

	# Copy SSH key `id_rsa_aspeed` to `~/.ssh/`.
	cp ./id_rsa_aspeed ~/.ssh/
	chmod 600 ~/.ssh/id_rsa_aspeed
	eval "$(ssh-agent -s)"

	# Add SSH key
	ssh-add ~/.ssh/id_rsa_aspeed

	# Add know_hosts
	ssh-keyscan gitlab.com >> ~/.ssh/known_hosts
```

## Get SDK source code for the first time

Use following shell commands:
```
	git clone git@gitlab.com:aspeed/pc_ext/AST15xx/SDKv3_customer.git
```

## Update SDK source code

Use following shell commands:
```
	git pull git@gitlab.com:aspeed/pc_ext/AST15xx/SDKv3_customer.git
```
