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
   - After making the necessary configuration changes, build and run the project.



