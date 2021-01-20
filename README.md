
# :diamond_shape_with_a_dot_inside: vmcrec
**Record &amp; replay VirtualMotionCapture motions with lightweight data format**

## Dependencies

You need [VirtualMotionCapture](https://sh-akira.github.io/VirtualMotionCapture/) with OSC motion sender enabled. More specifically you'll need [paid version of VirtualMotionCapture](https://akira.fanbox.cc/).

## Generating Dump

- Run `vmcrec.exe -o [output file name]` (on Windows). It listens [VMC protocol](https://sh-akira.github.io/VirtualMotionCaptureProtocol/specification) on port `39539`.
- Launch [VirtualMotionCapture](https://sh-akira.github.io/VirtualMotionCapture/). Check `Enable OSC motion sender` from `Settings`.  Note that you'll need [paid version of VirtualMotionCapture](https://akira.fanbox.cc/) that supports OSC motion sender.
- `vmcrec.exe` starts recording motions and generate dump file.
- In order to stop recording, *type Ctrl-C from your keyboard* on `vmcrec.exe` console. This will generate resulting output file.

## Replay

- Run `vmcrec.exe -i [input file name] --replay` (on Windows). It parses input file and re-send UDP packet using [VMC protocol](https://sh-akira.github.io/VirtualMotionCaptureProtocol/specification) to port `39539`. **WIP**

## Options

- `-p <port number>` ... port number to listen VMC data. `39539` by default.
- `-o <output file name>` ... Output file name to generate.
- `--replay`  ... Start replaying dump file. Must use with `-i` option.
- `-i <input file name>`  ... Input file name to replay.

## Building

You need [Cmake](https://cmake.org/download/) and Visual Studio with C++ environment installed. You don't need Unity nor UniVRM to build vmcrec. There is a CMakeLists.txt file which has been tested with [Cmake](https://cmake.org/download/) on Windows. For instance in order to generate a Visual Studio 10 project, run cmake like this:


```
> mkdir build; cd build
> cmake -G "Visual Studio 10" ..
```

