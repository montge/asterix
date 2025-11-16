LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH valgrind -v --leak-check=full --show-leak-kinds=all --error-exitcode=1 ../bin/asterix -P -d ../share/asterix/config/asterix.ini -f ../share/asterix/samples/cat_062_065.pcap -jh

if [ $? = 1 ]; then
        echo "Memory leak test failed"
	exit 1
else
        echo "Memory leak test OK"
	exit 0
fi
