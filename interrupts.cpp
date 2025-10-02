/**
 *
 * @file interrupts.cpp
 * @author <Your Name>
 *
 */

 #include "interrupts.hpp"

 int main(int argc, char** argv) {
     // vectors = ISR addresses, delays = device delays
     auto [vectors, delays] = parse_args(argc, argv);
     std::ifstream input_file(argv[1]);
 
     std::string trace;      //!< string to store single line of trace file
     std::string execution;  //!< string to accumulate the execution output
     int current_time = 0;   //!< simulation clock
 
     // constants
     const int KERNEL_SWITCH_MS = 1;
     int CONTEXT_SAVE_MS = 10;     //vary 10/20/30
     const int VECTOR_LOOKUP_MS = 1;
     const int LOAD_ISR_ADDR_MS = 1;
     const int DRIVER_ACTIVITY_MS = 40; //vary 40..200
     const int IRET_MS = 1;
 
     // helper: append one line
     auto log = [&](int dur, const std::string& what) {
         execution += std::to_string(current_time) + ", "
                    + std::to_string(dur) + ", "
                    + what + "\n";
         current_time += dur;
     };
 
     // helper: run an interrupt (boilerplate + driver activities + IRET)
     auto run_interrupt = [&](int dev, int activities) {
         auto [pre, after] = intr_boilerplate(current_time, dev, CONTEXT_SAVE_MS, vectors);
         execution += pre;
         current_time = after;
 
         for (int i = 0; i < activities; i++) {
             log(DRIVER_ACTIVITY_MS, "call device driver (activity " + std::to_string(i+1) + ")");
         }
 
         log(IRET_MS, "IRET");
     };
 
     //parse each line of the input trace file
     while (std::getline(input_file, trace)) {
         auto [activity, val] = parse_trace(trace);
 
         if (activity == "CPU") {
             log(val, "CPU burst");
         }
         else if (activity == "SYSCALL") {
             int dev = val;
             // system call interrupt (use 3 driver activities)
             run_interrupt(dev, 3);
             // after ISR, we assume device I/O is now in progress
         }
         else if (activity == "END_IO") {
             int dev = val;
             // when I/O finishes, log the interrupt + ISR
             log(delays[dev], "end of I/O " + std::to_string(dev) + ": interrupt");
             run_interrupt(dev, 1); // typically 1 activity for completion
         }
         else {
         }
     }
 
     input_file.close();
     write_output(execution);
     return 0;
 }
 //command kine to run the code 
 //bash build.sh
 //./bin/interrupts trace.txt vector_table.txt device_table.txt
 //head -20 execution.txt
