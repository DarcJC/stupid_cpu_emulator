# B22 程序模拟器

Darc Z.的面向对象课程设计作业。包括README在内的全部文件都为原创(除LICENSE)。

Copyright @DarcJC. Following Anti-996 License Version 1.0.

## Application File Stipulate
The file consists of 3 parts: AH(application header), .data segment and .text segment. (ordered)
Use -101 to end each part, and AH will not load into application memory.

Note that .data and .text will be loaded *continuously*.

Datas in .data and .text should be integer and smaller than LONG_MAX(depend on OS, defined in limits.h), and each data separated by std::cin readable char(\n, \r).


### AH(not implemented yet)
AH should be located at the start of the file. AH included the informations of the application -- like author, version and copyrights.
Organize this informations into Key-Value is recommended.

### .data segment
Starting when first -101 appear. Making some data definition(or declaration).

### .text segment
The pc register will be pointed to first element of the .text segment.
++pc will exec after exec any none jmp-like opteration number.

## Operation Numbers
| 00 | fill & align      |
| 10 | input from stdin  |
| 12 | output to stdin   |
| 20 | load              |
| 22 | store             |
| 30 | add               |
| 32 | sub               |
| 34 | mul               |
| 36 | div               |
| 38 | surplus           |
| 40 | jmp anyway        |
| 42 | jmp if ax == 0    |
| 44 | program exit      |
| 50 | set eax           |
| 52 | set ebx           |
| 54 | flat the register |
| 60 | AND               |
| 62 | OR                |
| 64 | XOR               |
| 66 | NOT               |
|    |                   |
