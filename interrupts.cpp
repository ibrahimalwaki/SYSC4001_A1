/**
 *
 * @file interrupts.cpp
 * @author Ibrahim Alwaki 101235067
 * @author Sahand Maaroof 101311395
 *
 */

 #include "interrupts.hpp"

 int main(int argc, char** argv) {
 
     // vectors: ISR addresses; delays: device delays (index = device number)
     auto [vectors, delays] = parse_args(argc, argv);
     std::ifstream input_file(argv[1]);
 
     std::string trace;      //!< string to store single line of trace file
     std::string execution;  //!< string to accumulate the execution output
 
     /******************ADD YOUR VARIABLES HERE*************************/
 
     int current_time = 0;
     const int context_save_time = 10;   // can vary: 10, 20, 30
     const int isr_activity_time = 40;   // can vary: 40–200
     const int iret_time = 1;
 
     /******************************************************************/
 
     // helper to append one line and advance time
     auto log = [&](int dur, const std::string& what) {
         execution += std::to_string(current_time) + ", " +
                      std::to_string(dur) + ", " + what + "\n";
         current_time += dur;
     };
 
     // parse each line of the input trace file
     while (std::getline(input_file, trace)) {
         auto [activity, duration_intr] = parse_trace(trace);
 
         if (activity == "CPU") {
             execution += std::to_string(current_time) + ", " +
                          std::to_string(duration_intr) + ", CPU burst\n";
             current_time += duration_intr;
         }
 
         else if (activity == "SYSCALL") {
             auto [partial, time_after] = intr_boilerplate(current_time, duration_intr, context_save_time, vectors);
             execution += partial;
             current_time = time_after;
 
             for (int i = 1; i <= 3; i++) {
                 execution += std::to_string(current_time) + ", " +
                              std::to_string(isr_activity_time) + ", call device driver (activity " +
                              std::to_string(i) + ")\n";
                 current_time += isr_activity_time;
             }
 
             // return from interrupt
             execution += std::to_string(current_time) + ", " +
                          std::to_string(iret_time) + ", IRET\n";
             current_time += iret_time;
         }
 
         else if (activity == "END_IO") {
             int dev = duration_intr;
 
             int device_delay = delays.at(dev);
             execution += std::to_string(current_time) + ", " +
                          std::to_string(device_delay) + ", end of I/O " +
                          std::to_string(dev) + ": interrupt\n";
             current_time += device_delay;
 
             // now perform END_IO interrupt 
             execution += std::to_string(current_time) + ", 1, switch to kernel mode\n";
             current_time += 1;
 
             execution += std::to_string(current_time) + ", " +
                          std::to_string(context_save_time) + ", context saved\n";
             current_time += context_save_time;
 
             // compute vector memory position 
             int mem_pos = ADDR_BASE + (dev * VECTOR_SIZE);
             std::stringstream mem_hex;
             mem_hex << "0x" << std::uppercase << std::hex << mem_pos;
 
             execution += std::to_string(current_time) + ", 1, find vector " +
                          std::to_string(dev) + " in memory position " +
                          mem_hex.str() + "\n";
             current_time += 1;
 
             execution += std::to_string(current_time) + ", 1, load address " +
                          vectors.at(dev) + " into the PC\n";
             current_time += 1;
 
             // ISR body 
             execution += std::to_string(current_time) + ", " +
                          std::to_string(isr_activity_time) + ", call device driver (activity 1)\n";
             current_time += isr_activity_time;
 
             // return from interrupt
             execution += std::to_string(current_time) + ", " +
                          std::to_string(iret_time) + ", IRET\n";
             current_time += iret_time;
         }
 
         else {
             std::cerr << "Unknown activity: " << activity << std::endl;
         }
     }
 
     input_file.close();
     write_output(execution);
     return 0;
 }

// running the code (through windows cmd)
// g++ -g -O0 -I . -o bin\interrupts.exe interrupts.cpp
// bin\interrupts.exe trace.txt vector_table.txt device_table.txt

// running on WSL/macOS/Linux
// source build.sh
// ./bin/interrupts trace.txt vector_table.txt device_table.txt
