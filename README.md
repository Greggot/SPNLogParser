# Log Parser

Program reads config files of SPN values and log Format and then parses
data from log file.

## Log Format Config

Config file sonsists of several values:

- `Divider` specifies ASCII code of dividion symbol
- `ID` - CAN identifier column number(position)
- `DLC` - CAN Data Length Code position
- `Data` - First CAN data byte position
- `Time` - Time needed for analyzing file position

## SPN config

Config file consists of rows:

```C
SPNvariableName 0x18FFFFFF 8 16 +0
    ...
AnotherName 0x18FFFFEE 32 8 *15 -56
```

Name, ID, Position-Length(in bits) and sequence of operations separated either by `spaces` or `tabs` must be defined. Several variables can have same ID
