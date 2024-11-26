
# Setup

```
# for linux users
sudo wget -O /usr/bin/ape https://cosmo.zip/pub/cosmos/bin/ape-$(uname -m).elf
sudo chmod +x /usr/bin/ape
sudo sh -c "echo ':APE:M::MZqFpD::/usr/bin/ape:' >/proc/sys/fs/binfmt_misc/register"
sudo sh -c "echo ':APE-jart:M::jartsr::/usr/bin/ape:' >/proc/sys/fs/binfmt_misc/register"

# WSL
sudo sh -c "echo -1 >/proc/sys/fs/binfmt_misc/WSLInterop"


git clone https://github.com/upc-malware-project/malware.git
cd malware
mkdir cosmocc && cd cosmocc
wget https://cosmo.zip/pub/cosmocc/cosmocc.zip
unzip cosmocc.zip
```

# Compiling and running

```
# linux only
make all
./bin/main.com 
```


## Notes about packer/exploit
TODO: this is just a quick commit to share the data, will update README later, sry -.-
