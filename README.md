# RAMPA simmulator

This program is used to evaluate the proposed method called Resource-Aware Model Partitioning and Allocation (RAMPA) in "Resource-Aware Deep Learning Model Partitioning and Allocation for Inference Tasks in Clusters with Heterogeneous Graphics Processing Units." RAMPA aims to minimize the allocation of critical resources during future deep learning (DL) service executions, ensuring that these executions are not hindered. By doing so, RAMPA enables the system to run more DL services while still meeting performance requirements. This simulator models the allocation of DL services within a GPU cluster, allowing for the evaluation of the number of services that meet performance requirements, as well as the performance metrics such as throughput and execution time.


## Requirements

- Visual Studio (version 2019 or later)
- Boost library (version 1.77 or later)
- OpenMP support enabled in Visual Studio

## Setup Instructions

1. **Open the Solution:**
   - Open `RAMPA_sim.sln` in Visual Studio.

2. **Configure Boost Library:**
   - In Visual Studio, right-click on `ConsoleApplication1` in the Solution Explorer and select **Properties**.
   - Navigate to **Configuration Properties** > **C/C++** > **General**.
   - In the **Additional Include Directories** field, add the path to the `boost-1_77` directory.

3. **Enable OpenMP Support:**
   - Still in the **Properties** window, go to **Configuration Properties** > **C/C++** > **Language**.
   - Set **OpenMP Support** to **Yes**.

4. **Build and Run:**
   - After making the necessary configuration changes, build and run the project in Release.



## Usage Instructions

1. **Compile the Project:**
   - Open the project in Visual Studio and build the solution to generate the `.exe` file.

2. **Place Required Files:**
   - After compiling the project, place the following files in the same folder as the generated `.exe` file:
     - **GPU Cluster Configuration File:** `graph.txt`
     - **Model Partitioning and Allocation Method Configuration File:** `param.txt`
     - **Service Execution Request Sequence Configuration File:** `allocSeq.txt`
     - **Folder containing DL Models:** `MODELS` (which contains the model configuration files)

3. **Running the Simulation:**
   - Execute the `.exe` file. The simulation will begin, and the following output files will be generated:
     - **`utility_teian_`:** This file contains the following information for each time slot:
       - Assigned DL model number
       - Rejection rate (number of rejected tasks / number of allocated tasks)
       - Link utilization rate (number of allocated links / total links in the cluster)
       - GPU utilization rate (number of allocated GPUs / total GPUs)
       - Number of services affected by changes in communication delay due to allocation
       - Generation count when the solution converges
       - Resource allocation cost
       - Allocation time
       - Execution time
       - Throughput
       - Model partitioning details
     - **`link`:** This file lists services using GPU cluster links for communication in each time slot and their traffic.
     - **`residual`:** This file describes the GPU allocation status for each time slot.
     - **`lightpath`:** This file describes the lightpath routes connecting GPUs in the cluster for each time slot.

4. **Folder Structure for Input Files:**
   - The following folders contain the necessary input data for the simulation:
     - `graphs`: Contains the cluster configuration data.
     - `allocs`: Contains the service request sequence.
     - `params`: Contains the method parameters.
     - `MODELS`: Contains the DL models used for the simulation.

5. **Automatic Evaluation for All Cases:**
   - To run the evaluation for all cases automatically, place the input files and folders (`graphs`, `allocs`, `params`, `MODELS`) in the same folder as the `.exe` file.
   - Run the `paramMaker.py` script to generate the necessary parameter files in the `params` folder.
   - Then, run `allProc.bat` to trigger the evaluation for all cases.

## Evaluation Results

For the experimental results in this paper, please refer to the `result` folder.

