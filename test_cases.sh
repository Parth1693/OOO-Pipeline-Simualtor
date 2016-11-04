echo "Hello World"
make clean
make

./sim 16 8 1 val_trace_gcc1.txt > test_val1.txt
./sim 16 8 2 val_trace_gcc1.txt > test_val2.txt
./sim 60 15 3 val_trace_gcc1.txt > test_val3.txt
./sim 64 16 8 val_trace_gcc1.txt > test_val4.txt
./sim 64 16 4 val_trace_perl1.txt > test_val5.txt
./sim 128 16 5 val_trace_perl1.txt > test_val6.txt
./sim 256 64 5 val_trace_perl1.txt > test_val7.txt
./sim 512 64 7 val_trace_perl1.txt > test_val8.txt
diff -iw test_val1.txt validations/val1.txt
diff -iw test_val2.txt validations/val2.txt
diff -iw test_val3.txt validations/val3.txt
diff -iw test_val4.txt validations/val4.txt
diff -iw test_val5.txt validations/val5.txt
diff -iw test_val6.txt validations/val6.txt
diff -iw test_val7.txt validations/val7.txt
diff -iw test_val8.txt validations/val8.txt
