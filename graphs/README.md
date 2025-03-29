# GPU Cluster Network Configuration

This repository contains configuration files for different GPU cluster environments used in our research paper:
**"Resource-Aware Deep Learning Model Partitioning and Allocation for Inference Tasks in Clusters with Heterogeneous Graphics Processing Units."**

## Cluster Configurations

The following configurations correspond to different GPU cluster environments:

- **graph1**: Base Cluster (Neutral GPU cluster for comparison)
- **graph2**: High-Bandwidth Cluster (GPU cluster with high network bandwidth)
- **graph3**: High-Performance Cluster (GPU cluster with many high-performance GPUs)
- **graph4**: Large Cluster (GPU cluster with a large number of GPUs)

## Parameters

Each configuration file defines the following parameters:

### **Network Parameters**
- **link_sw**: Propagation delay (ms) between switches.
- **link_resource**: Propagation delay (ms) between switches and resources.
- **bw**: Network bandwidth.

### **Circuit Switching Parameters**
Defined as a comma-separated list:
- **Number of switches** in the cluster.
- **Switching processing delay**.
- **Topology information**:
  - Each switch is assigned an ID from `0` to `Number of Switches - 1`.
  - Connections are described as `switchID1-switchID2-linkCount`.
  - Multiple connections are separated by commas.
  
  **Example:** If switch `1` and switch `3` are connected with `4` links, it is described as `1-3-4`.

### **GPU Parameters**
Defined as a comma-separated list:
- **Number of GPU racks**.
- **Number of GPUs per rack**.
- **Transfer processing delay** for GPUs.
- **FLOPS** (Floating-point Operations per Second).
- **Memory size** (GB).
- **Memory bandwidth** (GB/s).
- **Switch ID** to which the GPU rack is connected.



