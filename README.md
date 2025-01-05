# ⚠️⚠️ Legal Disclaimer ⚠️⚠️
This project was created in the context of a university project and only serves educational purposes!


Any illegal usage is strictly forbidden and shall be punished with bad Karma!


The authors take no responsibility for any damage done to your system or the systems of others, as well as any other consequences you might face!
# Setup
We need to setup stock server to trigger the attack
```
python3 stock_value_server.py
```
Also we need to modify the address in [stock_client.c](https://github.com/upc-malware-project/malware/blob/main/src/main/modules/stock_client.c#L18).

# Compiling and running
needs python 3.13
```
pip3 install regex
sudo apt install -y gcc-11 libc6-dev
python3 ./pack.py
```

To run the malware, just execute the binaryfile in the `bin/` directory
```
bin/microworm
```

## Target Machines
The CUPS exploit was successfully tested on machines running Ubuntu 24.04 LTS.


For the privilege escalation, it was working on Ubuntu 18.04.3.


The kernel rootkit requires 5.4 and did not work in VirtualBox but works on QEMU under Fedora 41.




