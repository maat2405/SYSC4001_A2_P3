if [ ! -d "bin" ]; then
    mkdir bin
else
	rm bin/*
fi
g++ -g -O0 -I . -o bin/interrupts_101302780_101306866 interrupts_101302780_101306866.cpp
./bin/interrupts_101302780_101306866 ./trace.txt vector_table.txt device_table.txt external_files.txt