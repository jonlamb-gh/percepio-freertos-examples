target extended-remote :1234

monitor reset halt

load

break vAssertCalled
