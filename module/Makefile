obj-m += dvt-driver.o

all:
	make -C /lib/modules/5.4.0-42-generic/build M=$(PWD) modules
clean:
	make -C /lib/modules/5.4.0-42-generic/build M=$(PWD) clean
