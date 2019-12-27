# File Structure

1. [File Structure](#file-structure)
   1. [Header](#header)
   2. [Chunks](#chunks)

## Header

The file header is the first 12 bytes of the file.

| Bytes  | Data Type | Description                                                         |
| :----: | --------- | :------------------------------------------------------------------ |
| [0,6]  | char[7]   | Are a unique name to identify the file type.                        |
|   7    | char      | Identifies the machine architecture that the file was was saved in. |
|   8    | char      | Identifies the byte order that the file was was saved in.           |
| [8,12] | int       | Is the file version.                                                |

Codes for bytes 7,8

| ASCII | Hex  | Description             |
| ----- | ---- | ----------------------- |
| '-'   | 0x2D | Byte 7 is 64 bits       |
| '_'   | 0x5F | Byte 7 is 32 bits       |
| 'V'   | 0x56 | Byte 8 is big endian    |
| 'v'   | 0x76 | Byte 8 is little endian |

## Chunks

