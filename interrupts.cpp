/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * 
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
    const int context_save_time = 10;   // 10, 20, 30
    const int isr_activity_time = 40;   // 40-200

    /******************************************************************/

    // helper to append one line and advance time
    auto log = [&](int dur, const std::string& what) {
        execution += std::to_string(current_time) + ", " +
                     std::to_string(dur) + ", " + what + "\n";
        current_time += dur;
    };

    //parse each line of the input trace file
    while (std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        if (activity == "CPU") {
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU\n";
            current_time += duration_intr;
        }

        else if (activity == "SYSCALL") {
            auto [partial, time_after] = intr_boilerplate(current_time, duration_intr, context_save_time, vectors);
            execution += partial;
            current_time = time_after;

            // run the ISR
            int delay_time = delays.at(duration_intr);
            int part1 = delay_time / 2;
            int part2 = delay_time - part1;

            execution += std::to_string(current_time) + ", " + std::to_string(part1) + ", run device driver (part 1)\n";
            current_time += part1;

            execution += std::to_string(current_time) + ", " + std::to_string(part2) + ", run device driver (part 2)\n";
            current_time += part2;

            // return
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;

            // switching back to user mode
            execution += std::to_string(current_time) + ", 1, switch to user mode\n";
            current_time += 1;
        }
        else if (activity == "END_IO") {
            execution += std::to_string(current_time) + ", 1, switch to kernal mode\n";
            current_time += 1;

            execution += std::to_string(current_time) + ", " + std::to_string(context_save_time) + ", context save\n";
            current_time += context_save_time;

            execution += std::to_string(current_time) + ", 1, find vector " + std::to_string(duration_intr) + "\n";
            current_time += 1;

            execution += std::to_string(current_time) + ", 1, load address " + vectors.at(duration_intr) + " into PC\n";
            current_time += 1;

            // ISR body
            execution += std::to_string(current_time) + ", 40, ENDIO: run the ISR\n";
            current_time += 40;

            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;

            execution += std::to_string(current_time) + ", 1, switch to user mode\n";
            current_time += 1;
        }
        else {
            std::cerr << "Unknown activity: " << activity << std::endl;
        }
    }

    /************************************************************************/

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
