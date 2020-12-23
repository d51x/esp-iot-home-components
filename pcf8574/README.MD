TODO:
проверка
адреса от 0x20 до 0x27 и 
       от 0x38 до 0x3F

P0
P1
P2
P3
P4
P5
P6
P7

есть прерывание INT, срабатывает на LOW

There is only one register to control four possibilities of the port pin: Input HIGH, input
LOW, output HIGH, or output LOW.

тип порта
Input HIGH
Input LOW
Output HIGH
Output LOW

этим управляет галка Режим инверсии

Simple code WRITE mode:
<S> <slave address + write> <ACK> <data out> <ACK> <data out> <ACK> ...
<data out> <ACK> <P>

Simple code for Read mode:
<S> <slave address + read> <ACK> <data in> <ACK> ... <data in> <ACK> <data in>
<NACK> <P>
