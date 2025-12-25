This repo is my attempt at implementing an extremely complete and complex system to drive any stepper motor in a multi axis or single axis system, with the advantage of supporting many commercially available motor drivers and sensors, achieving perfect performance, accuracy and precision with any combination of them, while adding features such as quick system identification for FF control, FB control via Nested or single level PID controllers, close loop or open loop operation of stepper motors, high order torque/jerk/acceleration/velocity profiles and motion curves, etc.

The idea is that in the end we have a system that can both work as a standalone motion coordinator system for simple multi axis or single axis system (e.g. simple pick and place robot, single stepper demo, multi stepper synchronization demo, etc...) and also as intermediate layer such that higher level motion planning system via ROS or SLAM or others can control a more kinematically complex system such as a 6DOF robot arm, a parallel robot, a quadruped, or others for example (some of these might be also controllable as a demo in the standalone variant).

For this i want to use Spec-Driven Development. In case you do not know what this is, Spec-Driven Development (SDD) is a modern software development approach, especially for AI-assisted workflows like this, where we create detailed, structured specifications (specs) outlining all requirements, intent, and constraints before writing most of the code, making specs the "source of truth" and guiding AI agents to generate high-quality, maintainable code, rather than relying on chaotic "vibe coding". It emphasizes clear planning, breaking down large tasks, and version-controlling our thinking to ensure better alignment with goals, especially for complex features.

Therfore i'd like to start with defining a clear repository structure. Taken some inspiration from other projects of mine, i suggest a structure like the following. Please ignore the fact that it is all python files, this USD project will likely be a mix of Python, C++ and others depending on the specs defined.

Repo structure:

UltimateStepperDriver/
├── UltimateStepperDriver.py                      # Main entry point.
├── README.md
├── requirements.txt
├── USD.ico
│
├── src/                            # Main package
│   ├── __init__.py                      # Package exports
│   │
│   ├── core/                            # Backend workers, interfacers & coordination
│   │   ├── __init__.py
│   │   ├── execOrchestrator.py          # Execution coordination - Manages multiple motors to coordinate motion among them. E.g. for multi-axis motion, coordinated sweeps, 6DoF motion, etc.
|   |   ├── motorController.py           # Motor controller core logic. Receives the desired motion from the orchestrator and generates the needed torque/speed/position curves, as well as hosting control loops such as nested PID or Feedforward control.  
│   │   ├── motorDriver.py               # Motor driver interface. Receives the torque/speed/position curves from the controller and interfaces with the motor driver hardware
│   │   ├── dataSaver.py                 # Save data if needed. Records logs and motion data if needed.
│   │   ├── sysOverseer.py               # System safety overseer. Monitors system status and triggers safety protocols if needed, e.g. overheating, overcurrent, bad movement, disconnection, data errors, system hang, freezing, wathcdog violations, heartbeat misses, etc.
│   │   └── commsWorker.py               # Server communication. Handles all communications with external devices, e.g. GUI, remote clients, etc.
│   │
│   ├── devices/                         # Device abstractions & clients
│   │   ├── __init__.py
│   │   ├── TMC2209.py                   # TMC2209 device implementation for motor control
│   │   ├── TMC2208.py                   # TMC2208 device implementation for motor control
│   │   ├── AS5600.py                    # AS5600 device implementation for position control
│   │   └── etc...                       # more devices to interface with  
│   │
│   ├── ui/                              # PyQt6 dialogs & widgets if needed
│   │   ├── __init__.py
│   │   ├── controlWidget1.py            # Control widget to set up a motor for example
│   │   ├── controlWidget1.py            # another widget style menu
│   │   └── motorDialog1.py              # another widget style menu
│   │   └── etc...
│   │
│   └── utils/                           # Cross-cutting utilities
│       ├── __init__.py
│       ├── log_utils.py                 # Logging utilities
│       └── schema_registry.py           # Data schema management
│
├── schemas/                             # Data schemas for specific data saving structures 
│   ├── MEMSYS_Lab_data_scheme_v1.0.json
│   └── MEMSYS_Lab_data_scheme_v2.0.json
│
├── tests/                               # Unit Testing suite
│   ├── __init__.py
│   ├── test_sweep_io.py
│   └── test_sweep_model.py
│
├── docs/                                # Documentation
│
└── [Data/, Profiles/, logs/, build/]    # User/output folders (keep in root, most are added to the .gitignore file)