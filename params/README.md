# Model Allocation Parameter Settings

This repository contains parameter configuration files for model partitioning and allocation methods used in research evaluation. Specifically:
- `param0.txt`: Parameter settings for RAMPA.
- `paramMaker.py`: Generates parameter configuration files for PE and NCAR, the comparative methods.

## Parameter Configuration Format
The parameter files follow the structure below. For detailed functionality, please refer to the research paper.

| Parameter         | Description |
|------------------|-------------|
| `antSedai`       | Number of iterations in Ant Colony Optimization (ACO). |
| `antNum`         | Number of agents generated for resource exploration and pathfinding in ACO. |
| `feromonGensui`  | Pheromone decrease rate (default: `0.1`). |
| `feromonParam`   | Pheromone weight. |
| `costParam`      | Resource allocation cost weight. |
| `feromonRate`    | Pheromone increase rate (default: `100`). |
| `initialFeromon` | Initial pheromone value. |
| `thread`         | Number of threads used for simulation processing. |
| `allocatePolicy` | ID for model partitioning and allocation methods: `0` for RAMPA, `1` for NCAR, `2` for PE. |
| `models`         | Number of DL services allocated in the simulation. |
| `log`            | Whether to generate a log file (`1` to generate, `0` to disable). |
| `tumekomi`       | Consideration of service release (`0` to consider, `1` to ignore). |
| `stage`          | Number of partitions for each DL model (valid only for PE and NCAR). |

## Usage
To generate parameter files for PE and NCAR, run:
```bash
python paramMaker.py
```
This will create the necessary parameter configuration files.

For further details, please refer to the research paper.

