/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

 #include "interrupts.hpp"

 int main(int argc, char** argv) {
 
     //vectors is a C++ std::vector of strings that contain the address of the ISR
     //delays  is a C++ std::vector of ints that contain the delays of each device
     //the index of these elemens is the device number, starting from 0
     auto [vectors, delays] = parse_args(argc, argv);
     std::ifstream input_file(argv[1]);
 
     std::string trace;      //!< string to store single line of trace file
     std::string execution;  //!< string to accumulate the execution output
 
         // --- simulation state ---
    int current_time = 0;                 // global clock (ms)

    // timing constants from spec (you can vary later in report)
    const int KERNEL_SWITCH_MS = 1;
    int CONTEXT_SAVE_MS = 10;             
    const int VECTOR_LOOKUP_MS = 1;
    const int LOAD_ISR_ADDR_MS = 1;       
    const int DRIVER_ACTIVITY_MS = 40;    
    const int IRET_MS = 1;

    
    std::vector<int> io_finish(delays.size(), -1);

    auto log = [&](int start, int dur, const std::string& what){
        execution += std::to_string(start) + ", " + std::to_string(dur) + ", " + what + "\n";
    };

    auto run_interrupt = [&](int intr_num, int activities){
        auto [pre, after_time] = intr_boilerplate(current_time, intr_num, CONTEXT_SAVE_MS, vectors);
        execution += pre;
        current_time = after_time;

        for(int i=0;i<activities;i++){
            log(current_time, DRIVER_ACTIVITY_MS, "call device driver (activity " + std::to_string(i+1) + ")");
            current_time += DRIVER_ACTIVITY_MS;
        }
        log(current_time, IRET_MS, "IRET");
        current_time += IRET_MS;
    };

    auto next_completion_time = [&](){
        int t = INT_MAX;
        for(int ft : io_finish) if(ft >= 0) t = std::min(t, ft);
        return t;
    };

    auto device_at_time = [&](int t){
        for(size_t d=0; d<io_finish.size(); ++d) if(io_finish[d]==t) return (int)d;
        return -1;
    };

 
     while(std::getline(input_file, trace)) {
         auto [activity, duration_intr] = parse_trace(trace);
 
        std::string kind = activity;
        if(!kind.empty() && kind.back()==' ') kind.pop_back();

        if (kind == "CPU") {
            int cpu = duration_intr;
            while (cpu > 0) {
                int tDone = next_completion_time();
                if (current_time + cpu <= tDone) {
                    log(current_time, cpu, "CPU burst");
                    current_time += cpu;
                    cpu = 0;
                } else {
                    int slice = tDone - current_time;
                    if (slice > 0) {
                        log(current_time, slice, "CPU burst");
                        current_time += slice;
                        cpu -= slice;
                    }
                    int dev = device_at_time(tDone);
                    if (dev >= 0) {
                        int delay_ms = delays.at(dev);
                        log(current_time, delay_ms, "end of I/O " + std::to_string(dev) + ": interrupt");
                        io_finish[dev] = -1;
                        run_interrupt(dev, /*activities=*/1);
                    } else {
                    }
                }
            }
        }
        else if (kind == "SYSCALL") {
            int dev = duration_intr;

            run_interrupt(dev, /*activities=*/3);

            int delay_ms = delays.at(dev);
            io_finish[dev] = current_time + delay_ms;
        }
        else if (kind == "END_IO") {
            
        }
        else {
            // ignore unknown lines gracefully
        }

 
     }
 
     input_file.close();
 
     write_output(execution);
 
     return 0;
 }

 //bash build.sh
 //./bin/interrupts trace.txt vector_table.txt device_table.txt
 //head -20 execution.txt
