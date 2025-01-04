# MATLAB API for mTMS software

## Using API

### Getting started

Build the ROS 2 messages for MATLAB by running:

```
build-mtms-matlab
```

After finished, open MATLAB and add the `api/matlab` directory to the MATLAB path.

Test the API by running:

```
api = MTMSApi();
api.start_device();
```

See `examples/example.m` for examples on how to use the API.
