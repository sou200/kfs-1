#!/bin/sh
curl https://releases.hashicorp.com/vagrant/2.4.9/vagrant_2.4.9_linux_amd64.zip -o ~/goinfre/vagrant.zip
unzip ~/goinfre/vagrant.zip -d ~/goinfre
mkdir -p /home/$USER/goinfre/vagrant_home
mkdir -p /home/$USER/goinfre/VMS

export VAGRANT_HOME="/home/$USER/goinfre/vagrant_home"
export PATH="$PATH:/home/$USER/goinfre/"
vboxmanage setproperty machinefolder /home/$USER/goinfre/VMS
