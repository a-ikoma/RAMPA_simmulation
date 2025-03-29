# DL Service Deployment Request Sequences

This repository contains the deployment request sequences for DL services used in the evaluation of our study. These sequences are generated using `allocationSequenceMaker.py` based on the generation probability of deployment requests for each service category.

## Evaluation Cases

The generated request sequences are categorized into the following evaluation cases:

- **Case1:** Equal demand for all services.
- **Case2:** High demand for high-throughput services.
- **Case3:** High demand for low-delay services.
- **Case4:** High demand for huge model services.

Each case corresponds to a dedicated folder (`Case1`, `Case2`, `Case3`, `Case4`) containing the respective deployment request sequence files used in our study.

## File Format

Each file contains deployment requests, with one request per line. The format is as follows (comma-separated values):

```
Timestamp, allocate, DL Service ID, Service Lifetime
```

### Field Descriptions

- **Timestamp**: The time at which the request is made.
- **allocate**: Allocation request identifier.
- **DL Service ID**: Identifier of the requested DL service.
- **Service Lifetime**: The duration for which the service remains allocated (only applicable when `tumekomi` is set to `0` in `param.txt`).

### Service Expiration and Resource Release

The service lifetime is counted from the timestamp of allocation. Once the lifetime expires, the allocated GPUs and network resources are released.

## Reference

These deployment request sequences are used in the evaluation described in our paper "".

For further details, refer to `allocationSequenceMaker.py` and `param.txt`.

