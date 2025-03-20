# a07g-exploring-the-CLI

* Team Number: T16 
* Team Name: Slap Queen
* Team Members: Tianle Chen & Sitong Li
* GitHub Repository URL: https://github.com/ese5160/final-project-a07g-a14g-t16-slap-queen.git
* Description of test hardware: (development boards, sensors, actuators, laptop + OS, etc)


## 1. Updated Hardware and Software Requirements Specification (HRS & SRS)

### Hardware Requirements Specification (HRS)
- **Main Controller**: ATSAMW25H18-MR210PB1952 MCU and Wi-Fi module
- **Sensor System**:
   - MAX31855JASA+ temperature sensor, connected via SPI interface
   - VEML7700-TR light intensity sensor, connected via I2C interface
- **Actuators**:
   - COM-11288 heating pad, controlled via PWM
   - BL-HBXJXGX32L blue LED, controlled via PWM
- **Display System**: Adafruit 326 OLED monitor, connected via I2C interface
- **Power System**:
   - BQ24075 power management IC, supporting USB (5V/1.5A) and Li-Ion battery (3.7V/2200mAh) dual input
   - TPS631010 Buck-Boost converter, providing 5V/825mA power to the heating pad
   - TPS628438DRL Buck converter, providing 3.3V/167mA power to MCU and peripherals

### Software Requirements Specification (SRS)
- **System Control Task**:
   - Responsible for overall system coordination and operation mode control
   - Handles user interface logic and system state management
   - Implements state machine control of the diagnostic process
- **Sensor Task**:
   - Temperature sensor data acquisition and processing (SPI interface)
   - Light intensity sensor data acquisition and processing (I2C interface)
   - Sensor data filtering and anomaly detection
- **Heating Control Task**:
   - PWM control of heating pad temperature
   - Implementation of PID temperature control algorithm
   - Heating safety protection mechanism
- **Display Task**:
   - OLED display interface updates
   - System status and measurement results display
   - User prompt information display
- **Wi-Fi Communication Task**:
   - Establishing connection with remote server
   - Transmission of diagnostic data
   - Receiving remote control commands

## 2. Software Task Block Diagram

![Software Task Block Diagram](images/Software-Task.png)

## 3. Task State Machine Diagrams

### System Control Task State Machine

![System Control Task State Machine](images/System-Control.png)

### Sensor Task State Machine

![Sensor Task State Machine](images/Sensor-Task.png)

### Heater Control Task State Machine

![Heater Control Task State Machine](images/Heater-Control.png)

### Wi-Fi Communication Task State Machine

![Wi-Fi Communication Task State Machine](images/Wi-Fi-Task.png)


## Understanding the Starter Code

The starter code initializes the UART at 115200 baud 8N1, starts the FreeRTOS kernel, and implements a character echo system through ring buffers.

### UART Implementation Analysis

#### Question 1: InitializeSerialConsole() Function
`InitializeSerialConsole()` configures the UART interface for serial communication. It:
- Initializes the UART hardware at 115200 baud with 8 data bits, no parity, and 1 stop bit
- Creates two circular buffers: `cbufRx` for receiving characters and `cbufTx` for transmitting characters
- Sets up the interrupt handlers for UART events
- Registers callback functions for character reception and transmission

The variables `cbufRx` and `cbufTx` are circular buffer data structures used to store received and transmitted characters respectively.

#### Question 2: Circular Buffer Initialization
`cbufRx` and `cbufTx` are initialized using the `circular_buffer_init()` function, which allocates memory for the buffer and sets initial read/write pointers.

The circular buffer implementation is defined in `circular_buffer.c` and the corresponding header file `circular_buffer.h`.

#### Question 3: Character Storage Arrays
The actual character data is stored in arrays defined within the circular buffer structure:
- For `cbufRx`: `rxBuffer` with a size of `SERIAL_BUFFER_SIZE` (typically 64 bytes)
- For `cbufTx`: `txBuffer` with a size of `SERIAL_BUFFER_SIZE` (typically 64 bytes)

#### Question 4: UART Interrupt Definitions
The UART interrupts are defined in the `sercom3_handler()` function in the `uart_driver.c` file. This handler is triggered when:
- A character is received (RX_COMPLETE)
- A character has been transmitted (TX_COMPLETE)
- The transmit buffer is empty (TX_EMPTY)

#### Question 5: UART Callback Functions
The callback functions are:
1. For character reception (RX): `SerialConsoleRxCallback()`
2. For character transmission (TX): `SerialConsoleTxCallback()`

#### Question 6: Callback Function Operations
- `SerialConsoleRxCallback()`:
  - Is triggered when a character is received
  - Reads the character from the UART hardware
  - Adds the character to the `cbufRx` circular buffer
  - Echoes the character back by adding it to the `cbufTx` buffer

- `SerialConsoleTxCallback()`:
  - Is triggered when a character has been sent
  - Checks if there are more characters in the `cbufTx` buffer
  - If yes, it gets the next character and sends it via UART
  - If no, it disables the TX_EMPTY interrupt until more characters are added

## Program Flow Diagrams

### UART Receive Flow

```
User types character on terminal
    ↓
UART Hardware receives character and triggers RX_COMPLETE interrupt
    ↓
sercom3_handler() detects RX_COMPLETE and calls SerialConsoleRxCallback()
    ↓
SerialConsoleRxCallback() reads character from UART hardware
    ↓
Character is added to cbufRx using circular_buffer_write()
    ↓
Character is also added to cbufTx to echo it back
    ↓
SerialConsoleRxCallback() triggers TX if needed by enabling TX_EMPTY interrupt
    ↓
Character is now available in cbufRx for the application to read using SerialConsoleReadCharacter()
```

### UART Transmit Flow

```
Application calls SerialConsoleWriteString() with a string
    ↓
Each character of the string is added to cbufTx using circular_buffer_write()
    ↓
If UART is idle, the TX_EMPTY interrupt is enabled
    ↓
TX_EMPTY interrupt triggers sercom3_handler()
    ↓
sercom3_handler() calls SerialConsoleTxCallback()
    ↓
SerialConsoleTxCallback() reads the next character from cbufTx
    ↓
Character is sent to UART hardware for transmission
    ↓
When transmission is complete, TX_COMPLETE interrupt is triggered
    ↓
Process repeats for next character until cbufTx is empty
    ↓
Characters appear on the terminal screen
```

### FreeRTOS Tasks

#### Question 7: startTasks() Function
The `startTasks()` function in `main.c` initializes and starts the FreeRTOS tasks:

- It creates the following tasks:
  1. `vCommandConsoleTask`: Handles command line interface processing
  2. `taskSystem`: Manages system-level operations
  3. `taskPrint`: Handles periodic status printing
  
- Total number of tasks: 3

- The function sets appropriate priorities for each task and provides stack sizes.
- After creating the tasks, it starts the FreeRTOS scheduler using `vTaskStartScheduler()`.

## Conclusion

The starter code provides a robust foundation for UART communication using circular buffers and interrupt-driven I/O. It also establishes a basic FreeRTOS task structure for command processing and system management.

The circular buffer implementation enables efficient handling of asynchronous UART communication, allowing the application to read and write characters without blocking or losing data.