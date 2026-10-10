# Vagrantfile
Vagrant.configure("2") do |config|
  config.vm.box = "debian/bookworm64"
  config.vm.hostname = "kernel-dev"

  # Project folder is shared at /vagrant, so mykernel.iso appears on your host
  config.vm.synced_folder ".", "/vagrant"

  config.vm.provider "virtualbox" do |vb|
    vb.name   = "kernel-dev"
    vb.memory = 2048
    vb.cpus   = 2
  end

  config.vm.provision "shell", inline: <<-SHELL
    set -e
    export DEBIAN_FRONTEND=noninteractive
    apt-get update
    apt-get install -y --no-install-recommends \
      build-essential gcc-multilib \
      nasm \
      xorriso mtools \
      make
    #   grub-pc-bin grub-common \
    echo "Toolchain ready."
  SHELL
end