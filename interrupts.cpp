/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * 
 * 
 *
 */

#include<interrupts.hpp>

static unsigned int next_pid = 1;
std::tuple<std::string, std::string, int> simulate_trace(std::vector<std::string> trace_file, int time, std::vector<std::string> vectors, std::vector<int> delays, std::vector<external_file> external_files, PCB current, std::vector<PCB> wait_queue) {

    std::string trace;      //!< string to store single line of trace file
    std::string execution = "";  //!< string to accumulate the execution output
    std::string system_status = "";  //!< string to accumulate the system status output
    int simulation_time = 0; // this is the total duration of the sim
    const int context_time = 10; // context
    const int isr_time = 200; // isr
    int current_time = time;
    

    //parse each line of the input trace file. 'for' loop to keep track of indices.
    for(size_t i = 0; i < trace_file.size(); i++) {
        auto trace = trace_file[i];

        auto [activity, duration_intr, program_name] = parse_trace(trace);

        if (activity == "CPU") {
            execution += std::to_string(simulation_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
            simulation_time += duration_intr;
        }

        else if (activity == "SYSCALL") {
            auto [execution_temp, simulation_time_temp] = intr_boilerplate(simulation_time, duration_intr, context_time, vectors);
            execution += execution_temp;
            simulation_time = simulation_time_temp;

            execution += std::to_string(simulation_time) + ", " + std::to_string(delays[duration_intr]) + ", " + "SYSCALL: run the ISR for device " + std::to_string(duration_intr) + "\n";
            simulation_time += delays[duration_intr];

            execution += std::to_string(simulation_time) + ", " + std::to_string(isr_time) + ", " + "transfer data from device to memory\n";
            simulation_time += isr_time;

            execution += std::to_string(simulation_time) + ", " + std::to_string(isr_time) + ", " + "check for errors\n";
            simulation_time += isr_time;

            execution += std::to_string(simulation_time) + ", " + std::to_string(1) + ", " + "IRET\n";
            simulation_time++;

        }

        else if (activity == "END_IO") {
            auto [execution_temp, simulation_time_temp] = intr_boilerplate(simulation_time, duration_intr, context_time, vectors);
            execution += execution_temp;
            simulation_time = simulation_time_temp;

            execution += std::to_string(simulation_time) + ", " + std::to_string(delays[duration_intr]) + ", " + "END I/O\n";
            simulation_time += delays[duration_intr];

            execution += std::to_string(simulation_time) + ", " + std::to_string(1) + ", " + "IRET\n";
            simulation_time++;
        }

        else if (activity == "UNKOWN_ACTIVITY") {
            execution += std::to_string(simulation_time) + ", " + std::to_string(1) + ", " + "Unknown Activity\n";
            simulation_time++;
        } else if(activity == "FORK") {
            auto [intr, time] = intr_boilerplate(current_time, 2, 10, vectors);
            execution += intr;
            current_time = time;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your FORK output here

            // Clone the PCB for the child process
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", cloning the PCB\n";
            current_time += duration_intr;
            
            // Create child PCB (clone from current, assign new PID)
            PCB child(next_pid++, current.PID, current.program_name, current.size, -1);
            
            // Allocate memory for child
            if(!allocate_memory(&child)) {
                std::cerr << "ERROR! Memory allocation failed for child process!" << std::endl;
            }
            
            // Add parent to wait queue
            wait_queue.push_back(current);
            
            // Scheduler called
            execution += std::to_string(current_time) + ", " + std::to_string(0) + ", scheduler called\n";


            ///////////////////////////////////////////////////////////////////////////////////////////

            //The following loop helps you do 2 things:
            // * Collect the trace of the chile (and only the child, skip parent)
            // * Get the index of where the parent is supposed to start executing from
            std::vector<std::string> child_trace;
            bool skip = true;
            bool exec_flag = false;
            int parent_index = 0;

            for(size_t j = i; j < trace_file.size(); j++) {
                auto [_activity, _duration, _pn] = parse_trace(trace_file[j]);
                if(skip && _activity == "IF_CHILD") {
                    skip = false;
                    continue;
                } else if(_activity == "IF_PARENT"){
                    skip = true;
                    parent_index = j;
                    if(exec_flag) {
                        break;
                    }
                } else if(skip && _activity == "ENDIF") {
                    skip = false;
                    continue;
                } else if(!skip && _activity == "EXEC") {
                    skip = true;
                    child_trace.push_back(trace_file[j]);
                    exec_flag = true;
                }

                if(!skip) {
                    child_trace.push_back(trace_file[j]);
                }
            }
            i = parent_index;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //With the child's trace, run the child (HINT: think recursion)



            ///////////////////////////////////////////////////////////////////////////////////////////


        } else if(activity == "EXEC") {
            auto [intr, time] = intr_boilerplate(current_time, 3, 10, vectors);
            current_time = time;
            execution += intr;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your EXEC output here



            ///////////////////////////////////////////////////////////////////////////////////////////


            std::ifstream exec_trace_file(program_name + ".txt");

            std::vector<std::string> exec_traces;
            std::string exec_trace;
            while(std::getline(exec_trace_file, exec_trace)) {
                exec_traces.push_back(exec_trace);
            }

            ///////////////////////////////////////////////////////////////////////////////////////////
            //With the exec's trace (i.e. trace of external program), run the exec (HINT: think recursion)



            ///////////////////////////////////////////////////////////////////////////////////////////

            break; //Why is this important? (answer in report)

        }
    }

    return {execution, system_status, current_time};
}

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    //external_files is a C++ std::vector of the struct 'external_file'. Check the struct in 
    //interrupt.hpp to know more.
    auto [vectors, delays, external_files] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    //Just a sanity check to know what files you have
    print_external_files(external_files);

    //Make initial PCB (notice how partition is not assigned yet)
    PCB current(0, -1, "init", 1, -1);
    //Update memory (partition is assigned here, you must implement this function)
    if(!allocate_memory(&current)) {
        std::cerr << "ERROR! Memory allocation failed!" << std::endl;
    }

    std::vector<PCB> wait_queue;

    /******************ADD YOUR VARIABLES HERE*************************/


    /******************************************************************/

    //Converting the trace file into a vector of strings.
    std::vector<std::string> trace_file;
    std::string trace;
    while(std::getline(input_file, trace)) {
        trace_file.push_back(trace);
    }

    auto [execution, system_status, _] = simulate_trace(   trace_file, 
                                            0, 
                                            vectors, 
                                            delays,
                                            external_files, 
                                            current, 
                                            wait_queue);

    input_file.close();

    write_output(execution, "execution.txt");
    write_output(system_status, "system_status.txt");

    return 0;
}
