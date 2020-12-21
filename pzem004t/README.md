# Component: PZEM004t


pzem_init(0); // UART_NUM_0
PZEM_Address pzem_addr = {192, 168, 1, 1};
pzem_set_addr(pzem_addr);
float v = pzem_read_voltage();
float c = pzem_read_current();
float p =pzem_read_power();
float e = pzem_read_energy();