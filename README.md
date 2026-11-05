# High-Speed SECS/HSMS Semiconductor Equipment Emulator

A production-grade, multi-threaded C++17 implementation of the **SEMI E37 (HSMS)** transport protocol and **SEMI E5 (SECS-II)** messaging layer, designed for fab automation and equipment controller interfaces.

---

## Architectural Highlights

* **Protocol Framing & Endianness Handling:** Robust 10-byte HSMS header codec with big-endian / network byte order serialization.
* **Session State Engine:** Fully compliant SEMI E37 finite state machine managing connection life-cycles (`NOT CONNECTED` → `NOT SELECTED` → `SELECTED`).
* **SECS-II Data Layer:** Dynamic binary item encoder/decoder supporting dynamic `std::variant` types (Lists, ASCII, U1, U4, I4, Booleans) and standardized streams/functions (`S1F1`, `S1F2`, `S2F41`, `S6F11`).
* **Concurrent Socket Engine:** Thread-safe TCP socket driver with non-blocking worker threads and a mutex-guarded asynchronous message queue (`ThreadSafeQueue<T>`).
* **Automated CI/CD:** Dynamic build matrix testing across Linux GCC and Windows MSVC with GoogleTest suite execution on every commit.

---

## Standard Compliance Matrix

| SEMI Standard | Feature / Message | Implementation Details |
| :--- | :--- | :--- |
| **SEMI E37 (HSMS)** | Session State Machine | `NotConnected`, `NotSelected`, `Selected` states |
| **SEMI E37 (HSMS)** | Control Framing | `Select.req/rsp`, `Linktest.req/rsp`, `Separate.req` |
| **SEMI E5 (SECS-II)**| Item Header Codec | 1, 2, and 3-byte format & length header parsing |
| **SEMI E5 (SECS-II)**| `S1F1` / `S1F2` | *Are You There?* / *On Line Data* Heartbeat |
| **SEMI E5 (SECS-II)**| `S2F41` | *Host Command Send* (`START`, `STOP`, `ABORT`) |
| **SEMI E5 (SECS-II)**| `S6F11` | *Event Report Send* Equipment Status Notification |

---

## Directory Architecture

```text
sec2_hsms_emulator/
├── .github/
│   └── workflows/ci.yml       # Automated GitHub Actions build matrix
├── include/
│   └── hsms/
│       ├── byte_order.hpp     # Runtime endianness conversion utilities
│       ├── common.hpp         # System-wide versioning constants
│       ├── header.hpp         # HSMS 10-byte binary header definition
│       ├── secs2.hpp          # SECS-II binary payload encoder/decoder
│       ├── state_machine.hpp  # SEMI E37 session finite state machine
│       ├── tcp_server.hpp     # Multi-threaded TCP socket driver
│       └── thread_safe_queue.hpp # Thread-safe producer-consumer queue
├── src/                       # Module implementations
├── tests/                     # 15 GoogleTest unit & robustness test cases
└── CMakeLists.txt             # Target-based build automation