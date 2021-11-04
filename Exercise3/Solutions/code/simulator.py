import sys
import copy

class Process:
    def __init__(self, arrival_time, computaion_time):
        self.arrival_time = arrival_time
        self.computaion_time = computaion_time
        # Turnaround time is the total amount of time spent by the process from coming in the ready state for the first time to its completion.
        self.start_time = 0
        self.end_time = 0
        self.turn_around_time = 0
        self.waiting_time = 0
        self.remaining_time = 0

path = str(sys.argv[1])
file = open(path, 'r')

# Using readlines()
Lines = file.readlines()
process_list = []
count = 0

for line in Lines:
    if count == 0:
        num_of_processes = int(line.strip(' \n')) # Strips newline character and whitespaces from the left and right of the string
        count += 1
        continue
    line_cpy = line.strip(' \n')
    line_split = line_cpy.split(",") #split the string containing a comma

    if int(line_split[1]) != 0:
        process_list.append(Process(arrival_time=int(line_split[0]), computaion_time=int(line_split[1])))

    count += 1
    if count == int(num_of_processes) + 1: #finished iterate of all the processes
        break

file.close()

process_list_cpy1 = copy.deepcopy(process_list)
process_list_cpy2 = copy.deepcopy(process_list)
process_list_cpy3 = copy.deepcopy(process_list)
process_list_cpy4 = copy.deepcopy(process_list)
process_list_cpy5 = copy.deepcopy(process_list)

def FCFS(process_list,num_of_processes):
    process_list = sorted(process_list, key=lambda x: x.arrival_time, reverse=False) #sort the process_list by arrival time
    current_time = 0
    turnaround_time = 0
    for id, process in enumerate(process_list):
        if id == 0:
            process.start_time = process.arrival_time
            current_time = process.start_time + process.computaion_time
            process.end_time = process.start_time + process.computaion_time
            # process.turn_around_time = process.computaion_time

        else:
            # IN case there is a job a and job b, and there  is a break between job a to job b
            if process.arrival_time > current_time:
                current_time = process.arrival_time

            process.start_time = max(current_time, process.arrival_time)
            current_time += process.computaion_time
            process.end_time = process.start_time + process.computaion_time

        turnaround_time = turnaround_time  + process.end_time - process.arrival_time

    avg_turnaround_time = turnaround_time / num_of_processes

    print('FCFS non-preemptive, average TAT: %f' % avg_turnaround_time)

def LCFS_non_preemptive(process_list,num_of_processes):
    # sort the processes by their arrival time
    process_list = sorted(process_list, key=lambda x: x.arrival_time, reverse=False)
    process_list_stack = []  # LCFS
    total_turnaround_time = 0
    current_time = 0

    for id, process in enumerate(process_list):
        if id == 0:
            process.start_time = process.arrival_time
            process.end_time = current_time = process.start_time + process.computaion_time  # non preemptive
            process.turn_around_time = process.end_time - process.start_time
            total_turnaround_time += process.turn_around_time

        if id != 0:
            if current_time < process.arrival_time and len(process_list_stack) == 0:
                current_time = process.arrival_time + process.computaion_time
                total_turnaround_time += process.computaion_time
                continue
            if current_time >= process.arrival_time:
                process_list_stack.append(process)
                continue
            else:
                while (len(process_list_stack) != 0):
                    current_process = process_list_stack.pop()
                    current_process.waiting_time = current_time - current_process.arrival_time
                    current_process.turn_around_time = current_process.waiting_time + current_process.computaion_time
                    total_turnaround_time += process.turn_around_time
                    current_time += current_process.computaion_time
                    if current_time >= process.arrival_time:
                        process_list_stack.append(process)
                        break
                continue


    while (len(process_list_stack) != 0):
        current_process = process_list_stack.pop()
        current_process.waiting_time = current_time - current_process.arrival_time
        current_process.turn_around_time = current_process.waiting_time + current_process.computaion_time
        total_turnaround_time += current_process.turn_around_time
        current_time += current_process.computaion_time

    avg_turnaround_time = total_turnaround_time / num_of_processes

    print('LCFS non-preemptive, average TAT: %f' % avg_turnaround_time)


def LCFS_preemptive(process_list,num_of_processes):
    # sort the processes by their arrival time
    process_list = sorted(process_list, key=lambda x: x.arrival_time, reverse=False)
    current_time = 0
    total_turnaround_time = 0
    process_list_stack = []  # LCFS_preemptive

    for counter, process in enumerate(process_list):
        # last element in the list
        if counter + 1 == len(process_list):
            current_time = process.arrival_time + process.computaion_time
            process.turn_around_time = process.computaion_time
            total_turnaround_time += process.turn_around_time
            break

        if process_list[counter + 1].arrival_time - process.arrival_time >= process.computaion_time:
            current_time = process.arrival_time + process.computaion_time
            process.turn_around_time = process.computaion_time
            total_turnaround_time += process.turn_around_time
        else:
            current_time = process_list[counter + 1].arrival_time  # preemptive !
            process.turn_around_time += process_list[counter + 1].arrival_time - process.arrival_time
            process.computaion_time -= process.turn_around_time
            process_list_stack.append(process)

    while (len(process_list_stack) != 0):
        current_process = process_list_stack.pop()
        current_process.waiting_time = current_time - current_process.arrival_time - current_process.turn_around_time
        current_process.turn_around_time += current_process.waiting_time + current_process.computaion_time
        total_turnaround_time += current_process.turn_around_time
        current_time += current_process.computaion_time

    avg_turnaround_time = total_turnaround_time / num_of_processes

    print('LCFS preemptive, average TAT: %f' % avg_turnaround_time)

def SJF_preemptive(process_list,num_of_processes):
    # sort the processes by their arrival time
    process_list = sorted(process_list, key=lambda x: x.arrival_time, reverse=False)
    total_turnaround_time = 0
    complete = False
    time = 0

    while (complete == False):
        process_list_helper = [] #SJF_preemptive
        flag = True
        for counter, process in enumerate(process_list):
            if time >= process.arrival_time and process.computaion_time > 0:
                process_list_helper.append(process)
            if (process.computaion_time > 0):  # still exist processes who needs the CPU
                flag = False
        if flag == True:
            break

        if len(process_list_helper) != 0:
            process_list_helper = sorted(process_list_helper, key=lambda x: x.computaion_time, reverse=False)
            process_list_helper[0].computaion_time -= 1
            total_turnaround_time += len(process_list_helper)
            # for process in process_list_helper:
            #     process.turn_around_time += 1
        time += 1

    avg_turnaround_time = total_turnaround_time / num_of_processes

    print('SJF preemptive, average TAT: %f' % avg_turnaround_time)

def RR(process_list, num_of_processes):
    # sort the processes by their arrival time
    process_list = sorted(process_list, key=lambda x: x.arrival_time, reverse=False)
    time_quantum = 2
    time = 0
    total_turnaround_time = 0
    complete = False
    process_queue = []

    while (complete == False):
        flag = True
        for process in process_list:
            if time == process.arrival_time and process.computaion_time > 0:
                process_queue.append(process)
            if (process.computaion_time > 0):  # still exist processes who needs the CPU
                flag = False
        if flag == True:
            break

        if len(process_queue) != 0:
            current_process = process_queue.pop(0)
            if current_process.computaion_time > time_quantum:
                current_process.computaion_time -= time_quantum
                total_turnaround_time += len(process_queue) * time_quantum + time_quantum
                # check if in the interval time of the time_quantum new processes arrived
                for i in range(time+1,time+time_quantum+1,1): #time --> time+time_quantum
                    for process in process_list:
                        if i == process.arrival_time and process.computaion_time > 0:
                            process_queue.append(process)
                            total_turnaround_time += time + time_quantum - i
                process_queue.append(current_process)
                time += time_quantum + 1

            else:
                cpu_time = current_process.computaion_time
                current_process.computaion_time = 0
                total_turnaround_time += len(process_queue) * cpu_time + cpu_time

                # check if in the interval time of the cpu_time new processes arrived
                for i in range(time + 1, time + cpu_time + 1, 1):  # time --> time+time_quantum
                    for process in process_list:
                        if i == process.arrival_time and process.computaion_time > 0:
                            process_queue.append(process)
                            total_turnaround_time += time + cpu_time - i
                time += cpu_time + 1
        else:
            time += 1

    avg_turnaround_time = total_turnaround_time / num_of_processes

    print('RR, average TAT: %f' % avg_turnaround_time)

FCFS(process_list_cpy1,num_of_processes)
LCFS_non_preemptive(process_list_cpy2,num_of_processes)
LCFS_preemptive(process_list_cpy3,num_of_processes)
RR(process_list_cpy4,num_of_processes)
SJF_preemptive(process_list_cpy5,num_of_processes)

