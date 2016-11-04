#!/bin/sh
rm -rf parta_gcc_report.txt
rm -rf parta_perl_report.txt
rm -rf partb_gcc_report.txt
rm -rf partb_perl_report.txt
for wid in 1 2 4 8
do
for iq in 8 16 32 64 128 256
do
./sim 512 $iq $wid val_trace_gcc1.txt >> parta_gcc_report.txt
./sim 512 $iq $wid val_trace_perl1.txt >> parta_perl_report.txt
done
echo " " >> gcc_report_test.txt
echo " " >> perl_report_test.txt
done
./sim 32 8 1 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 64 8 1 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 128 8 1 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 256 8 1 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 512 8 1 val_trace_gcc1.txt >> partb_gcc_report.txt
echo " " >> partb_gcc_report.txt
./sim 32 16 2 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 64 16 2 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 128 16 2 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 256 16 2 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 512 16 2 val_trace_gcc1.txt >> partb_gcc_report.txt
echo " " >> partb_gcc_report.txt
./sim 32 32 4 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 64 32 4 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 128 32 4 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 256 32 4 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 512 32 4 val_trace_gcc1.txt >> partb_gcc_report.txt
echo " " >> partb_gcc_report.txt
./sim 32 128 8 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 64 128 8 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 128 128 8 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 256 128 8 val_trace_gcc1.txt >> partb_gcc_report.txt
./sim 512 128 8 val_trace_gcc1.txt >> partb_gcc_report.txt
##################################################################
./sim 32 8 1 val_trace_perl1.txt >> partb_perl_report.txt
./sim 64 8 1 val_trace_perl1.txt >> partb_perl_report.txt
./sim 128 8 1 val_trace_perl1.txt >> partb_perl_report.txt
./sim 256 8 1 val_trace_perl1.txt >> partb_perl_report.txt
./sim 512 8 1 val_trace_perl1.txt >> partb_perl_report.txt
echo " " >> partb_perl_report.txt
./sim 32 32 2 val_trace_perl1.txt >> partb_perl_report.txt
./sim 64 32 2 val_trace_perl1.txt >> partb_perl_report.txt
./sim 128 32 2 val_trace_perl1.txt >> partb_perl_report.txt
./sim 256 32 2 val_trace_perl1.txt >> partb_perl_report.txt
./sim 512 32 2 val_trace_perl1.txt >> partb_perl_report.txt
echo " " >> partb_perl_report.txt
./sim 32 64 4 val_trace_perl1.txt >> partb_perl_report.txt
./sim 64 64 4 val_trace_perl1.txt >> partb_perl_report.txt
./sim 128 64 4 val_trace_perl1.txt >> partb_perl_report.txt
./sim 256 64 4 val_trace_perl1.txt >> partb_perl_report.txt
./sim 512 64 4 val_trace_perl1.txt >> partb_perl_report.txt
echo " " >> partb_perl_report.txt
./sim 32 128 8 val_trace_perl1.txt >> partb_perl_report.txt
./sim 64 128 8 val_trace_perl1.txt >> partb_perl_report.txt
./sim 128 128 8 val_trace_perl1.txt >> partb_perl_report.txt
./sim 256 128 8 val_trace_perl1.txt >> partb_perl_report.txt
./sim 512 128 8 val_trace_perl1.txt >> partb_perl_report.txt
